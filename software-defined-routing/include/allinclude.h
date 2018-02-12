
/* ...........LIBRARIES ............ */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include <limits.h>


/*...........OTHER HEADER FILES.........*/

#include "../include/global.h"
#include "../include/control_header_lib.h"



/*...........FUNCTIONS.................*/

// AUTHOR

void author_response(int sock_index);

// INIT

void init_handler(int sock_index, char* cntrl_init_payload);
void init_response(int sock_index);

// ROUTING TABLE

void routing_table_response(int sock_index);

// DISTANCE VECTOR

void assignToSelf();
void update_cost_matrix();

// DATA HANDLER

void start_data_socket();
int new_data_conn(int sock_index);
void process_data_packet(int sock_index);
bool isData(int sock_index);

// ROUTER PROCESS HANDLER

void start_router_socket();
void recv_router_update(int sock_index);
void process_router_data(char* payload);
void send_routing_updates();
void update_crashed_router(int timer_fd);

// SENDFILE COMMAND HANDLER

void sendfile_handler(int sock_index, char* cntrl_sendfile_payload, uint16_t payload_len);
void sendfile_response(int sock_index);

// SENDFILE STATS COMMAND HANDLER

void sendfile_stats_handler(int sock_index, char* cntrl_sendifle_stats_payload);

// LAST DATA PACKET

void last_data_packet_handler(int sock_index);

// PENULTIMATE DATA PACKET

void penultimate_data_packet_handler(int sock_index);

// UPDATE COMMAND HANDLER

void process_update_command(int sock_index, char* cntrl_update_payload);
void update_response(int sock_index);

// CONNECTION MANAGER

void main_loop();
void init();
void addToList(int fd);

// CONTROL HANDLER

int create_control_sock();
int new_control_conn(int sock_index);
void remove_control_conn(int sock_index);
bool isControl(int sock_index);
bool control_recv_hook(int sock_index);
void crash_response(int sock_index);

// NETWORK UTIL

void init_globals();
void set_router_details();
ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes);
ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes);
void getIP(uint32_t IP_t, char IP[16]);
void packi16(unsigned char *buf, unsigned int i);
void packi32(unsigned char *buf, unsigned long int i);
uint16_t getID(char IP[16], uint16_t port);
int getTimerByID(uint16_t router_id);
void setRouterTimer(uint16_t router_id, int timer_fd);
void set_cost(int router_index, uint16_t cost[5]);
bool isTimer(int sock_index);
bool isSelfTimer(int timer_fd);
void assign_data_fd(int fd);
uint16_t get_nexthop_id(char ip[16]);
bool isConnected(uint16_t nexthop);
void connect_to_router(uint16_t nexthop);
int get_nexthop_data_fd(uint16_t nexthop);
int check_msb(uint32_t n);
int get_index(uint16_t router_id);
void reassign_self_timer(int timer_fd);
void print_all_costs();
void make_symmetric();
void print_routing_table();
void fill_neighbor_cost();
void print_cost_mat();
void print_router_id_list();
void reset_cost_mat();
void add_to_stats_list(uint8_t transfer_id, uint8_t TTL, uint16_t seq_num);
void router_list_insert(struct RouterConn *router_conn);











