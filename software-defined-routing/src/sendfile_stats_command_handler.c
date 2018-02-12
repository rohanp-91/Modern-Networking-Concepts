
#include "../include/allinclude.h"



void sendfile_stats_handler(int sock_index, char* cntrl_sendfile_stats_payload)
{
	printf("*** in sendfile_stats_handler ***\n");
	char* response;
	char* response_header;
	char* response_payload;
	int seq_num_count, i;
	uint8_t TTL;
	uint16_t seq_num[10240], padding;

	uint8_t transfer_id;
	memcpy(&transfer_id, cntrl_sendfile_stats_payload, sizeof(transfer_id));

	printf("transfer_id: %u\n", transfer_id);

	//printf("here 1\n");

	sendfile_stats = malloc(sizeof(struct SendfileStats));
	LIST_FOREACH(sendfile_stats, &sendfile_stats_list, next)
	{
		//printf("here in loop\n");
		if(sendfile_stats->transfer_id == transfer_id)
		{
			TTL = sendfile_stats->TTL;
			printf("TTL: %u\n", TTL);
			seq_num_count = sendfile_stats->seq_num_count;
			printf("seq_num_count: %d\n", seq_num_count);
			for(i = 0; i < seq_num_count; i++)
			{
				//printf("%u\n", sendfile_stats->seq_num[i]);
				seq_num[i] = sendfile_stats->seq_num[i];
			}
		}
	}

	//printf("here 2\n");

	uint16_t payload_len = (sizeof(uint16_t) * seq_num_count) + 4;
	response_payload = (char*) malloc(sizeof(char)*payload_len);

	response_header = create_response_header(sock_index, 6, 0, payload_len);

	padding = 0x00;

	memcpy(response_payload, &transfer_id, sizeof(uint8_t));
	memcpy(response_payload + 1, &TTL, sizeof(uint8_t));
	packi16(response_payload + 2, padding);

	int offset = 4;

	//printf("creating sendfile_stats payload: \n");
	for(i = 0; i < seq_num_count; i++)
	{	
		if(i == seq_num_count - 1)
			printf("last seq num added to packet: %u\n", seq_num[i]);
		packi16(response_payload + offset, seq_num[i]);
		offset += 2;
	}

	//printf("here 3\n");
	uint16_t response_len = CNTRL_RESP_HEADER_SIZE + payload_len;
	response = (char*)malloc(sizeof(char)*response_len);

	memcpy(response, response_header, CNTRL_RESP_HEADER_SIZE);
	free(response_header);
	memcpy(response + CNTRL_RESP_HEADER_SIZE, response_payload, payload_len);
	free(response_payload);

	//printf("response: %s\n", response);

	sendALL(sock_index, response, response_len);
	free(response);
}