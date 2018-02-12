#ifndef ALLINCLUDE_H_
#define ALLINCLUDE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "../include/global.h"
#include "../include/logger.h"

#define TRUE 1
#define FALSE 0
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define HOSTNAME_SIZE 256
#define BACKLOG 5
#define STDIN 0
#define CMD_SIZE 100

//Struct for details of each client
typedef struct details clientDetails;
struct details 
{
	int socket;  
	char* port;
	char* IP;
	char* hostName;
	int msgSentCount;
	int msgRecvCount;
	char* status;
	char* bufferMsg;
	char* blockedUser[4];
	int blockedCount;	
};

//Maximum number of clients 4
extern clientDetails* allClients[4];

//Details of present client
extern char* myPort;
extern char* myIP;
extern char* myHostName;
extern int isLoggedIn;
extern int hasLoggedInBefore;

extern int server_fd;

extern int ret_print;
extern int ret_log;
char LOGFILE[FILEPATH_LEN];


//Server Functions

int serverProcess(char* portNum);
int serverInitialize(char* portNum);
void serverEvents(char* msg, int client_fd);
void serverCommands(char* cmd);
void serverLoginEvent();
void serverRefreshEvent(int client_fd);
void serverLogoutEvent(int client_fd);
void serverExitEvent(int client_fd);
void serverSendEvent(int client_fd, char* msg);
void serverBroadcastEvent(int client_fd, char* msg);
void serverBlockEvent(int client_fd, char* msg);
void serverUnblockEvent(int client_fd, char* msg);
void serverStatisticCommand(char* cmd);
void serverBlockedCommand(char* cmd);

//Client Functions

int clientProcess(char* portNum);
void clientInitialize(char* portNum);
int clientLogin(char* server_ip, char* server_port);
void clientRefresh(char* cmd);
void clientLogout(char* cmd);
void clientExit(char* cmd);
void clientSend(char* cmd);
void clientBroadcast(char* cmd);
void clientBlock(char* cmd);
void clientUnblock(char* cmd);
int connect_to_host(char *server_ip, char* server_port);
void msgRecvEvent(char* msg);
void clientProcessResponse(char* msg);
void processRefreshResponse(char* msg);
void processLoginResponse(char* msg);

//Common Functions

void commonAuthor();
void commonIP();
void commonPort();
void commonList();

//Utility Functions

void getMyAddress(char* portNum);
int tokenize(char* cmd, char* tokens[], char* delim);
int isValidIP(char* IP);
int isValidPort(char* port);
void newClient(int client_fd, char* IP, char* port, char* hostname, clientDetails* client);
void getClientInfoBySocket(int client_fd, clientDetails** client);
void getClientInfoByIP(char* IP, clientDetails** client);
int isBlocked(clientDetails* hostClient, clientDetails* destClient);
char* getClientHostName(char* client_ip);
void error(char* cmd);
int sendall(int dest, char *buf, int *len);
int recvall(int host, char* buf, int* len);
void assignToAllClients(char* IP, char* hostname, char* port, int msgsent, int msgrecv, char* status, clientDetails* client);
void sortByPort(clientDetails* allClients[]);
void printAllClients();
void sortByPort2(clientDetails* client[], int n);

//Logging Functions

void cse4589_print_and_log(const char* format, ...);
void cse4589_init_log(char* port);


#endif


