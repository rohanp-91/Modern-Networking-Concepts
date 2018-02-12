/**
 * @rpathak3_assignment1
 * @author  Rohan Pathak <rpathak3@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "../include/allinclude.h"


clientDetails* allClients[4];

char* myPort;
char* myIP;
char* myHostName;
int isLoggedIn;
int hasLoggedInBefore;

int main(int argc, char *argv[])
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));
	int i, j;
	for(i = 0; i<4; i++) {

		allClients[i] = (clientDetails*)malloc(sizeof(clientDetails)); 
		allClients[i]->socket = 0;  
		allClients[i]->port = (char*)malloc(6);
		allClients[i]->IP = (char*)malloc(16);
		allClients[i]->hostName = (char*)malloc(256);
		allClients[i]->msgSentCount = 0;
		allClients[i]->msgRecvCount = 0;	
		allClients[i]->status = (char*)malloc(11);
	 	allClients[i]->bufferMsg = (char*)malloc(1024);
	 	strcat(allClients[i]->bufferMsg, "");
		for(j = 0; j<4; j++)
	 		allClients[i]->blockedUser[j] = (char*)malloc(16);
	 	allClients[i]->blockedCount = 0;	
	}
	//for (i = 0; i < 4; i++)
	//	printf("IP in main: %s\n", allClients[i]->IP);	

	if(argc != 3) {

		printf("Usage:%s [s/c] [port]\n", argv[0]);
		exit(-1);
	}

	/*Start Here*/
	if(*argv[1] == 's') {
		serverProcess(argv[2]);
	}

	else if(*argv[1] == 'c') {
		clientProcess(argv[2]);
	}

	else {
		printf("Error\n");
		return -1;
	}
	free(myIP);
	free(myHostName);
	for(i = 0;i<4; i++) {
		free(allClients[i]->port);
		free(allClients[i]->IP);
		free(allClients[i]->hostName);
		free(allClients[i]->bufferMsg);
		free(allClients[i]->status);
		for(j=0; j<4; j++)
			free(allClients[i]->blockedUser[j]);
	}
	return 0;
}


