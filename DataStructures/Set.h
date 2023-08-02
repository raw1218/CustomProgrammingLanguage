#ifndef SET_H
#define SET_H

#include <stdbool.h>

// SetNode structure for the set
typedef struct SetNode {
    void *data;
    struct SetNode *next;
} SetNode;

// Set structure
typedef struct {
    SetNode *head;
    bool (*compare)(void*, void*);
    int size;
} Set;

// Function to create a set
Set* createSet(bool (*compare)(void*, void*));

// Function to add an item to the set
void addToSet(Set *set, void *data);

// Function to check if an item is in the set
bool setContains(Set *set, void *data);

//Function to compare two sets
bool compareSets(Set* set1, Set* set2);

// Function to destroy a set
void destroySet(Set *set);


Set* unionSet(Set * a, Set * b);


#endif