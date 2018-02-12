
#include "../include/allinclude.h"



void process_update_command(int sock_index, char* cntrl_update_payload)
{
	printf("****************************************\n");
	printf("            RECEIVED UPDATE\n");
	printf("****************************************\n");
	uint16_t router_id, cost;

	memcpy(&router_id, cntrl_update_payload, sizeof(router_id));
	memcpy(&cost, cntrl_update_payload + 2, sizeof(cost));

	router_id = ntohs(router_id);
	cost = ntohs(cost);

	router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    	if(router_conn->router_id == router_id)
    		router_conn->cost = cost;

    fill_neighbor_cost();
    //send_routing_updates();

	cost_mat[get_index(self->router_id)][get_index(router_id)] = cost;
	//reset_cost_mat();

	update_cost_matrix();
	update_response(sock_index);
}


void update_response(int sock_index)
{
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 3, 0, 0);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
}
