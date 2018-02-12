
#include "../include/allinclude.h"


void start_router_socket()
{
	printf("%s\n", "router socket listening...");
    int sock;
    struct sockaddr_in router_addr;
    socklen_t router_addr_len;

    router_addr_len = sizeof(struct sockaddr_in);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
        ERROR("Router socket failed!");

    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("Router socket setsockopt() failed!");

    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    router_addr.sin_port = htons(self->router_port);

    if((bind(sock, (struct sockaddr*)&router_addr, router_addr_len)) < 0)
        ERROR("Router socket bind failed!");

    router_socket = sock;
    addToList(sock);

    //printf("self->router_id: %u\n", self->router_id);
    //printf("%s\n", "leaving start_router_socket");
}


void recv_router_update(int sock_index)
{
	//printf("%s\n", "in udp_recvfrom");
	struct sockaddr_in router_addr;
	socklen_t router_addr_len;
	int payload_len;
	char* udp_payload;

	payload_len = ROUTER_UPDATE_CONST_SIZE + (router_n * EACH_ROUTER_UPDATE_DETAIL_SIZE);
	udp_payload = (char*)malloc(payload_len);

	int recv = recvfrom(sock_index, udp_payload, payload_len, 0, (struct sockaddr*)&router_addr, &router_addr_len);
	if(recv <= 0)
		ERROR("UDP recvfrom failed!");

	process_router_data(udp_payload);
	free(udp_payload);
	//printf("%s\n", "leaving udp_recvfrom");
}

void process_router_data(char* payload)
{
	//printf("%s\n", "in process_router_data");
	int i, offset = ROUTER_UPDATE_CONST_SIZE;
	uint16_t cost[router_n];
	uint16_t source_port_t, source_port;
	uint16_t router_id;
	uint32_t source_ip_t;
	char source_ip[16];

	memcpy(&source_port_t, payload + 2, sizeof(source_port_t));
	source_port = ntohs(source_port_t);

	memcpy(&source_ip_t, payload + 4, sizeof(source_ip_t));
	source_ip_t = ntohl(source_ip_t);
	getIP(source_ip_t, source_ip);

	uint16_t sender_id = getID(source_ip, source_port);

	for(i = 0; i<router_n; i++)
	{
		struct ROUTER_UPDATE_HEADER *router_header = (struct ROUTER_UPDATE_HEADER *) (payload + offset);
		router_id = ntohs(router_header->router_id);
		int router_index = get_index(router_id);
		cost[router_index] = ntohs(router_header->cost);
		offset += EACH_ROUTER_UPDATE_DETAIL_SIZE;
	}

	//printf("Received router data from: %u\n", sender_id);

	// Got idea of timerfd from Stack Overflow: https://stackoverflow.com/questions/2328127/select-able-timers //
	int timer_val = getTimerByID(sender_id);
	
	if(timer_val == -1)
	{
		int timer_fd;
		struct itimerspec timer;
		timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

		timer.it_value.tv_sec = update_interval * 3;
		timer.it_value.tv_nsec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_nsec = 0;

		if(timerfd_settime(timer_fd, 0, &timer, NULL) < 0)
			ERROR("Error CREATING timer value for router!");

		setRouterTimer(sender_id, timer_fd);
		addToList(timer_fd);
	}
	else
	{
		//printf("%s\n", "in timer_val != -1");
		struct itimerspec timer;
		// timerfd_gettime(timer_val, &timer);
		timer.it_value.tv_sec = update_interval * 3;
		timer.it_value.tv_nsec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_nsec = 0;

		if(timerfd_settime(timer_val, 0, &timer, NULL) < 0)
			ERROR("Error SETTING timer value for router!");
	}

	int sender_index = get_index(sender_id);
	//printf("Cost before setting, in udp recv:\n");
	//for(int i = 0; i<router_n; i++)
	//	printf("%u\n", cost[i]);
	set_cost(sender_index, cost);
	update_cost_matrix();
	//printf("%s\n", "leaving process_router_data");
}

void send_routing_updates()
{
	//printf("%s\n", "in send_routing_updates");
	char* response;
	int offset = 8;
	int total_response_len = offset + (PER_ROUTER_RESPONSE_LEN * router_n);

	response = (char*)malloc(sizeof(char)*total_response_len);

	packi16(response, router_n);
	packi16(response + 2, self->router_port);
	packi32(response + 4, self->router_ip_t);

	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		struct ROUTER_RESPONSE_HEADER *router_response;
		router_response = (struct ROUTER_RESPONSE_HEADER *) (response + offset);

		router_response->router_ip_t = htonl(router_conn->router_ip_t);
		router_response->router_port_t = htons(router_conn->router_port);
		router_response->filler = 0x00;
		router_response->router_id_t = htons(router_conn->router_id);
		router_response->cost_t = htons(router_conn->cost);

		offset += PER_ROUTER_RESPONSE_LEN;
	}

	LIST_FOREACH(router_conn, &router_list, next)
	{
		if(router_conn->isNeighbor)
		{
			//printf("Sending updates to router: %u\n", router_conn->router_id);
			int sock;
			struct sockaddr_in router_addr;
			socklen_t router_addr_len;
			router_addr_len = sizeof(router_addr);

			if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
				ERROR("UDP send socket error!");

			bzero(&router_addr, router_addr_len);

			router_addr.sin_family = AF_INET;
			router_addr.sin_port = htons(router_conn->router_port);
			inet_pton(AF_INET, router_conn->router_ip, &(router_addr.sin_addr));

			if(sendto(sock, response, total_response_len, 0, (struct sockaddr*)&router_addr, router_addr_len) <= 0)
				ERROR("UDP send failed!");

			close(sock);
		}
	}
	//printf("%s\n", "leaving send_routing_updates");
}

void update_crashed_router(int timer_fd)
{
	uint16_t crashed_router_id;
	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		if(router_conn->timer_val == timer_fd)
		{
			printf("Crashed router is: %u\n", router_conn->router_id);
			router_conn->isNeighbor = FALSE;
                        router_conn->cost = INF;
                        cost_mat[get_index(self->router_id)][get_index(router_conn->router_id)] = INF;
			break;
			//printf("out of if...\n");
		}
	}	
	//printf("before update_cost_matrix...\n");
	update_cost_matrix();
	//printf("leaving update_crashed_router...\n");
}
