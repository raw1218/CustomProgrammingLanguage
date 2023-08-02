#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct QueueNode {
    void* data;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode* front;
    QueueNode* back;
} Queue;

Queue* createQueue();
bool isQueueEmpty(Queue* queue);
void enqueue(Queue* queue, void* data);
void* dequeue(Queue* queue);

#endif  // QUEUE_H
