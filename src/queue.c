#include <stdio.h>
#include <stdlib.h>
#include "param.h"
#include "queue.h"


// Function prototypes
void initializeQueue(Queue* q);
void enqueue(Queue* q, packet item);
packet dequeue(Queue* q);
packet front(const Queue* q);
packet back(const Queue* q);
int isEmpty(const Queue* q);
size_t size(const Queue* q);

// Initialize the queue
void initializeQueue(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

// Check if the queue is empty
int isEmpty(const Queue* q) {
    return q->front == NULL;
}

// Get the size of the queue
size_t size(const Queue* q) {
    return q->size;
}

// Add an item to the queue
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

// Remove an item from the queue
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

// Get the front item of the queue
packet front(const Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        packet empty; // Should ideally handle this case better
        return empty;
    }
    return q->front->data;
}

// Get the back item of the queue
packet back(const Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        packet empty; // Should ideally handle this case better
        return empty;
    }
    return q->rear->data;
}
