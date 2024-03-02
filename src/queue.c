#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "queue.h"

// Function prototypes
void initializeQueue(Queue* q);
void enqueue(Queue* q, packet item);
packet dequeue(Queue* q);
packet front(const Queue* q);
packet back(const Queue* q);
int isEmpty(const Queue* q);
size_t size(const Queue* q);

void initializeQueue(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

Queue* constructQueue(){
    Queue* newq = (Queue*)malloc(sizeof(Queue));
    initializeQueue(newq);
    return newq;
}

int isEmpty(const Queue* q) {
    return q->front == NULL;
}

size_t size(const Queue* q) {
    return q->size;
}

void enqueue(Queue* q, packet item) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newNode == NULL) {
        printf("Failed to allocate memory for new node\n");
        return;
    }
    newNode->data = item;
    newNode->next = NULL;
    
    if (isEmpty(q)) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

packet dequeue(Queue* q) {
    packet item;
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return item; // Should ideally handle this case better
    }
    QueueNode* temp = q->front;
    item = temp->data;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return item;
}

packet front(const Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        packet empty; 
        return empty;
    }
    return q->front->data;
}

packet back(const Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        packet empty; 
        return empty;
    }
    return q->rear->data;
}
