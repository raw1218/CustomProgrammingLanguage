#include "Queue.h"


Queue* createQueue() {
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queue->front = queue->back = NULL;
    return queue;
}

bool isQueueEmpty(Queue* queue) {
    return queue->front == NULL;
}

void enqueue(Queue* queue, void* data) {
    QueueNode* node = (QueueNode*) malloc(sizeof(QueueNode));
    node->data = data;
    node->next = NULL;

    if (isQueueEmpty(queue)) {
        queue->front = queue->back = node;
    } else {
        queue->back->next = node;
        queue->back = node;
    }
}

void* dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
        return NULL;
    }

    QueueNode* temp = queue->front;
    void* data = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->back = NULL;
    }

    free(temp);
    return data;
}