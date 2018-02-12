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

/* Global Variables */

float increment = 4.0;

// Variables for A

int seq_A;
struct pkt* currentpkt_A;
int inTransit;
char buffer[1000][21];
int hasBuffer;
int buffCount;

// Variables for B

int ack_B;
struct pkt* currentpkt_B;
int reached;
char msg[20];


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
	//printf("Message in A: %s\n", message.data);
	if(inTransit == 0) {
		if(hasBuffer > 0) {

                        printf("%s\n", "Sending from buffer...");
			memset(&currentpkt_A->payload, '\0', 20);
		
			memcpy(currentpkt_A->payload, (char*)&buffer[buffCount], 20);
			currentpkt_A->seqnum = seq_A;
			setChecksum(currentpkt_A);
			printf("Checksum from A: %d\n", currentpkt_A->checksum);
			printf("Payload from A: %s\n", currentpkt_A->payload);
			tolayer3(0, *currentpkt_A);
			inTransit = 1;

			starttimer(0, increment);

			printf("%s\n", "Adding to buffer when sending from buffer...");
			memcpy((char*)&buffer[hasBuffer], (char*)&message.data, 20);
			hasBuffer++;
			buffCount++; 
		}
		else {
			printf("Sending directly...\n");
			memset(&currentpkt_A->payload, '\0', 20);
			
			strncpy(currentpkt_A->payload, message.data, 20);
			currentpkt_A->seqnum = seq_A;
			setChecksum(currentpkt_A);
			printf("Checksum from A: %d\n", currentpkt_A->checksum);
			printf("Payload from A: %s\n", currentpkt_A->payload);
			tolayer3(0, *currentpkt_A);

			starttimer(0, increment);

			inTransit = 1;
		}
	}
	else {
		printf("%s\n", "Adding to buffer when in transit...");
		memcpy((char*)&buffer[hasBuffer], (char*)&message.data, 20);
		hasBuffer++;
	}
	//printf("Buffer now has: \n");
	//for(int i = 0; i<hasBuffer; i++)
	//	printf("%s\n", buffer[i]);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(!isValidChecksum(packet))
		printf("%s\n", "ACK checksum does not match!");
	else if(packet.acknum == seq_A && inTransit == 1) {
		printf("ACK received for sequence number: %d\n", seq_A);
		seq_A = (seq_A == 0) ? 1 : 0;
		inTransit = 0;
		stoptimer(0);
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{

	tolayer3(0, *currentpkt_A);
	starttimer(0, increment);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	seq_A = 0;
	inTransit = 0;
	currentpkt_A = new pkt;
	memset(&currentpkt_A->payload, '\0', 20);
	currentpkt_A->seqnum = 0;
	currentpkt_A->checksum = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	printf("Checksum in B: %d\n", packet.checksum);

	if(!isValidChecksum(packet)) {
		printf("%s\n", "Packet checksum does not match!");
	}
	else if(packet.seqnum == ack_B) {
		printf("Packet received for ack number: %d\n", ack_B);
		strncpy(msg, packet.payload, 20);
		printf("Message going to layer5: %s\n", msg);
		tolayer5(1, msg);

		memset(&currentpkt_B->payload, '\0', 20);
		currentpkt_B->seqnum = 0;
		currentpkt_B->acknum = 0;
		currentpkt_B->checksum = 0;

		currentpkt_B->acknum = ack_B;
		setChecksum(currentpkt_B);
		tolayer3(1, *currentpkt_B);
		ack_B = (ack_B == 0) ? 1: 0;
	}
	else {
		tolayer3(1, *currentpkt_B);
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	ack_B = 0;
	currentpkt_B = new pkt;
	currentpkt_B->acknum = 0;
}
