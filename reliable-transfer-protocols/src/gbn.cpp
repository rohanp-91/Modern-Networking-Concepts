#define TRUE 1
#define FALSE 0

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

/* Global variables */
float increment = 40.0;
int winsize;

// Variables for A

int winbase;
int nextseq;
int counter;
struct pkt packets[1000];

// Variables for B

int ack_B;
char msg[20];

void printAll() 
{
	for(int i = 0; i < counter; i++)
		printf("Payload in sender buffer: %s\n", packets[i].payload);
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
	packet.seqnum = counter;
	memset(&packet.payload, '\0', 20);
	memcpy((char*)&packet.payload, (char*)&message.data, 20);
	memset(&packets[counter].payload, '\0', 20);
	packets[counter] = packet;
	counter++;
	//printAll();

	if(nextseq < winbase + winsize) {
		//packets[nextseq].seqnum = nextseq;
		setChecksum(&packets[nextseq]);
		tolayer3(0, packets[nextseq]);

		if(winbase == nextseq) 
			starttimer(0, increment);
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
		winbase = packet.acknum;
		stoptimer(0);
		starttimer(0, increment);
		
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	starttimer(0, increment);
	for(int i = winbase; i < nextseq; i++) {
		//printf("Sending to layer3 from A_timerinterrupt: %s\n", packets[i]->payload);
		//packets[i].seqnum = i;
		setChecksum(&packets[i]);
		tolayer3(0, packets[i]);
	}
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	winsize = getwinsize();
	winbase = 0;
	nextseq = 0;
	counter = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	if(!isValidChecksum(packet))
		printf("%s\n", "Packet checksum does not match!");
	else if(packet.seqnum == ack_B) {
		printf("Packet received for ack number: %d\n", ack_B);
		memset(&msg, '\0', 20);
		strncpy(msg, packet.payload, 20);
		tolayer5(1, msg);

		struct pkt ackpkt;
		memset(&ackpkt.payload, '\0', 20);
		ackpkt.seqnum = 0;
		ackpkt.acknum = ack_B;
		setChecksum(&ackpkt);
		tolayer3(1, ackpkt);

		ack_B++;
	}
	else {
		struct pkt ackpkt;
		memset(&ackpkt.payload, '\0', 20);
		ackpkt.seqnum = 0;
		ackpkt.acknum = ack_B;
		setChecksum(&ackpkt);
		tolayer3(1, ackpkt);
	}
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	ack_B = 0;
	memset(&msg, '\0', 20);
}
