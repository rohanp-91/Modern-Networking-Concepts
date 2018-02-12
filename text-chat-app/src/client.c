
#include "../include/allinclude.h"

int server_fd;

int clientProcess(char* portNum)
{
    clientInitialize(portNum);
    fd_set sock_list;
    int selret, fd_max;
    server_fd = 1;    

    hasLoggedInBefore = FALSE;
    isLoggedIn = 0;

    while(TRUE) {

        //printAllClients();
        FD_ZERO(&sock_list);
        FD_SET(STDIN, &sock_list);
        FD_SET(server_fd, &sock_list);

        fd_max = server_fd;

        printf("\n[C]$ ");
		fflush(stdout);
                
        selret = select(fd_max + 1, &sock_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("Select failed");
        else {    
            if(FD_ISSET(STDIN, &sock_list)) {

                char* cmd = (char*) malloc(sizeof(int)*MSG_SIZE); 
                memset(cmd, '\0', sizeof(int)*MSG_SIZE);   
                if(fgets(cmd, sizeof(int)*MSG_SIZE, stdin) == NULL)                 
                    continue;

                cmd[strcspn(cmd, "\n")] = '\0';    
                char* token[3];                          
                int numOfLines = tokenize(cmd, token, " ");
                printf("token[1] in client: %s\n", token[1]);
                if(!strcmp(token[0], "LOGIN") && isLoggedIn == 0) {
                    printf("here in login client cmd\n");
                    if(!isValidIP(token[1]) || !isValidPort(token[2])) {
                        error("LOGIN");
                    }
                    else {
                        if(hasLoggedInBefore == FALSE) {
                            server_fd = clientLogin(token[1], token[2]);
                            hasLoggedInBefore = TRUE;
                        }
                        else {
                            send(server_fd, "LOGIN", sizeof("LOGIN"), 0);
                        }
                        isLoggedIn = 1;
                    }
                }
                else if(!strcmp(token[0], "REFRESH") && isLoggedIn == 1) {
                    clientRefresh(token[0]);
                }
                else if(!strcmp(token[0], "LOGOUT") && isLoggedIn == 1) {
                    clientLogout(token[0]);
                    isLoggedIn = 0;
                }
                else if(!strcmp(token[0], "EXIT")) {
                    clientExit(token[0]);
                    isLoggedIn = 0;
                    close(server_fd);   
                    exit(1);
                }
                else if(!strcmp(token[0], "SEND") && isLoggedIn == 1) {
                    char* msgToSend = (char*)malloc(sizeof(int)*MSG_SIZE);
                    memset(msgToSend, '\0', sizeof(int)*MSG_SIZE);
                    strcat(msgToSend, token[0]);
                    strcat(msgToSend, " ");
                    for(int i=1; i<numOfLines; i++) {
                        strcat(msgToSend, token[i]);
                        strcat(msgToSend, " ");
                    }
                    int flag = 0;
                    for(int i=0; i<4; i++) {
                        if(!strcmp(allClients[i]->IP, token[1])) {
                                clientSend(msgToSend);
                                flag = 1;
                                break;
                        }
                    }
                    if(flag == 0)
                        error("SEND");
                }
                else if(!strcmp(token[0], "BROADCAST") && isLoggedIn == 1) {
                    char* msgToSend = (char*)malloc(sizeof(int)*MSG_SIZE);
                    memset(msgToSend, '\0', sizeof(int)*MSG_SIZE);
                    strcat(msgToSend, token[0]);
                    strcat(msgToSend, " ");
                    for(int i=1; i<numOfLines; i++) {
                        strcat(msgToSend, token[i]);
                        strcat(msgToSend, " ");
                    }
                    clientBroadcast(msgToSend);
                }
                else if(!strcmp(token[0], "BLOCK") && isLoggedIn == 1) {
                    char* msgToSend = (char*)malloc(sizeof(int)*MSG_SIZE);
                    memset(msgToSend, '\0', sizeof(int)*MSG_SIZE);
                    strcat(msgToSend, token[0]);
                    strcat(msgToSend, " ");
                    for(int i=1; i<numOfLines; i++) {
                        strcat(msgToSend, token[i]);
                        strcat(msgToSend, " ");
                    }
                    int flag = 0;
                    for(int i=0; i<4; i++) {
                        if(!strcmp(allClients[i]->IP, token[1])) {
                                clientBlock(msgToSend);
                                flag = 1;
                                break;
                        }
                    }
                    if(flag == 0)
                        error("BLOCK");
                } 
                else if(!strcmp(token[0], "UNBLOCK") && isLoggedIn == 1) {
                    char* msgToSend = (char*)malloc(sizeof(int)*MSG_SIZE);
                    memset(msgToSend, '\0', sizeof(int)*MSG_SIZE);
                    strcat(msgToSend, token[0]);
                    strcat(msgToSend, " ");
                    for(int i=1; i<numOfLines; i++) {
                        strcat(msgToSend, token[i]);
                        strcat(msgToSend, " ");
                    }
                    int flag = 0;
                    for(int i=0; i<4; i++) {
                        if(!strcmp(allClients[i]->IP, token[1])) {
                            clientUnblock(msgToSend);
                            flag = 1;
                            break;
                        }
                    }
                    if(flag == 0)
                        error("UNBLOCK");
                }
                else if(!strcmp(token[0], "AUTHOR")) {
                    commonAuthor();
                }
                else if(!strcmp(token[0], "IP")) {
                    commonIP();
                }
                else if(!strcmp(token[0], "PORT")) {
                    commonPort();
                }
                else if(!strcmp(token[0], "LIST") && isLoggedIn == 1) {
                    commonList();
                }
                fflush(stdin);
            }
            else if(FD_ISSET(server_fd, &sock_list)) {
                
                char* buffer = (char*)malloc(sizeof(int)*MSG_SIZE);
                memset(buffer, '\0', sizeof(int)*MSG_SIZE);
                int recv_val = recv(server_fd, buffer, sizeof(int)*MSG_SIZE, 0);
                if(recv_val <= 0) {
                    close(server_fd);
                    server_fd = -1;
                }
                else {
                    //printf("%s\n", "message from server");
                    printf("message from server is: %s\n", buffer);

                    int i = 0, count = 0;
                    char* eachClientDetail[6];
                    //printf("count in client: %d\n", count);
                    char* brokenMsg[MSG_SIZE];
                    //printf("message in client: %s\n", buffer);
                    int numOfLines = tokenize(buffer, brokenMsg, "\n");
                    printf("brokenMsg[0] and [1] in client recv: %s & %s\n", brokenMsg[0], brokenMsg[1]);

                    if(!strcmp(brokenMsg[1], "LOGINSUCCESS"))
                    {
                        cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");

                        for(i = 2; i<numOfLines; i++) {

                            if(!strcmp(brokenMsg[i], "ENDOFCLIENTINFO"))
                                break;  
                            tokenize(brokenMsg[i], eachClientDetail, " ");
                            assignToAllClients(eachClientDetail[0], eachClientDetail[1], eachClientDetail[2], 
                                             eachClientDetail[3], eachClientDetail[4], eachClientDetail[5], allClients[count]);
                            count++;
                        }
                        //printf("count: %d\n", count);
                        for(;count<4; count++)
                            assignToAllClients("", "", "", 0, 0, "", allClients[count]);
                        for(i = i+1; i<numOfLines; i++) {
                            printf("%s\n", "have buffer msg");
                            char* buffMsgInfo[BUFFER_SIZE];
                            if(!strcmp(brokenMsg[i], "BUFFEREND"))
                                break;
                            msgRecvEvent(brokenMsg[i]);
                        }

                        cse4589_print_and_log("[%s:END]\n", "LOGIN");
                    }

                    else if(!strcmp(brokenMsg[1], "REFRESHSUCCESS"))
                    {
                            cse4589_print_and_log("[%s:SUCCESS]\n", "REFRESH");
                            for(i = 2; i<numOfLines; i++) {
                                if(!strcmp(brokenMsg[i], "ENDOFCLIENTINFO"))
                                    break;  
                                
                                tokenize(brokenMsg[i], eachClientDetail, " ");
                                assignToAllClients(eachClientDetail[0], eachClientDetail[1], eachClientDetail[2], 
                                                 eachClientDetail[3], eachClientDetail[4], eachClientDetail[5], allClients[count]);
                                count++;
                            }
                            for(;count<4; count++)
                                assignToAllClients("", "", "", 0, 0, "", allClients[count]);
                            if(i == 2) {
                                for(i = 0; i<4; i++)
                                    if(allClients[i]->socket != 0)
                                        allClients[i]->status = "logged-out";
                            }

                            cse4589_print_and_log("[%s:END]\n", "REFRESH");
                    }
                    else if(!strcmp(brokenMsg[0], "SEND"))
                    {
                        cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");

                        char* fullMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
                        sprintf(fullMsg, "%s", brokenMsg[2]);
                        strcat(fullMsg, " ");
                        for(i = 3; i<numOfLines; i++) {
                            strcat(fullMsg, brokenMsg[i]);
                            strcat(fullMsg, " ");
                        }
                        cse4589_print_and_log("msg from:%s\n[msg]:%s\n", brokenMsg[1], fullMsg);
                        cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                        //free(fullMsg);
                        //printf("%s\n", "completed client side of receive");
                    }
                    else if(!strcmp(brokenMsg[0], "BROADCAST"))
                    {
                        cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");

                        char* fullMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
                        sprintf(fullMsg, "%s", brokenMsg[2]);
                        strcat(fullMsg, " ");
                        for(i = 3; i<numOfLines; i++) {
                            strcat(fullMsg, brokenMsg[i]);
                            strcat(fullMsg, " ");
                        }
                        cse4589_print_and_log("msg from:%s\n[msg]:%s\n", brokenMsg[1], fullMsg);
                        cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                        //free(fullMsg);
                    }
                    else if(!strcmp(brokenMsg[0], "SBLOCK")) {
                        cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCK");
                        cse4589_print_and_log("[%s:END]\n", "BLOCK");
                    }
                    else if(!strcmp(brokenMsg[0], "EBLOCK")) {
                        error("BLOCK");
                    }
                    else if(!strcmp(brokenMsg[0], "SUNBLOCK")) {
                        printf("%s\n", "came in SUNBLOCK");
                        cse4589_print_and_log("[%s:SUCCESS]\n", "UNBLOCK");
                        cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                    }
                    else if(!strcmp(brokenMsg[0], "EUNBLOCK")) {
                        printf("%s\n", "came in EUNBLOCK");
                        error("UNBLOCK");
                    }

                }
            }
        }
    }

    return 0;
}


void clientInitialize(char* portNum) {

    getMyAddress(portNum);
    
}