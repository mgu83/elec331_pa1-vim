/**
 * @file queue.h
 * @author Maggie Gu (@mgu83), 
 * @author Vi Kankanamge (@vidunikankan)
 * @brief Function prototypes of queue data structure
 * @bug No none bugs
 * 
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include "types.h"

/**
 * @brief Data structure for Queue component
 * 
 */
typedef struct QueueNode {
    packet data;
    struct QueueNode* next;
} QueueNode;

/**
 * @brief Data structure for a queue, since it 
 * is not built-in to C. We use the queue to 
 * manage the packets on sender side. 
 * 
 * Queue is implemented using linked list component, QueueNode 
 * 
 */
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    size_t size;
} Queue;

/**
 * @brief Instantiates queue
 * 
 * @param q 
 */
void initializeQueue(Queue* q);
Queue* constructQueue();
/**
 * @brief Adds item to existing queue
 * 
 * @param q 
 * @param item 
 */
void enqueue(Queue* q, packet item);
/**
 * @brief Removes item to existing queue
 * 
 * @param q 
 * @return packet 
 */
packet dequeue(Queue* q);
/**
 * @brief Gets the first item in queue
 * 
 * @param q 
 * @return packet 
 */
packet front(const Queue* q);
/**
 * @brief Gets the last item in queue
 * 
 * @param q 
 * @return packet 
 */
packet back(const Queue* q);
/**
 * @brief Checks if queue is empty
 * 
 * @param q 
 * @return int 
 */
int isEmpty(const Queue* q);
/**
 * @brief Returns number of items in queue
 * 
 * @param q 
 * @return size_t 
 */
size_t size(const Queue* q);

#endif // QUEUE_H
