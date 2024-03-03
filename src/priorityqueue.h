/**
 * @file priorityqueue.h
 * @author Maggie Gu (@mgu83), 
 * @author Vi Kankanamge (@vidunikankan)
 * @brief Function prototypes of priority queue data structure
 * @bug No none bugs
 * 
 */
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdlib.h>

#include "types.h"

/**
 * @brief Structure to help managed packets coming to 
 * receiver. Used priority queue so we could prioritize 
 * packets from ascending order
 * 
 */
typedef struct {
    packet* array;    // Dynamic array to store the heap elements
    size_t size;      // Current number of elements in the heap
    size_t capacity;  // Current capacity of the array
} PriorityQueue;

/**
 * @brief Initializes PriorityQueue
 * 
 * @param pq 
 */
void pq_init(PriorityQueue* pq);

/**
 * @brief Initializes pq constructor, calls init
 * 
 */
PriorityQueue* constructPQ();

/**
 * @brief Adds packet to existing PriorityQueue
 * 
 * @param pq 
 * @param item 
 */
void pq_push(PriorityQueue* pq, packet item);
/**
 * @brief Fetches item from front of PriorityQueue
 * 
 * @param pq 
 * @return packet 
 */
packet pq_pop(PriorityQueue* pq);
/**
 * @brief Checks if PriorityQueue is empty
 * 
 * @param pq 
 * @return int 
 */
int pq_empty(const PriorityQueue *pq);
/**
 * @brief Fetches packet at top of PriorityQueue
 * 
 * @param pq 
 * @return packet 
 */
packet pq_top(const PriorityQueue *pq);
/**
 * @brief Returns the number of items in PriorityQueue
 * 
 * @param pq 
 * @return int 
 */
int pq_size(const PriorityQueue *pq);

#endif // PRIORITY_QUEUE_H
