#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct StackNode {
    void* data;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* top;
    int numItems;
} Stack;

Stack* createStack();
bool isStackEmpty(Stack* stack);
void push(Stack* stack, void* data);
void* pop(Stack* stack);
void * peek(Stack * stack);
void * peekAtIndex(Stack * stack, int index);

#endif // STACK_H
