

#include "../include/allinclude.h"

void init_handler(int sock_index, char* cntrl_init_payload)
{
	//printf("%s\n", "in init_handler");
	uint16_t router_n_t, update_interval_t;

	memcpy(&router_n_t, cntrl_init_payload, sizeof(router_n_t));
	router_n = ntohs(router_n_t);

	memcpy(&update_interval_t, cntrl_init_payload + 2, sizeof(update_interval_t));
	update_interval = ntohs(update_interval_t);

	printf("Update Interval is: %u\n", update_interval);

	int offset = 4, i;
	for(i = 0; i<router_n; i++)
	{
		struct INIT_HEADER *init_header = (struct INIT_HEADER *) (cntrl_init_payload + offset);

		router_conn = malloc(sizeof(struct RouterConn));

		router_conn->router_id = ntohs(init_header->router_id);
		router_conn->router_port = ntohs(init_header->router_port);
		router_conn->data_port = ntohs(init_header->data_port);
		router_conn->cost = ntohs(init_header->cost);
		router_conn->router_ip_t = ntohl(init_header->router_ip);

		getIP(router_conn->router_ip_t, router_conn->router_ip);

		router_list_insert(router_conn);
		offset += 12;
	}

	set_router_details();
	//print_router_id_list();

	i = 0;
	uint16_t cost[router_n];

	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		cost[i++] = router_conn->cost;
	}

	assignToSelf();

	fill_neighbor_cost();

	start_router_socket();
	start_data_socket();

	printf("DATA PACKET COUNT: %d\n", data_packet_count);
	
	int self_index = get_index(self->router_id);

	set_cost(self_index, cost);
	//update_cost_matrix();

	init_response(sock_index);

	

	//printf("%s\n", "leaving init_handler");
}

void init_response(int sock_index)
{
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 1, 0, 0);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
}