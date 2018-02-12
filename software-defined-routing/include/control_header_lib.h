#ifndef CONTROL_HANDLER_LIB_H_
#define CONTROL_HANDLER_LIB_H_

#define CNTRL_HEADER_SIZE 8
#define CNTRL_RESP_HEADER_SIZE 8
#define PER_ROUTER_RESPONSE_LEN 12 
#define DATA_HEADER_SIZE 12
#define FILE_SIZE 1024
#define MAX_FILE_SIZE 10485760
#define EACH_ROUTER_DETAIL_SIZE 8
#define UPDATE_PACKET_SIZE 4
#define ROUTER_UPDATE_CONST_SIZE 8
#define EACH_ROUTER_UPDATE_DETAIL_SIZE 12

#define PACKET_USING_STRUCT 

#ifdef PACKET_USING_STRUCT

    struct __attribute__((__packed__)) CONTROL_HEADER
    {
        uint32_t dest_ip_addr;
        uint8_t control_code;
        uint8_t response_time;
        uint16_t payload_len;
    };

    struct __attribute__((__packed__)) CONTROL_RESPONSE_HEADER
    {
        uint32_t controller_ip_addr;
        uint8_t control_code;
        uint8_t response_code;
        uint16_t payload_len;
    };

    struct __attribute__((__packed__)) INIT_HEADER
    {
        uint16_t router_id;
        uint16_t router_port;
        uint16_t data_port;
        uint16_t cost;
        uint32_t router_ip;
    };

    struct __attribute__((__packed__)) ROUTER_UPDATE_HEADER
    {
        uint32_t router_ip;
        uint16_t router_port;
        uint16_t filler;
        uint16_t router_id;
        uint16_t cost;
    };

    struct __attribute__((__packed__)) ROUTER_RESPONSE_HEADER
    {
        uint32_t router_ip_t;
        uint16_t router_port_t;
        uint16_t filler;
        uint16_t router_id_t;
        uint16_t cost_t;
    };

    struct __attribute__((__packed__)) DATA_HEADER
    {
        uint32_t dest_ip_t;
        uint8_t transfer_id;
        uint8_t TTL;
        uint16_t seq_num;
        uint32_t fin_pad;   
    };

    struct __attribute__((__packed__)) ROUTING_TABLE_RESPONSE_HEADER
    {
        uint16_t router_id;
        uint16_t filler;
        uint16_t nexthop;
        uint16_t cost;
    };

    struct __attribute__((__packed__)) SENDFILE_PAYLOAD
    {
        uint32_t dest_ip;
        uint8_t TTL;
        uint8_t transfer_id;
        uint16_t init_seq_num;
        char filename[];
    };

#endif

char* create_response_header(int sock_index, uint8_t control_code, uint8_t response_code, uint16_t payload_len);

#endif