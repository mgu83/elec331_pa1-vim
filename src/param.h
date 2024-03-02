/**
 * @file param.h
 * @author Maggie Gu (@mgu83)
 * @brief Data structure for TCP
 */
#ifndef PARAM_H
#define PARAM_H

#include <stdlib.h>
#include <stdio.h>

#define MSS             512 // Maximum Segment Size
#define AMPLIFIER       256 // Amplify the ssthreash by this factor
#define INIT_SST_AMP    512 // Initial ssthreash is this factor times the MSS
#define MAX_QUEUE_SIZE  10  // Maximum size of queue
#define TIMEOUT         10000 // Timeout variable

/**
 * @brief Types of TCP packets - meant to substitute for flags
 * DATA = Data packets
 * ACK = Acknowledgment packets
 * FIN = Last packet from sender
 * FINACK = Acknowledgment of Finish Packages
 */
typedef enum packet_type{
    DATA,
    ACK,
    FIN,
    FINACK
} packet_type;

/**
 * @brief Current status of the TCP connection in terms of congestion control
 * Details listed in course textbook 
 * 
 */
typedef enum status_type {
    SLOW_START = 5,
    CONGESTION_AVOIDANCE,
    FAST_RETRANSMIT,
    FAST_RECOVERY
} status_type;

/**
 * @brief Represents a TCP packet
 * 
 */
typedef struct  {
    packet_type pkt_type;
    uint64_t 	seq_num;
    uint64_t    ack_num;
	int 	    data_size;
	char        data[MSS];
} packet;

#endif



