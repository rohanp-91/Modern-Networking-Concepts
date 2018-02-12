
#include "../include/allinclude.h"


void serverLoginEvent(int client_fd) {

	int i;
	
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

	// if(strcmp(client->bufferMsg, "")) {

	// 	char* temp_msg = (char*)realloc(msg, strlen(msg) + strlen(client->bufferMsg) + 1);
	// 	msg = temp_msg;
	// 	strcat(msg, client->bufferMsg);
	// 	strcat(msg, "BUFFEREND\n");
	
	// }

	char* final_msg = (char*)malloc(sizeof(int)*MSG_SIZE);
	sprintf(final_msg, "%d\n%s", strlen(msg) + 3, msg);
	//printf("message from server: %s\n", final_msg);
	int lengthOfMsg = strlen(final_msg);
	if(sendall(client_fd, final_msg, &lengthOfMsg) < 0)
		perror("send error from serverLoginEvent");

	//free(msg);
	//free(final_msg);
	//free(client);
}

void serverRefreshEvent(int client_fd) {

	int i, counter = 1; 
	char* eachClientData[5];
	for(i = 0; i<5; i++)
		eachClientData[i] = (char*)malloc(256);
	eachClientData[0] = "REFRESHSUCCESS\n";
	char* msg;
	for(i = 0; i<4; i++) {
		
		if(!strcmp(allClients[i]->status, "logged-in") && allClients[i]->socket != 0) {
			
			msg = malloc(snprintf(NULL, 0, "%s %s %s %d %d %s", allClients[i]->IP, allClients[i]->hostName, allClients[i]->port, 
							allClients[i]->msgSentCount, allClients[i]->msgRecvCount, allClients[i]->status) + 1);

			sprintf(msg, "%s %s %s %d %d %s", allClients[i]->IP, allClients[i]->hostName, allClients[i]->port, allClients[i]->msgSentCount, 
						allClients[i]->msgRecvCount, allClients[i]->status);
			
			sprintf(eachClientData[counter], "%s", msg);
			counter++;
			//free(msg);
		}
	}
	
	msg = (char*)malloc(sizeof(int)*MSG_SIZE);
	strcpy(msg, eachClientData[0]);
	for(i = 1; i<counter; i++) {
		if(eachClientData[i] != NULL) {
			strcat(msg, eachClientData[i]);
			strcat(msg, "\n");
		}
	}
	
	strcat(msg, "ENDOFCLIENTINFO\n");
	char* final_msg = (char*)malloc(sizeof(int)*MSG_SIZE);
	if(strlen(msg) < 100)
		sprintf(final_msg, "0%d\n", strlen(msg) + 3);
	else
		sprintf(final_msg, "%d\n", strlen(msg) + 3);
	strcat(final_msg, msg);
	int lengthOfMsg = strlen(final_msg);
	//printf("final_msg from server: %s\n", final_msg);
	if(sendall(client_fd, final_msg, &lengthOfMsg) < 0)
		perror("send error in serverLoginEvent");

	//free(msg);
	//free(final_msg);
}

void serverLogoutEvent(int client_fd) {

	int i;
	for(i = 0; i<4; i++) {
		if(allClients[i]->socket == client_fd) {
			allClients[i]->status = "logged-out";
			return;
		}
	}
}

void serverExitEvent(int client_fd) {

	//printf("%s\n", "here serverExitEvent");
	int i;
	for(i = 0; i<4; i++) 
		if(allClients[i]->socket == client_fd && allClients[i]->socket != 0) 
			allClients[i]->socket = 0;
	close(client_fd); 
        // char* msg = "EXITSUCCESS";
	// int lengthOfMsg = strlen(msg);
	// sendall(client_fd, msg, &lengthOfMsg);
	// //printf("%s\n", "sent exit");
}

