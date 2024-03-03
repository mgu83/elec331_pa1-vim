/**
 * @file priorityqueue.c
 * @author Maggie Gu (@mgu83),
 * @author Vi Kankanamge (@vidunikankan)
 * @brief Implementation of priority queue data structure
 * @bug No none bugs
 *
 */
#include <stdlib.h>
#define INITIAL_SIZE 10
#include "types.h"
#include "priorityqueue.h"

void pq_init(PriorityQueue *pq)
{
    pq->array = (packet *)malloc(INITIAL_SIZE * sizeof(packet));
    if (!pq->array)
    {
        perror("Failed to allocate memory for priority queue");
        exit(EXIT_FAILURE);
    }
    pq->size = 0;
    pq->capacity = MAX_QUEUE_SIZE;
}
// TODO: Add destructor / free & call at the end of connection

PriorityQueue *constructPQ()
{
    PriorityQueue *newq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    if (newq == NULL)
    {
        perror("Failed to construct pq");
        exit(EXIT_FAILURE);
    }
    pq_init(newq);
    return newq;
}

void pq_push(PriorityQueue *pq, packet item)
{
    // Resize if capacity is reached
    if (pq->size == pq->capacity)
    {
        pq->capacity *= 2;
        pq->array = (packet *)realloc(pq->array, pq->capacity * sizeof(packet));
        if (!pq->array)
        {
            perror("Failed to resize priority queue");
            exit(EXIT_FAILURE);
        }
    }
    // Place the item at the end and heapify-up
    size_t i = pq->size++;
    while (i && item.seq_num < pq->array[(i - 1) / 2].seq_num)
    {
        pq->array[i] = pq->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    pq->array[i] = item;
}

packet pq_pop(PriorityQueue *pq)
{
    if (pq->size == 0)
    {
        fprintf(stderr, "Priority queue underflow\n");
        exit(EXIT_FAILURE);
    }

    packet result = pq->array[0];
    packet last_item = pq->array[--pq->size];

    // Heapify-down
    size_t i = 0;
    while ((i * 2 + 1) < pq->size)
    {
        size_t left = i * 2 + 1;
        size_t right = i * 2 + 2;
        size_t j = left;
        if (right < pq->size && pq->array[right].seq_num < pq->array[left].seq_num)
        {
            j = right;
        }
        if (last_item.seq_num <= pq->array[j].seq_num)
        {
            break;
        }
        pq->array[i] = pq->array[j];
        i = j;
    }
    pq->array[i] = last_item;

    return result;
}

int pq_empty(const PriorityQueue *pq)
{
    return pq->size == 0;
}

packet pq_top(const PriorityQueue *pq)
{
    if (pq_empty(pq))
    {
        fprintf(stderr, "Attempt to access top element of an empty priority queue\n");
        exit(EXIT_FAILURE);
    }
    return pq->array[0];
}

int pq_size(const PriorityQueue *pq)
{
    return pq->size;
}
