#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef enum {FALSE, TRUE} bool;

#define INF UINT16_MAX	
#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}


/* Global Structures and Lists*/


// 1) For active router connections

struct RouterConn 
{

	/* From Controller INIT Payload */
	uint16_t router_id;
	uint16_t router_port;
	uint16_t data_port;
	uint16_t cost;
	uint32_t router_ip_t;
	char router_ip[16];

	/* To be determined */
	uint16_t nexthop;
	bool isNeighbor;
	int timer_val;
	int data_fd;

	LIST_ENTRY(RouterConn) next;

}*router_conn, *router_conn_temp;

LIST_HEAD(RouterConnHead, RouterConn) router_list;


// 2) For sendfile statistics, data saved per transfer-id

struct SendfileStats
{
	uint8_t transfer_id;
	uint8_t TTL;
	uint16_t seq_num[10240];

	int seq_num_count;

	LIST_ENTRY(SendfileStats) next;

}*sendfile_stats, *sendfile_stats_temp;

LIST_HEAD(SendfileStatsHead, SendfileStats) sendfile_stats_list;


// 3) For all data packets sent/received/routed

struct DataPacket
{
	char* data_packet;
	int data_packet_count;
	LIST_ENTRY(DataPacket) next;
}*data_packet_store;

LIST_HEAD(DataPacketHead, DataPacket) data_packet_list;

// 4) Local copy of router details

struct myInfo
{
	int router_index;
	uint16_t router_id;
	uint16_t router_port;
	uint16_t data_port;
	uint32_t router_ip_t;
	char router_ip[16];
};

struct myInfo *self;

/* Global variables */

uint16_t router_n; 					// Total number of routers
uint16_t update_interval;			// Update interval value received from init
uint16_t cost_mat[5][5];			// Cost matrix - used for Bellman Ford Algorithm
uint16_t neighbor_cost[5];          // Link costs to neighbor. DON'T CHANGE!
int global_file_offset;				// Global file offset used in writing to file from each packet received
int data_packet_count;				// Global count of data packet sent/routed/received
int control_socket; 				// Listening socket for controller connection
int router_socket;					// Listening socket for router connection
int data_socket;					// Listening socket for data connection
bool data_packet_flag;
uint16_t CONTROL_PORT;				// Control port taken as application parameter 

char* last_data_packet;
char* penultimate_data_packet;
char* file_buffer;

int last_data_packet_count;
int penultimate_data_packet_count;

#endif