
#include "../include/allinclude.h"


void sendfile_handler(int sock_index, char* cntrl_sendfile_payload, uint16_t payload_len)
{
	uint32_t dest_ip_t;
	char dest_ip[16];
	uint8_t TTL;
	uint8_t transfer_id;
	uint16_t init_seq_num;
	char* filename;
	char* file_buffer;
	int filesize;
	char* data_packet;
	uint16_t filename_len;

	struct SENDFILE_PAYLOAD *sendfile = (struct SENDFILE_PAYLOAD *) cntrl_sendfile_payload;
	dest_ip_t = ntohl(sendfile->dest_ip);
	getIP(dest_ip_t, dest_ip);
	TTL = sendfile->TTL;
	transfer_id = sendfile->transfer_id;
	init_seq_num = ntohs(sendfile->init_seq_num);

	filename_len = payload_len - 8;
	
	filename = (char*)malloc(sizeof(char)*(filename_len + 1));
	bzero(filename, filename_len + 1);

	memcpy(filename, &(sendfile->filename), filename_len);
	memset(filename + filename_len + 1, '\0', 1);
	printf("filename: %s\n", filename);

	FILE *ptr;
	ptr = fopen(filename, "rb");
	if(!ptr)
	{
		ERROR("Error opening file for reading!");
		return;
	}

	// Following from SO: https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c //
	fseek(ptr, 0L, SEEK_END);
	filesize = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);
	//......//

	int packets_n = filesize/FILE_SIZE;
	printf("packets_n: %d\n", packets_n);
	file_buffer = (char*)malloc(sizeof(char)*filesize);
	fread(file_buffer, filesize, 1, ptr);
	fclose(ptr);
	printf("here 1\n");
	uint16_t nexthop = get_nexthop_id(dest_ip);		
	if(!isConnected(nexthop)) 
	{
		printf("here 2\n");
		printf("not connected...\n");
		connect_to_router(nexthop);
	}
    int data_fd = get_nexthop_data_fd(nexthop);

    sendfile_stats = malloc(sizeof(struct SendfileStats));
 	sendfile_stats->transfer_id = transfer_id;
 	sendfile_stats->TTL = TTL;
 	sendfile_stats->seq_num_count = packets_n;

    uint32_t fin_pad = 0 | (1UL << 31);
    int offset = 0, counter = 0;
    uint16_t data_packet_len = DATA_HEADER_SIZE + FILE_SIZE;

	for(int i = 1; i <= packets_n - 1; i++)
	{
		//printf("creating and sending packets till second last packet...\n");
		
		data_packet = (char*)malloc(sizeof(char)*(data_packet_len));
		bzero(data_packet, data_packet_len);

		struct DATA_HEADER *d_header = (struct DATA_HEADER *) data_packet;
		d_header->dest_ip_t = htonl(dest_ip_t);
		d_header->transfer_id = transfer_id;
		d_header->TTL = TTL;
		d_header->seq_num = htons(init_seq_num);
		d_header->fin_pad = 0;

		
		memcpy(data_packet + DATA_HEADER_SIZE, file_buffer + offset, FILE_SIZE);
		sendALL(data_fd, data_packet, data_packet_len);

		offset += FILE_SIZE;
		sendfile_stats->seq_num[counter++] = init_seq_num++;

        penultimate_data_packet_count++;
		if(i == packets_n - 1)
		{
			bzero(penultimate_data_packet, data_packet_len);
			memcpy(penultimate_data_packet, data_packet, data_packet_len);
		}
	}
	
	data_packet = (char*)malloc(sizeof(char)*(DATA_HEADER_SIZE + FILE_SIZE));
	bzero(data_packet, data_packet_len);

	struct DATA_HEADER *d_header = (struct DATA_HEADER *) data_packet;
	d_header->dest_ip_t = htonl(dest_ip_t);
	d_header->transfer_id = transfer_id;
	d_header->TTL = TTL;
	d_header->seq_num = htons(init_seq_num);
	d_header->fin_pad = htonl(fin_pad);

	memcpy(data_packet + DATA_HEADER_SIZE, file_buffer + offset, FILE_SIZE);
	sendALL(data_fd, data_packet, data_packet_len);

	bzero(last_data_packet, data_packet_len);
	memcpy(last_data_packet, data_packet, data_packet_len);

	free(data_packet);

	last_data_packet_count = penultimate_data_packet_count + 1;
	sendfile_stats->seq_num[counter] = init_seq_num;
	if(LIST_EMPTY(&sendfile_stats_list)) 
	{
		LIST_INIT(&sendfile_stats_list);
		LIST_INSERT_HEAD(&sendfile_stats_list, sendfile_stats, next);
	}
	else
		LIST_INSERT_HEAD(&sendfile_stats_list, sendfile_stats, next);

	sendfile_response(sock_index);

	data_packet_count += packets_n;

	printf("****************************************\n");
	printf("*       DONE SENDING ALL PACKETS       *\n");
	printf("****************************************\n");
}

void sendfile_response(int sock_index)
{
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 5, 0, 0);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
}