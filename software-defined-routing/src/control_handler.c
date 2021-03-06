/**
 * @control_handler
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
 * Handler for the control plane.
 */

#include "../include/allinclude.h"

/* Linked List for active control connections */
struct ControlConn
{
    int sockfd;
    LIST_ENTRY(ControlConn) next;
}*connection, *conn_temp;

LIST_HEAD(ControlConnsHead, ControlConn) control_conn_list;

int create_control_sock()
{
    int sock;
    struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        ERROR("socket() failed");

    /* Make socket re-usable */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    bzero(&control_addr, addrlen);

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(CONTROL_PORT);

    if(bind(sock, (struct sockaddr *)&control_addr, addrlen) < 0)
        ERROR("bind() failed");

    if(listen(sock, 5) < 0)
        ERROR("listen() failed");

    LIST_INIT(&control_conn_list);

    return sock;
}

int new_control_conn(int sock_index)
{
    int fdaccept, caddr_len;
    struct sockaddr_in remote_controller_addr;

    caddr_len = sizeof(remote_controller_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&remote_controller_addr, &caddr_len);
    if(fdaccept < 0)
        ERROR("accept() failed");

    /* Insert into list of active control connections */
    connection = malloc(sizeof(struct ControlConn));
    connection->sockfd = fdaccept;
    LIST_INSERT_HEAD(&control_conn_list, connection, next);

    return fdaccept;
}

void remove_control_conn(int sock_index)
{
    LIST_FOREACH(connection, &control_conn_list, next) 
    {
        if(connection->sockfd == sock_index) 
        {
            LIST_REMOVE(connection, next); // this may be unsafe?
            free(connection);
        }
    }

    close(sock_index);
}

bool isControl(int sock_index)
{
    LIST_FOREACH(connection, &control_conn_list, next)
        if(connection->sockfd == sock_index) return TRUE;
    return FALSE;
}

bool control_recv_hook(int sock_index)
{
    char *cntrl_header, *cntrl_payload;
    uint8_t control_code;
    uint8_t response_time;
    uint16_t payload_len;

    /* Get control header */
    cntrl_header = (char *) malloc(sizeof(char)*CNTRL_HEADER_SIZE);
    bzero(cntrl_header, CNTRL_HEADER_SIZE);

    if(recvALL(sock_index, cntrl_header, CNTRL_HEADER_SIZE) < 0)
    {
        remove_control_conn(sock_index);
        free(cntrl_header);
        return FALSE;
    }
    
    /* Get control code and payload length from the header */
    
    struct CONTROL_HEADER *header = (struct CONTROL_HEADER *) cntrl_header;
    control_code = header->control_code;
    payload_len = ntohs(header->payload_len);
    response_time = header->response_time;

    free(cntrl_header);

    /* Get control payload */
    if(payload_len != 0)
    {
        cntrl_payload = (char *) malloc(sizeof(char)*payload_len);
        bzero(cntrl_payload, payload_len);

        if(recvALL(sock_index, cntrl_payload, payload_len) < 0)
        {
            remove_control_conn(sock_index);
            free(cntrl_payload);
            return FALSE;
        }
    }

    /* Triage on control_code */
    switch(control_code)
    {
        case 0: author_response(sock_index);
                break;

        case 1: init_handler(sock_index, cntrl_payload);
                break;

        case 2: routing_table_response(sock_index);
                break;

        case 3: process_update_command(sock_index, cntrl_payload);
                break;        
 
        case 4: crash_response(sock_index);
                exit(0);
                break;

        case 5: sendfile_handler(sock_index, cntrl_payload, payload_len);
                break;

        case 6: sendfile_stats_handler(sock_index, cntrl_payload);
                printf("here after sendfile_stats_handler\n");
                break;

        case 7: last_data_packet_handler(sock_index);
                break;

        case 8: penultimate_data_packet_handler(sock_index);
                break;

    }

    if(payload_len != 0) free(cntrl_payload);
    return TRUE;
}


void crash_response(int sock_index)
{
    char *cntrl_response_header;
    cntrl_response_header = create_response_header(sock_index, 4, 0, 0);
    //router_n--;
    sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
}