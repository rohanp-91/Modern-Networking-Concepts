#include "../include/allinclude.h"
 
int clientLogin(char* server_ip, char* server_port) {

	//printf("%s\n", "entered clientLogin");
	int i=0, count=0;

	server_fd = connect_to_host(server_ip, server_port);
	if(server_fd < 0) {
		error("LOGIN");
		close(server_fd);
		server_fd = -1;
		return -1;
	}
	char* msg = (char*)malloc(MSG_SIZE);
	//memset(msg, '\0', sizeof(*msg));
	//if(!strcmp(myPort, "200"))
	//	myIP = "127.0.0.1";
	sprintf(msg, "%s\n%s\n", myIP, myPort);
	//printf("sending from clientLogin: %s\n", msg);
	int lengthOfMsg = strlen(msg);
	sendall(server_fd, msg, &lengthOfMsg);
	//printf("total sent: %d\n", lengthOfMsg);
	return server_fd;
	
}

void clientRefresh(char* cmd) {

	//printf("%s\n", "entered client refresh");
	send(server_fd, cmd, strlen(cmd), 0);
}


void clientLogout(char* cmd) {

	if(send(server_fd, cmd, sizeof(cmd), 0) < 0)
		perror("logout send error");
	cse4589_print_and_log("[%s:SUCCESS]\n", "LOGOUT");;
	cse4589_print_and_log("[%s:END]\n", "LOGOUT");
}
	

void clientExit(char* cmd) {

	send(server_fd, cmd, strlen(cmd), 0);
	
}

void clientSend(char* cmd) {

	//printf("msg sent to server: %s\n", cmd);
    int lengthOfMsg = strlen(cmd);
	sendall(server_fd, cmd, &lengthOfMsg);
	cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
	cse4589_print_and_log("[%s:END]\n", "SEND");;

}
	
void clientBroadcast(char* cmd) {

	//printf("brodcast command from client: %s\n", cmd);
	int lengthOfMsg = strlen(cmd);
	sendall(server_fd, cmd, &lengthOfMsg);
	cse4589_print_and_log("[%s:SUCCESS]\n", "BROADCAST");
	cse4589_print_and_log("[%s:END]\n", "BROADCAST");
}
	
void clientBlock(char* cmd) {

	printf("msg from clientBlock: %s\n", cmd);
	int lengthOfMsg = strlen(cmd);
	sendall(server_fd, cmd, &lengthOfMsg);
	//cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCK");
	//cse4589_print_and_log("[%s:END]\n", "BLOCK");
}
	
void clientUnblock(char* cmd) {

	printf("msg from clientUnblock: %s\n", cmd);
	int lengthOfMsg = strlen(cmd);
	sendall(server_fd, cmd, &lengthOfMsg);
	
}
	
void msgRecvEvent(char* msg) {
	
	cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");

	cse4589_print_and_log("[%s:END]\n", "RECEIVED");
}


int connect_to_host(char* server_ip, char* server_port)
{
	struct sockaddr_in remote_server_addr;
	int fdsocket;
	uint16_t port = atoi(server_port);
	socklen_t len = sizeof(remote_server_addr);

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0) {
    	printf("%s\n", "socket error");
    	return -1;
    }

    memset(&remote_server_addr, 0, sizeof(remote_server_addr));

    remote_server_addr.sin_family = AF_INET;
    remote_server_addr.sin_addr.s_addr = inet_addr(server_ip);
    remote_server_addr.sin_port = htons(port);
    
    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, len) < 0) {
    	perror("connect error");
        return -1;
    }

    return fdsocket;
}
