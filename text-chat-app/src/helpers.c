

#include "../include/allinclude.h"
#include <ctype.h>
void getMyAddress(char* portNum) {

	myPort = portNum;
	char hostname[1024]; 
	gethostname(hostname, sizeof(hostname) - 1);
	if(hostname != NULL) {
		myHostName = (char*)malloc(snprintf(NULL, 0, "%s", hostname));
		snprintf(myHostName, "%s", hostname);
	}
	char* genIP = "8.8.4.4"; 						//Google DNS
	myIP = (char*)malloc(sizeof(char)*256);
	size_t myIP_len = sizeof(myIP);										
	uint16_t genPort = 53;                                   
	int sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = inet_addr(genIP);
  	server.sin_port = htons(genPort);
  	int t = connect(sock_udp, (struct sockaddr*)&server, sizeof(server));
  	struct sockaddr_in getname;
  	socklen_t getname_len = sizeof(getname);
  	getsockname(sock_udp, (struct sockaddr*)&getname, &getname_len);
  	inet_ntop(AF_INET, &getname.sin_addr, myIP, &myIP_len);
  	close(sock_udp);
}


int tokenize(char* cmd, char* tokens[], char* delim) {

    int i=0;                       
    char* token = strtok(cmd, delim);
    while(token) {
    	tokens[i++] = token;
    	token = strtok(NULL, delim);
	}
	return i;
}

int isValidIP(char* IP) {

	int flag =0;
	printf("%s\n", "here isValidIP 1");
	char* check[] = {"128.205.36.46", "128.205.36.33", "128.205.36.34", "128.205.36.35", "128.205.36.36"};
	printf("%s\n", "here isValidIP 2");
	for(int i=0; i<5; i++) {
		if(!strcmp(check[i], IP)) {
			flag = 1;
			break;
		}
	}
	printf("%s\n", "here isValidIP 3");
	if(flag == 0)
		return FALSE;
	return TRUE;
}

int isValidPort(char* port) {

	int i, flag = 0;
	for(i = 0; i<strlen(port); i++)
		if(!isdigit(port[i])) {
			flag++;
			break;
		}

	if(atoi(port) <= 0 || atoi(port) > 65535)
		flag++;
	if(flag > 0)
		return FALSE;
	return TRUE;
	
}

void newClient(int client_fd, char* IP, char* port, char* hostname, clientDetails* client) {

	(client->socket) = client_fd;
	(client->hostName) = hostname;
	(client->IP) = IP;
	(client->port) = port;
	(client->msgSentCount) = 0;
	(client->msgRecvCount) = 0;
	(client->status) = "logged-in";
}


void getClientInfoBySocket(int client_fd, clientDetails** client) {

	int i;
	//printf("client_fd: %d\n", client_fd);
	for(i = 0; i<4; i++) {

		if(allClients[i]->socket == client_fd && allClients[i]->socket != 0) {
			*client = allClients[i];
			break;
		}
	}
	//printf("i: %d\n", i);
}


void getClientInfoByIP(char* IP, clientDetails** client) {

	int i;
	for(i = 0; i<4; i++) {

		if(!strcmp(allClients[i]->IP, IP)) {
			*client = allClients[i];
			break;
		}
	}
}

int isBlocked(clientDetails* hostClient, clientDetails* destClient) {

	int i;
	for(i = 0; i<hostClient->blockedCount; i++) {

		if(!strcmp(hostClient->blockedUser[i], destClient->IP)) {
			return 1;
			break;
		}
		return 0;
	}
}

char* getClientHostName(char* client_ip) {

	char* hostnames[6][2];
	hostnames[0][0] = "128.205.36.34";
	hostnames[0][1] = "euston.cse.buffalo.edu";
	hostnames[1][0] = "128.205.36.35";
	hostnames[1][1] = "embankment.cse.buffalo.edu";
	hostnames[2][0] = "128.205.36.36";
	hostnames[2][1] = "underground.cse.buffalo.edu";
	hostnames[3][0] = "128.205.36.46";
	hostnames[3][1] = "stones.cse.buffalo.edu";
	hostnames[4][0] = "128.205.36.33";
	hostnames[4][1] = "highgate.cse.buffalo.edu";
	hostnames[5][0] = "192.168.1.179";
	hostnames[5][1] = "myhome.myroom.mybed.mymac";
	for(int i = 0; i<6; i++) {
		if(!strcmp(hostnames[i][0], client_ip))
			return (char*)hostnames[i][1];
	}
	return NULL;
}

void error(char* cmd) {

	cse4589_print_and_log("[%s:ERROR]\n", cmd);
  	cse4589_print_and_log("[%s:END]\n", cmd);

}

int sendall(int dest, char* buf, int* len)
{
    int total = 0;        
    int bytesleft = *len; 
    int n;

    while(total < *len) {
        n = send(dest, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
	}
    *len = total; 
    return n==-1?-1:0; 
}

int recvall(int host, char* buf, int* len)
{
	int read = 0;        
    int bytesleft = *len; 		 
    int n;

    while(bytesleft > 0) {
        n = recv(host, buf+read, bytesleft, 0);
        //printf("part recv: %s\n", buf);
        if (n == -1) { break; }
        read += n;
        bytesleft -= n;
	}
    *len = read; 
    return n==-1?-1:0;
}

void assignToAllClients(char* IP, char* hostname, char* port, int msgsent, int msgrecv, char* status, clientDetails* client) {

	client->socket = 1;
	client->IP = IP;
	client->hostName = hostname;
	client->port = port;
	client->msgSentCount = msgsent;
	client->msgRecvCount = msgrecv;
	client->status = status;
}

void sortByPort(clientDetails* allClients[]) {

	clientDetails* temp = (clientDetails*)malloc(sizeof(clientDetails));
	for(int i = 0; i< 3; i++) {

		for(int j = 0; j< 3 - i; j++) {

		//	if((allClients[j]->socket != 0) && (allClients[j+1]->socket != 0)) {
				if(atoi(allClients[j]->port) > atoi(allClients[j+1]->port))
				{
					temp = allClients[j];
					allClients[j] = allClients[j+1];
					allClients[j+1] = temp;
				}

		//	}
		}
	}			
}

void sortByPort2(clientDetails* clients[], int n) {

	clientDetails* temp = (clientDetails*)malloc(sizeof(clientDetails));
	for(int i = 0; i< n-1; i++) {

		for(int j = 0; j< n - i -1; j++) {

			//if((clients[j]->socket != 0) && (clients[j+1]->socket != 0)) {
				if(atoi(allClients[j]->port) > atoi(allClients[j+1]->port))
				{
					temp = clients[j];
					clients[j] = clients[j+1];
					clients[j+1] = temp;
				}

			
		}
	}	
}

void printAllClients() {

	for(int i=0; i<4; i++) {
		printf("allClients[%d] -> ", i);
		printf("PORT: %s ", allClients[i]->port); 
		printf("IP: %s ", allClients[i]->IP);
		printf("STATUS: %s ", allClients[i]->status);
		printf("HOSTNAME: %s\n", allClients[i]->hostName);
	}

}



