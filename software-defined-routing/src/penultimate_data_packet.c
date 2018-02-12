
#include "../include/allinclude.h"

void penultimate_data_packet_handler(int sock_index)
{
	printf("DATA PACKET COUNT: %d\n", data_packet_count);
	printf("PENULTIMATE DATA PACKET COUNT: %d\n", penultimate_data_packet_count);
	//printf("data_packet_count: %d\n", data_packet_count);
	char* response;
	char* response_header;
	char* response_payload;
	uint16_t payload_len, response_len;

	payload_len = DATA_HEADER_SIZE + FILE_SIZE;
	response_header = create_response_header(sock_index, 8, 0, payload_len);

	response_payload = (char*)malloc(sizeof(char)*payload_len);

	bzero(response_payload, payload_len);
	printf("******************************PENULTIMATE PAYLOAD*****************************\n");
	printf("%s\n", penultimate_data_packet + DATA_HEADER_SIZE);
	printf("******************************************************************************\n");
	memcpy(response_payload, penultimate_data_packet, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE + payload_len;
	response = (char*)malloc(sizeof(char)*response_len);

	memcpy(response, response_header, CNTRL_RESP_HEADER_SIZE);
	free(response_header);

	memcpy(response + CNTRL_RESP_HEADER_SIZE, response_payload, payload_len);
	free(response_payload);

	sendALL(sock_index, response, response_len);
	free(response);
}
