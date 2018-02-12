

#include "../include/allinclude.h"

void routing_table_response(int sock_index)
{
	printf("in routing_table_response\n");
	int offset = 0;
	char* response;
	char* response_payload;
	char* response_header;
	uint16_t payload_len, response_len;

	payload_len = router_n * EACH_ROUTER_DETAIL_SIZE;
	printf("payload_len: %u\n", payload_len);

	// Create response header
	response_header = create_response_header(sock_index, 2, 0, payload_len);
	printf("here 1\n");
	// Create response payload ...
	response_payload = (char*)malloc(sizeof(char)*payload_len);
	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		struct ROUTING_TABLE_RESPONSE_HEADER *rtable_header = (struct ROUTING_TABLE_RESPONSE_HEADER *) (response_payload + offset);
		rtable_header->router_id = htons(router_conn->router_id);
		rtable_header->filler = 0x00;
		rtable_header->nexthop = htons(router_conn->nexthop);
		rtable_header->cost = htons(router_conn->cost);

		offset += EACH_ROUTER_DETAIL_SIZE;
	} 
	// ... end of creating response payload
	printf("here 2\n");
	// Create response packet
	response_len = payload_len + CNTRL_RESP_HEADER_SIZE;
	printf("response_len: %u\n", response_len);
	response = (char*)malloc(sizeof(char)*response_len);
	
	memcpy(response, response_header, CNTRL_RESP_HEADER_SIZE);
	free(response_header);

	memcpy(response + CNTRL_RESP_HEADER_SIZE, response_payload, payload_len);
	free(response_payload);

	if(sendALL(sock_index, response, response_len) < 0)
		ERROR("Error sending routing table!");
	
	free(response);
	//printf("leaving routing_table_response\n");
}
