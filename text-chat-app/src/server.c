
#include "../include/allinclude.h"

fd_set mastersock_list, watchsock_list;  
int serverProcess(char* portNum)
{
    int listening_socket, client_fd, selret, sock_index, client_addr_len, fd_max;
	struct sockaddr_in client_addr;
	//fd_set mastersock_list, watchsock_list;
    char* client_ip[4];
    int ip_count;
    for(ip_count = 0; ip_count<4; ip_count++)
        client_ip[ip_count] = (char*)malloc(16);
    ip_count = 0;
	/* Socket */

    listening_socket = serverInitialize(portNum);
    /* Listen */
    if(listen(listening_socket, BACKLOG) < 0)
    	perror("Unable to listen on port");

    FD_ZERO(&mastersock_list);
    FD_ZERO(&watchsock_list);
    
    FD_SET(listening_socket, &mastersock_list);
    FD_SET(STDIN, &mastersock_list);

    fd_max = listening_socket;

    while(TRUE) {
        //printf("a[0] IP 1: %s\n", allClients[0]->IP);
        //printAllClients();
        printf("\n[S]$ ");
        fflush(stdout);
        
        watchsock_list = mastersock_list;

        selret = select(fd_max + 1, &watchsock_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");
        else{
    
            for(sock_index=0; sock_index<=fd_max; sock_index+=1) {

                if(FD_ISSET(sock_index, &watchsock_list)){

                    if (sock_index == STDIN) {
                    	char* cmd = (char*)malloc(sizeof(int)*MSG_SIZE);
                        memset(cmd, '\0', sizeof(int)*MSG_SIZE);
                        if(fgets(cmd, sizeof(int)*MSG_SIZE - 1, stdin) == NULL) 
							exit(-1);
                        cmd[strcspn(cmd, "\n")] = '\0';   //https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
                        char* dupCmd = strdup(cmd);
                        serverCommands(dupCmd);
                        fflush(stdin);
						//free(cmd);
                    }
                    
                    else if(sock_index == listening_socket) {
                        //memset(client_addr, '\0', sizeof(client_addr));
                        client_addr_len = sizeof(client_addr);
                        client_fd = accept(listening_socket, (struct sockaddr *)&client_addr, &client_addr_len);
                        
                        if(client_fd < 0)
                            perror("Accept failed.");
                        char* msg = (char*)malloc(MSG_SIZE);
                        recv(client_fd, msg, MSG_SIZE, 0);
                        //printf("msg: %s\n", msg);
                        char* t[2];
                        tokenize(msg, t, "\n");
                        //printf("t[0]: %s\n", t[0]);
                        //printf("t[1]: %s\n", t[1]);
                        
                        for(int i=0; i<4; i++)
                        {
                            if(allClients[i]->socket == 0) {

                                char* hostname = getClientHostName(t[0]);
                                newClient(client_fd, t[0], t[1], hostname, allClients[i]);
                                break;

                            }
                        }
                        serverLoginEvent(client_fd);

                        FD_SET(client_fd, &mastersock_list);
                        if(client_fd > fd_max) fd_max = client_fd;
                    }
                    
                    else {
                        //printf("%s\n", "message from client");
                        char* msg = (char*)malloc(sizeof(int)*MSG_SIZE);
                        memset(msg, '\0', sizeof(int)*MSG_SIZE);
                        int recv_val = recv(sock_index, msg, sizeof(int)*MSG_SIZE, 0);
                        
                        if(recv_val < 0) {

                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
                            
                            FD_CLR(sock_index, &mastersock_list);
                        }
                        else {	
                            serverEvents(msg, sock_index);
                            //printf("%s\n", "just after serverEvents");
                        }
                        //printf("message sent to server event: %s\n", msg);
                        //free(msg);
                    }
                }
            }
        }
    }

    return 0;
}

int serverInitialize(char* portNum) 
{
    getMyAddress(portNum);
    int port, listening_socket;
    struct sockaddr_in server_addr;
    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listening_socket < 0)
        perror("Cannot create socket");

    /* Fill up sockaddr_in struct */
    port = atoi(portNum);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Bind */
    if(bind(listening_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
        perror("Bind failed");
    return listening_socket;
}

void serverEvents(char* msg, int client_fd) {

    int i;
    char* token[MSG_SIZE];
    //memset(token, '\0', sizeof(token));
    //printf("msg in serverEvents: %s\n", msg);
    char* msgDup = strdup(msg);
    int numOfLines = tokenize(msg, token, " ");

    if(!strcmp(token[0], "LOGIN")) {
        //printf("%s\n", "here for already logged in client");
        clientDetails* client = (clientDetails*)malloc(sizeof(clientDetails));
        //printf("client_fd: %d\n", client_fd);
        getClientInfoBySocket(client_fd, &client);
        //printf("client->socket: %d client->port: %s\n", client->socket, client->port);
        client->status = "logged-in";
        int counter = 1;
        char* eachClientData[5];
        for(i = 0; i<5; i++)
            eachClientData[i] = (char*)malloc(256);
        eachClientData[0] = "LOGINSUCCESS\n";
        char* msg;
        for(i = 0; i<4; i++) {
            
            if(!strcmp(allClients[i]->status, "logged-in") && allClients[i]->socket != 0) {
                
                msg = (char*)malloc(snprintf(NULL, 0, "%s %s %s %d %d %s", allClients[i]->IP, allClients[i]->hostName, allClients[i]->port, 
                                allClients[i]->msgSentCount, allClients[i]->msgRecvCount, allClients[i]->status) + 1);

                sprintf(msg, "%s %s %s %d %d %s", allClients[i]->IP, allClients[i]->hostName, allClients[i]->port, allClients[i]->msgSentCount, 
                            allClients[i]->msgRecvCount, allClients[i]->status);
                sprintf(eachClientData[counter], "%s", msg);
                counter++;
                //free(msg);
            }
        }
        msg = (char*)malloc(sizeof(int)*MSG_SIZE);
        memset(msg, '\0', sizeof(int)*MSG_SIZE);
        strcpy(msg, eachClientData[0]);
        for(i = 1; i<counter; i++) {
            if(eachClientData[i] != NULL) {
                strcat(msg, eachClientData[i]);
                strcat(msg, "\n");
            }
        }
        
        strcat(msg, "ENDOFCLIENTINFO\n");

        // if(client->bufferMsg != NULL) {
        //     printf("client buffer msg: %s\n", client->bufferMsg);
        //     char* temp_msg = (char*)realloc(msg, strlen(msg) + strlen(client->bufferMsg) + 1);
        //     msg = temp_msg;
        //     strcat(msg, client->bufferMsg);
        //     strcat(msg, "BUFFEREND\n");
        // }

        char* final_msg = (char*)malloc(sizeof(int)*MSG_SIZE);
        sprintf(final_msg, "%d\n%s", strlen(msg) + 3, msg);
        //printf("message from server: %s\n", final_msg);
        int lengthOfMsg = strlen(final_msg);
        if(sendall(client_fd, final_msg, &lengthOfMsg) < 0)
            perror("send error from serverLoginEvent");

        // free(msg);
        // free(final_msg);
        // free(client);
    }

    if(!strcmp(token[0], "REFRESH")) { 
        serverRefreshEvent(client_fd);
    }
    if(!strcmp(token[0], "LOGOUT")) {
        serverLogoutEvent(client_fd);
    }
    if(!strcmp(token[0], "EXIT")) {
        serverExitEvent(client_fd);
        FD_CLR(client_fd, &mastersock_list);
    }
    if(!strcmp(token[0], "SEND")) {
        printf("in server message rec send area\n");
        char* sendMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
        memset(sendMsg, '\0', sizeof(sendMsg));
        strcat(sendMsg, token[0]);
        strcat(sendMsg, " ");
        for(i = 1; i<numOfLines; i++) {
            strcat(sendMsg, token[i]);
            strcat(sendMsg, " ");
        }

         //printf("sending to serverEvent: %s\n", sendMsg); 
         serverSendEvent(client_fd, sendMsg);
        //printf("%s\n", "done server send event");
    }
    if(!strcmp(token[0], "BROADCAST")) {
        char* sendMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
        memset(sendMsg, '\0', sizeof(int)*MSG_SIZE);          
        strcat(sendMsg, token[0]);
        strcat(sendMsg, " ");
        for(i = 1; i<numOfLines; i++) {
            strcat(sendMsg, token[i]);
            strcat(sendMsg, " ");
        }
        serverBroadcastEvent(client_fd, sendMsg);
    }
    if(!strcmp(token[0], "BLOCK")) {
        serverBlockEvent(client_fd, msgDup);
    }
    if(!strcmp(token[0], "UNBLOCK")) {
        serverUnblockEvent(client_fd, msgDup);
    }

}

void serverCommands(char* cmd) {

    printf("in srver commands\n");
    char* token[3];
    char* msgDup = strdup(cmd);
    tokenize(cmd, token, " ");
    printf("msgDup in serverCommands: %s\n", msgDup);
    if(!strcmp(token[0], "STATISTICS")) {
        serverStatisticCommand(msgDup);
    }
    else if(!strcmp(token[0], "BLOCKED")) {
        if(!isValidIP(token[1]))
            error("BLOCKED");
        else
            serverBlockedCommand(msgDup);
    }
    else if(!strcmp(token[0], "AUTHOR")) {
        commonAuthor();
    }
    else if(!strcmp(token[0], "IP")) {
        commonIP();
    }
    else if(!strcmp(token[0], "LIST")) {
        commonList();
    }
    else if(!strcmp(token[0], "PORT")) {
        commonPort();
    }
    else {
        return;
    }
}