void serverSendEvent(int client_fd, char* msg) {

	int i;
	//printf("msg in server send event: %s\n", msg);
	char* brokenMsg[MSG_SIZE];
	int lengthOfMsg = tokenize(msg, brokenMsg, " ");
	clientDetails* hostClient = (clientDetails*)malloc(sizeof(clientDetails));
	getClientInfoBySocket(client_fd, &hostClient);
	//printf("host client IP: %s\n", hostClient->IP);
	clientDetails* destClient = (clientDetails*)malloc(sizeof(clientDetails));
	for(i = 0; i<4; i++) {
		if(!strcmp(brokenMsg[1], allClients[i]->IP) && allClients[i]->socket != 0) {
			destClient = allClients[i];
			break;
		}
	}

	char* fullMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
	memset(fullMsg, '\0', sizeof(fullMsg));
	sprintf(fullMsg, "%s\n%s\n", brokenMsg[0], hostClient->IP);
	char* onlyMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
	memset(onlyMsg, '\0', sizeof(onlyMsg));
	for(i = 2;i<lengthOfMsg; i++) {
		strcat(onlyMsg, brokenMsg[i]);
		strcat(onlyMsg, "\n");
	}
	//printf("only msg: %s\n", onlyMsg);
	int flag = 0;
	strcat(fullMsg, onlyMsg);
	for(int j=0; j<destClient->blockedCount; j++) {
		if(!strcmp(destClient->blockedUser[j], hostClient->IP)) {
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		if(!strcmp(destClient->status, "logged-in")) {
			lengthOfMsg = strlen(fullMsg);
			sendall(destClient->socket, fullMsg, &lengthOfMsg);
		}
		else {
			strcat(destClient->bufferMsg, fullMsg);
			strcat(destClient->bufferMsg, " ");
		}
		destClient->msgRecvCount++;
		hostClient->msgSentCount++;
	}
		//printf("full msg: %s\n", fullMsg);
	for(i = 0; i<strlen(onlyMsg) - 1; i++)
		if(onlyMsg[i] == '\n')
			onlyMsg[i] = ' ';
	onlyMsg[i] = '\0';
	//printf("only msg modified: %s\n", onlyMsg);
	//onlyMsg[strlen(onlyMsg) - 1] = '\0';
	cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", hostClient->IP, destClient->IP, onlyMsg);
	cse4589_print_and_log("[%s:END]\n", "RELAYED");;
	//free(onlyMsg);
	//free(fullMsg);
}

void serverBroadcastEvent(int client_fd, char* msg) {

	int i;
	char* brokenMsg[MSG_SIZE];
	int lengthOfMsg = tokenize(msg, brokenMsg, " ");
	clientDetails* hostClient = (clientDetails*)malloc(sizeof(clientDetails));
	getClientInfoBySocket(client_fd, &hostClient);

	char* fullMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
	memset(fullMsg, '\0', sizeof(int)*MSG_SIZE);
        sprintf(fullMsg, "%s\n%s\n", brokenMsg[0], hostClient->IP);
	char* onlyMsg = (char*)malloc(sizeof(int)*MSG_SIZE);
	memset(onlyMsg, '\0', sizeof(int)*MSG_SIZE);
        for(i = 1;i<lengthOfMsg; i++) {
		strcat(onlyMsg, brokenMsg[i]);
		strcat(onlyMsg, "\n");
	}
	strcat(fullMsg, onlyMsg);
	for(i = 0; i<4; i++)
	{
		if(allClients[i]->socket == hostClient->socket)
			continue;
		lengthOfMsg = strlen(fullMsg);
		if(!strcmp(allClients[i]->status, "logged-in"))
			sendall(allClients[i]->socket, fullMsg, &lengthOfMsg);
		else {
			strcat(allClients[i]->bufferMsg, fullMsg);
			strcat(allClients[i]->bufferMsg, " ");
		}
		allClients[i]->msgRecvCount++;

	}
	hostClient->msgSentCount++;
	for(i = 0; i<strlen(onlyMsg); i++)
		if(onlyMsg[i] == '\n')
			onlyMsg[i] = ' ';
	//printf("full broadcast msg to client: %s\n", fullMsg);	
	cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", hostClient->IP, "255.255.255.255", onlyMsg);
	cse4589_print_and_log("[%s:END]\n", "RELAYED");;
	//free(onlyMsg);
	//free(fullMsg);
	
}

void serverBlockEvent(int client_fd, char* msg) {

	printf("msg in server block event: %s\n", msg);
	int i, flag;
	char* brokenMsg[MSG_SIZE];
	int lengthOfMsg = tokenize(msg, brokenMsg, " ");
	printf("brokenMsg[0] and [1] in serverBlockEvent: %s %s\n", brokenMsg[0], brokenMsg[1]);
	for(i = 0; i<4; i++) {
		if(allClients[i]->socket == client_fd) {
			flag = 0;
			int counter = allClients[i]->blockedCount;
			for(int j = 0; j<counter; j++) {	
				if(!strcmp(allClients[i]->blockedUser[j], brokenMsg[1])) {
					flag = 1;
					send(client_fd, "EBLOCK\n12", strlen("EBLOCK\n12"), 0); 
					break;
				}
			}

			if(flag == 0)
			{
				allClients[i]->blockedUser[counter] = strdup(brokenMsg[1]);
				allClients[i]->blockedCount++;
				send(client_fd, "SBLOCK\n12", strlen("SBLOCK\n12"), 0);
				break;
			}
		}
	}
		
}

void serverUnblockEvent(int client_fd, char* msg) {

	printf("msg in server unblock event: %s\n", msg);
	int i;
	char* brokenMsg[MSG_SIZE];
	int lengthOfMsg = tokenize(msg, brokenMsg, " ");
	printf("brokenMsg[0] and [1] in serverBlockEvent: %s %s\n", brokenMsg[0], brokenMsg[1]);
	for(i = 0; i<4; i++) {
		if(allClients[i]->socket == client_fd) {

			int flag = 0;
			int counter = allClients[i]->blockedCount;
			for(int j = 0; j<counter; j++) {	
				if(!strcmp(allClients[i]->blockedUser[j], brokenMsg[1])) {
					flag = 1;
					int k;
					for(k=j; k<3; k++)
						allClients[i]->blockedUser[k] = strdup(allClients[i]->blockedUser[k+1]);
					allClients[i]->blockedUser[k] = "";
					allClients[i]->blockedCount--;
					send(client_fd, "SUNBLOCK\n12", strlen("SUNBLOCK\n12"), 0);
					break;
				}
			}

			if(flag == 0)
			{
				allClients[i]->blockedUser[counter] = strdup(brokenMsg[1]);
				allClients[i]->blockedCount++;
				send(client_fd, "EUNBLOCK\n12", strlen("EUNBLOCK\n12"), 0);
				break;
			}
		}
	}
}

void serverStatisticCommand(char* cmd) {

	printf("in server serverStatisticCommand\n");
	int i, co;
	cse4589_print_and_log("[%s:SUCCESS]\n", "STATISTICS");
	for(i = 0; i<4; i++)
		if(allClients[i]->socket != 0) 
			cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, allClients[i]->hostName, 
									allClients[i]->msgSentCount, allClients[i]->msgRecvCount, 
										allClients[i]->status);
	cse4589_print_and_log("[%s:END]\n", "STATISTICS");
}

