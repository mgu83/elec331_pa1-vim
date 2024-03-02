#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdlib.h>

#include "param.h"

typedef struct {
    packet* array;    // Dynamic array to store the heap elements
    size_t size;      // Current number of elements in the heap
    size_t capacity;  // Current capacity of the array
} PriorityQueue;

// TO-DO add function headers
void pq_init(PriorityQueue* pq);
void pq_push(PriorityQueue* pq, packet item);
packet pq_pop(PriorityQueue* pq);
void pq_free(PriorityQueue* pq);
int pq_empty(const PriorityQueue *pq);
packet pq_top(const PriorityQueue *pq);
int pq_size(const PriorityQueue *pq);

#endif // PRIORITY_QUEUE_H
