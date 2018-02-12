/**
 * @network_util
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
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
 * Network I/O utility functions. send/recvALL are simple wrappers for
 * the underlying send() and recv() system calls to ensure nbytes are always
 * sent/received.
 */

#include "../include/allinclude.h"

void init_globals()
{
    router_n = 0;
    update_interval = 0;
    global_file_offset = 0;
    data_packet_count = 0;

    for(int i = 0; i<5; i++)
        for(int j = 0; j<5; j++)
            cost_mat[i][j] = INF;

    self = (struct myInfo*)malloc(sizeof(struct myInfo));
    last_data_packet = (char*)malloc(sizeof(char)*(DATA_HEADER_SIZE + FILE_SIZE));
    penultimate_data_packet = (char*)malloc(sizeof(char)*(DATA_HEADER_SIZE + FILE_SIZE));
    data_packet_flag = FALSE;
    last_data_packet_count = 0;
    penultimate_data_packet_count = 0;
    file_buffer = (char*)malloc(sizeof(char)*MAX_FILE_SIZE);
    bzero(file_buffer, MAX_FILE_SIZE);
}

void set_router_details()
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        router_conn->timer_val = -1;
        router_conn->data_fd = -1;

        if(router_conn->cost == INF)
        {
         router_conn->nexthop = INF;
         router_conn->isNeighbor = FALSE;
        }
        else if(router_conn->cost == 0)
        {
         router_conn->nexthop = router_conn->router_id;
         router_conn->isNeighbor = FALSE;
        }
        else
        {
         router_conn->nexthop = router_conn->router_id;
         router_conn->isNeighbor = TRUE;
        }
    }
}

ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = recv(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += recv(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = send(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += send(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

void getIP(uint32_t IP_t, char IP[16])
{
    unsigned char data[4];
    for(int i = 0; i<4; i++)
        data[i] = (IP_t >> (i*8)) & 0xFF;
    sprintf(IP, "%d.%d.%d.%d", data[3], data[2], data[1], data[0]);
}

void packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i>>8; *buf++ = i;
}

void packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i>>24;
    *buf++ = i>>16;
    *buf++ = i>>8;  
    *buf++ = i;
}

uint16_t getID(char IP[16], uint16_t port)
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(!strcmp(router_conn->router_ip, IP) && router_conn->router_port)
            return router_conn->router_id;
    }
    return 0;
}

int getTimerByID(uint16_t router_id)
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == router_id)
            return router_conn->timer_val;
    }
    return 0;
}

void setRouterTimer(uint16_t router_id, int timer_fd)
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == router_id)
            router_conn->timer_val = timer_fd;
    }
}

void set_cost(int router_index, uint16_t cost[5])
{
    //printf("%s\n", "in set_cost");
    int i;
    for(i = 0; i < router_n; i++)
        cost_mat[router_index][i] = cost[i];
    //printf("%s\n", "leaving set_cost");  
}

bool isTimer(int sock_index)
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->timer_val == sock_index)
            return TRUE;
    }
    return FALSE;
}


bool isSelfTimer(int timer_fd)
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->timer_val == timer_fd && router_conn->router_id == self->router_id)
            return TRUE;
    }
    return FALSE;
}

void assign_data_fd(int fd)
{
    uint32_t ip_addr_t;
    char ip[16];
    struct sockaddr_in addr;
    socklen_t addr_len;

    addr_len = sizeof(struct sockaddr_in);

    getpeername(fd, (struct sockaddr *)&addr, &addr_len);

    memcpy(&ip_addr_t, &(addr.sin_addr), sizeof(struct in_addr));
    ip_addr_t = ntohl(ip_addr_t);

    getIP(ip_addr_t, ip);

    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(!strcmp(router_conn->router_ip, ip))
            router_conn->data_fd = fd;
    }
}

uint16_t get_nexthop_id(char ip[16])
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(!strcmp(router_conn->router_ip, ip))
            return router_conn->nexthop;
    }
    return 0;
}

bool isConnected(uint16_t nexthop)
{
    //printf("in isConnected...\n");
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == nexthop)
            if(router_conn->data_fd == -1)
            {
                //printf("returning FALSE...\n");
                return FALSE;
            }
    }
    //printf("returning TRUE...\n");
    return TRUE;
}

