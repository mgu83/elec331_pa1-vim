/**
 * @file types.h
 * @author Maggie Gu (@mgu83) Vi Kankanamge (@vidunikankan)
 * @brief Data structure for TCP
 */
#ifndef PARAM_H
#define PARAM_H

#include <stdlib.h>
#include <stdio.h>

#define MSS             512 // Maximum Segment Size
#define MAX_QUEUE_SIZE  10  // Maximum size of queue
#define TIMEOUT         10000 // Timeout variable
#define BUF_SIZE_MAX    16348

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
    FINACK,
    TDACK
} packet_type;

/**
 * @brief Current status of the TCP connection in terms of congestion control
 * Details listed in course textbook 
 * 
 */
typedef enum state_type {
    SLOW_START,
    CONG_AVOID,
    SLOW_RETRANS
} state_type;

/**
 * @brief Represents a TCP packet
 * 
 */
typedef struct  {
    packet_type pkt_type;
    uint64_t 	seq_num;
    uint64_t    ack_num;
	int 	    data_size;
	char        data[BUF_SIZE_MAX];
} packet;

#endif



