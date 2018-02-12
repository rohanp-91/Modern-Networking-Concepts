#define TRUE 1
#define FALSE 0
#define MAXPOSSIBLETIMEOUT 

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

float increment = 15.0;
int winsize;

int winbase;
int nextseq;
int send_counter;
struct pkt send_packets[1000];
float timers[1000];
int acked[1000];
int unackedPkt;

char msg[20];
struct pkt recv_packets[1000];
int inorder[1000];
int recv_counter;

void checkUnackedPkt() {

	int unacked = winbase;
	for(int i = winbase; i < winbase + winsize; i++) {
		if(acked[i] != 1 && timers[i] != 0) {
			unacked = i;
			break;
		}
	}	
	unackedPkt = unacked;
}


void setChecksum(struct pkt* packet) {

	packet->checksum = 0;
	packet->checksum += (packet->seqnum + packet->acknum);
	for(int i = 0; i<20; i++) 
		packet->checksum += (int)packet->payload[i];

}

int getChecksum(struct pkt packet) {

	int checksum = 0;
	checksum += (packet.seqnum + packet.acknum);
	for(int i = 0; i<20; i++) 
		checksum += (int)packet.payload[i];
    return checksum;
}

int isValidChecksum(struct pkt packet) {

	//printf("packet checksum: %d\n", packet.checksum);
	//printf("get checksum: %d\n", getChecksum(packet));
	if(packet.checksum != getChecksum(packet))
		return FALSE;
	return TRUE;
	
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	struct pkt packet;
	packet.acknum = 0;
	packet.seqnum = 0;
	memset(&packet.payload, '\0', 20);
	memcpy((char*)&packet.payload, (char*)&message.data, 20);
	memset(&send_packets[send_counter].payload, '\0', 20);
	send_packets[send_counter] = packet;
	send_counter++;
	if(nextseq < winbase + winsize) {
		send_packets[nextseq].seqnum = nextseq;
		setChecksum(&send_packets[nextseq]);
		tolayer3(0, send_packets[nextseq]);

		timers[nextseq] = get_sim_time();

		if(winbase == nextseq) {
			starttimer(0, increment);
			unackedPkt = winbase;
		}
		nextseq++;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(!isValidChecksum(packet))
		printf("ACK checksum does not match!\n");
	else {
		printf("ACK received for sequence number: %d\n", packet.acknum);
		acked[packet.acknum] = 1;
		while(acked[winbase] == 1)
			winbase++;
		if(unackedPkt == packet.acknum) {
			stoptimer(0);
			checkUnackedPkt();
			if(timers[unackedPkt] != 0) {
				float localtime = get_sim_time();
				float unackedTimeLeft = timers[unackedPkt] + increment - localtime;
				printf("unackedTimeLeft: %f\n", unackedTimeLeft);
				starttimer(0, unackedTimeLeft);
			}
		}
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	tolayer3(0, send_packets[unackedPkt]);

	float localtime = get_sim_time();
	timers[unackedPkt] = localtime;
	checkUnackedPkt();
	float unackedTimeLeft = timers[unackedPkt] + increment - localtime;
	starttimer(0, unackedTimeLeft);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	winsize = getwinsize();
	winbase = 0;
	nextseq = 0;
	send_counter = 0;
	unackedPkt = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	if(!isValidChecksum(packet))
		printf("%s\n", "Packet checksum does not match!");
	else {
		struct pkt ackpkt;
		memset(&ackpkt.payload, '\0', 20);
		ackpkt.seqnum = 0;
		ackpkt.acknum = packet.seqnum;
		setChecksum(&ackpkt);
		tolayer3(1, ackpkt);
		recv_packets[packet.seqnum] = packet;
		inorder[packet.seqnum] = 1;
		while(inorder[recv_counter] == 1) {
			memset(&msg, '\0', 20);
			strncpy(msg, recv_packets[recv_counter].payload, 20);
			tolayer5(1, msg);
			recv_counter++;
		}
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	memset(&msg, '\0', 20);
	recv_counter = 0;
}