void connect_to_router(uint16_t nexthop)
{
    //printf("in connect_to_router...\n");
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == nexthop)
        {
            int sock;
            struct sockaddr_in router_addr;
            socklen_t router_addr_len;
            router_addr_len = sizeof(router_addr);

            sock = socket(AF_INET, SOCK_STREAM, 0);
            if(sock < 0)
                ERROR("Error creating socket in connect_to_router!");

            bzero(&router_addr, router_addr_len);
            router_addr.sin_family = AF_INET;
            router_addr.sin_port = htons(router_conn->data_port);
            inet_pton(AF_INET, router_conn->router_ip, &(router_addr.sin_addr));

            if(connect(sock, (struct sockaddr *)&router_addr, router_addr_len) < 0) 
            {
                ERROR("Cannot connect to router!");
                close(sock);
            }
            
            //printf("nexthop data_fd in connect_to_router is: %u\n", sock);
            router_conn->data_fd = sock;
            addToList(sock);
        }
    }
}

int get_nexthop_data_fd(uint16_t nexthop)
{
    //printf("in get_nexthop_data_fd...\n");
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == nexthop)
        {
            //printf("returning nexthop data_fd: %u\n", router_conn->data_fd);
            return router_conn->data_fd;
        }
    }
    return 0;
}

int check_msb(uint32_t n)
{
    int bits;
    uint32_t msb;
    bits = sizeof(uint32_t) * CHAR_BIT;
    msb = 1 << (bits - 1);
    if(n & msb)
        return 1;
    return 0;
}


int get_index(uint16_t router_id)
{
    int i = 0;
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->router_id == router_id)
            return i;
        i++;
    }  

    return -1; 
}

void print_cost_mat()
{
    for(int i = 0; i < router_n; i++)
    {
        for(int j = 0; j < router_n; j++)
        {
            if(cost_mat[i][j] == INF)
                printf("  INF  |");
            else if(cost_mat[i][j] < 10)
                printf("  %u    |", cost_mat[i][j]);
            else
                printf("  %u   |", cost_mat[i][j]);
        }
        printf("\n");
    }
    printf("****************************************\n");
}

void print_router_id_list()
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
        printf("router_id: %u\n", router_conn->router_id);
}

void reassign_self_timer(int timer_fd)
{
    struct itimerspec timer;
    // timerfd_gettime(timer_val, &timer);
    timer.it_value.tv_sec = update_interval;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;

    if(timerfd_settime(timer_fd, 0, &timer, NULL) < 0)
        ERROR("Error SETTING timer value for self!");
}

void print_routing_table()
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        printf("router_id: %u | cost: %u | nexthop: %u\n", router_conn->router_id, router_conn->cost, router_conn->nexthop);
    }
    printf("****************************************\n");
}

void fill_neighbor_cost()
{
    router_conn = malloc(sizeof(struct RouterConn));
    LIST_FOREACH(router_conn, &router_list, next)
    {
        if(router_conn->isNeighbor)
            neighbor_cost[get_index(router_conn->router_id)] = router_conn->cost;
    }
}

void reset_cost_mat()
{
    for(int i = 0; i < router_n; i++)
    {
        for(int j = 0; j < router_n; j++)
        {
            if(i == get_index(self->router_id))
                continue;
            cost_mat[i][j] = INF;
        }
    }
}


void add_to_stats_list(uint8_t transfer_id, uint8_t TTL, uint16_t seq_num)
{
    //printf("in add_to_stats_list\n");
    bool flag = FALSE;
    
    if(LIST_EMPTY(&sendfile_stats_list))
        LIST_INIT(&sendfile_stats_list);
    
    sendfile_stats = malloc(sizeof(struct SendfileStats));
    LIST_FOREACH(sendfile_stats, &sendfile_stats_list, next)
    {   
        
        if(sendfile_stats->transfer_id == transfer_id)
        {
            if(sendfile_stats->seq_num_count == 10240)
                sendfile_stats->seq_num_count = 0;                                 
            sendfile_stats->seq_num[sendfile_stats->seq_num_count++] = seq_num;
            flag = TRUE;
            break;
        }
    }
    
    if(!flag)
    {
        sendfile_stats_temp = malloc(sizeof(struct SendfileStats));
        sendfile_stats_temp->transfer_id = transfer_id;
        sendfile_stats_temp->TTL = TTL;
        sendfile_stats_temp->seq_num[sendfile_stats_temp->seq_num_count++] = seq_num;
        LIST_INSERT_HEAD(&sendfile_stats_list, sendfile_stats_temp, next);
    }   
    //printf("leaving add_to_stats_list\n");
}

void router_list_insert(struct RouterConn *router_conn)
{
    if(LIST_EMPTY(&router_list))
    {
        LIST_INIT(&router_list);
        LIST_INSERT_HEAD(&router_list, router_conn, next);
    }
    else
    {
        LIST_INSERT_HEAD(&router_list, router_conn, next);
    }
}