void serverBlockedCommand(char* cmd) {

	int i;
	printf("msg in serverBlockedCommand: %s\n", cmd);
	char* brokenMsg[2];
	int lengthOfMsg = tokenize(cmd, brokenMsg, " ");
	clientDetails* hostClient = (clientDetails*)malloc(sizeof(clientDetails));
	printf("brokenmsg[1]: %s\n", brokenMsg[1]);
	getClientInfoByIP(brokenMsg[1], &hostClient);
	clientDetails* blockedClients[hostClient->blockedCount];
	cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCKED");
	printf("ip: %s\n", hostClient->IP);
	for(i = 0; i<hostClient->blockedCount; i++) {
		//printf("%s\n", "here 1");
		clientDetails* destClient = (clientDetails*)malloc(sizeof(clientDetails));
		getClientInfoByIP(hostClient->blockedUser[i], &destClient);
		//printf("%s\n", "here 2");
		blockedClients[i] = destClient;
		//free(destClient);
	}
	printf("here 4\n");  
        if(i != 0)
		sortByPort2(blockedClients, hostClient->blockedCount);
	printf("%s\n", "here 3");
	for(i = 0; i<hostClient->blockedCount; i++)
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, blockedClients[i]->hostName, blockedClients[i]->IP, atoi(blockedClients[i]->port));
	cse4589_print_and_log("[%s:END]\n", "BLOCKED");	
	//free(hostClient);
}













