#include <stdlib.h>
#include "stack.h"

// Function to create a stack
Stack* createStack() {
    Stack* stack = (Stack*) malloc(sizeof(Stack));
    stack->top = NULL;
    stack->numItems = 0;
    return stack;
}

// Function to check if the stack is empty
bool isStackEmpty(Stack* stack) {
    return (stack->top == NULL);
}

// Function to add an element to the stack
void push(Stack* stack, void* data) {
    StackNode* newNode = (StackNode*) malloc(sizeof(StackNode));
    if (newNode == NULL) {
        exit(1); // failed to allocate memory
    }
    newNode->data = data;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->numItems++;
}

// Function to remove the top element from the stack
void* pop(Stack* stack) {
    if (isStackEmpty(stack)) {
        return NULL; // stack is empty
    }
    StackNode* topNode = stack->top;
    stack->top = topNode->next;
    void* data = topNode->data;
    free(topNode);
    stack->numItems--;
    return data;
}

// Function to peek at top element without removing it
void * peek(Stack * stack){

    if(isStackEmpty(stack)) return NULL;

    StackNode * topNode = stack->top;
    return topNode->data;
}


void * peekAtIndex(Stack * stack, int index){
    StackNode *  cur = stack->top;
   
    if(index >= stack->numItems) return NULL;



    else for(int i = 0; i < index; i++){
        
        cur = cur->next;
    }

    return cur->data;
}





