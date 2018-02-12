
#include "../include/allinclude.h"

struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
}*data_conn, *data_conn_temp;

LIST_HEAD(DataConnsHead, DataConn) data_conn_list;

void start_data_socket()
{
    printf("%s\n", "data socket listening...");
    int sock;
    struct sockaddr_in data_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        ERROR("Data socket failed!");

    /* Make socket re-usable */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("Data socket setsockopt() failed!");

    bzero(&data_addr, addrlen);

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(self->data_port);

    if(bind(sock, (struct sockaddr *)&data_addr, addrlen) < 0)
        ERROR("Data socket bind failed!");

    if(listen(sock, 5) < 0)
        ERROR("Data socket listen failed!");

    LIST_INIT(&data_conn_list);
    
    data_socket = sock;
    addToList(sock);

    //printf("self->router_id: %u\n", self->router_id);

    //printf("%s\n", "leaving start_data_socket");
}

int new_data_conn(int sock_index)
{
	int fdaccept;
	socklen_t router_addr_len;
    struct sockaddr_in router_addr;

    router_addr_len = sizeof(router_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&router_addr, &router_addr_len);
    if(fdaccept < 0)
        ERROR("Data connection accept failed");

    /* Insert into list of active data connections */
    data_conn = malloc(sizeof(struct DataConn));
    data_conn->sockfd = fdaccept;
    LIST_INSERT_HEAD(&data_conn_list, data_conn, next);

    assign_data_fd(fdaccept);

    return fdaccept;
}

void process_data_packet(int sock_index)
{
    //printf("in process_data_packet...\n");
	int i = 0;
	char* data_header;
    char* data_payload;
    char* data_packet;
    //char* file_buffer;
	uint32_t dest_ip_t, fin_pad;
	uint8_t transfer_id, TTL;
	uint16_t seq_num;
    uint16_t data_packet_len;
    bool TTL_flag = FALSE;
	char dest_ip[16];

    // file_buffer = (char*)malloc(sizeof(char)*MAX_FILE_SIZE);
    //memset(file_buffer, '\0', MAX_FILE_SIZE);

    // Keep a counter...
    data_packet_count++;

    // Recieve data header...
	data_header = (char*)malloc(sizeof(char)*DATA_HEADER_SIZE);
	bzero(data_header, DATA_HEADER_SIZE);

	if(recvALL(sock_index, data_header, DATA_HEADER_SIZE) < 0)
    	ERROR("Data header recieve error!");

    // Extract values from header...
    struct DATA_HEADER *d_header = (struct DATA_HEADER *) data_header;
    dest_ip_t = ntohl(d_header->dest_ip_t);
    transfer_id = d_header->transfer_id;
    TTL = d_header->TTL;
    seq_num = ntohs(d_header->seq_num);
    fin_pad = ntohl(d_header->fin_pad);
    getIP(dest_ip_t, dest_ip);

    // Check if TTL - 1 = 0
    TTL = TTL - 1;
    if(TTL > 0) 
    {
        d_header->TTL = TTL;
        TTL_flag = TRUE;
    }
    
    //printf("transfer_id: %u\n", transfer_id);
    //printf("TTL after decrement: %u\n", TTL);

    // Receive data payload...
    data_payload = (char*)malloc(sizeof(char)*FILE_SIZE);
    bzero(data_payload, FILE_SIZE);

    if(recvALL(sock_index, data_payload, FILE_SIZE) < 0)
        ERROR("Data payload recieve error!");

    // Create data packet...
    data_packet_len = DATA_HEADER_SIZE + FILE_SIZE;
    data_packet = (char*)malloc(sizeof(char)*data_packet_len);

    memcpy(data_packet, data_header, DATA_HEADER_SIZE);
    memcpy(data_packet + DATA_HEADER_SIZE, data_payload, FILE_SIZE);

    if(TTL_flag)
    {

        if(!data_packet_flag)
        {
            //printf("in data_packet_flag false...\n");
            memset(last_data_packet, '\0', data_packet_len);
            memcpy(last_data_packet, data_packet, data_packet_len);
            last_data_packet_count++;
            data_packet_flag = TRUE;
        }
        else
        {
            //printf("in data_packet_flag true...\n");
            memset(penultimate_data_packet, '\0', data_packet_len);
            memcpy(penultimate_data_packet, last_data_packet, data_packet_len);
            penultimate_data_packet_count++;
            memset(last_data_packet, '\0', data_packet_len);
            memcpy(last_data_packet, data_packet, data_packet_len);
            last_data_packet_count++;
        }

        if(check_msb(fin_pad))
            data_packet_flag = FALSE;
        
    }

    // Check if the destination is self or needs to be forwarded...
    if(TTL_flag)
    {
        if(!strcmp(dest_ip, self->router_ip))
        {
            memcpy(file_buffer + global_file_offset, data_payload, FILE_SIZE);
            global_file_offset += FILE_SIZE;
            if(check_msb(fin_pad))
            {
                char filename[15];
                sprintf(filename, "file-%d", transfer_id);
                FILE *ptr;
                ptr = fopen(filename, "wb");
                fwrite(file_buffer, global_file_offset, 1, ptr);
                fclose(ptr);
                printf("global_file_offset: %d\n", global_file_offset);
                global_file_offset = 0;
                bzero(file_buffer, MAX_FILE_SIZE);
            }	
        }
        else
        {
    		uint16_t nexthop = get_nexthop_id(dest_ip);
    		
    		if(!isConnected(nexthop))
            {
                printf("not connected...\n");
    			connect_to_router(nexthop);
            }

            int data_fd = get_nexthop_data_fd(nexthop);
            sendALL(data_fd, data_packet, data_packet_len);
        }
    }

    // Add statistics to list...
    if(TTL_flag) 
        add_to_stats_list(transfer_id, TTL, seq_num);
    
    free(data_header);
    free(data_payload);
    free(data_packet);
}

bool isData(int sock_index)
{
    data_conn = malloc(sizeof(struct DataConn));
    LIST_FOREACH(data_conn, &data_conn_list, next)
    {
        if(data_conn->sockfd == sock_index)
            return TRUE;
    }
    return FALSE;
}
















