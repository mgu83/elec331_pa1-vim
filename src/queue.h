#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include "param.h"

// TO-DO add function headers
typedef struct QueueNode {
    packet data;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    size_t size;
} Queue;

// Function prototypes
void initializeQueue(Queue* q);
void enqueue(Queue* q, packet item);
packet dequeue(Queue* q);
packet front(const Queue* q);
packet back(const Queue* q);
int isEmpty(const Queue* q);
size_t size(const Queue* q);

#endif // QUEUE_H
