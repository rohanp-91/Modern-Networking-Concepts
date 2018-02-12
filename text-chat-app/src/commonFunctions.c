
#include "../include/allinclude.h"


void commonAuthor() {

	cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "rpathak3");
	cse4589_print_and_log("[%s:END]\n", "AUTHOR");
}

void commonIP() {

	cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
	cse4589_print_and_log("IP:%s\n", myIP);
	cse4589_print_and_log("[%s:END]\n", "IP");	

}

void commonPort() {

	cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
	cse4589_print_and_log("PORT:%d\n", atoi(myPort));
	cse4589_print_and_log("[%s:END]\n", "PORT");	

}

void commonList() {

	int i, counter = 1;
	cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
	sortByPort(allClients);
	for(i = 0; i<4; i++) {
		//printf("allClients[%d] -> PORT: %s SOCKET: %d STATUS: %s\n", i, allClients[i]->port, allClients[i]->socket, allClients[i]->status);
		if(!strcmp(allClients[i]->status, "logged-in") && allClients[i]->socket != 0) {
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", counter++, allClients[i]->hostName, allClients[i]->IP, atoi(allClients[i]->port));
		}
	}
	cse4589_print_and_log("[%s:END]\n", "LIST");
}

