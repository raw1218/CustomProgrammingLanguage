#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Set.h"


// Function to create a set
Set* createSet(bool (*compare)(void*, void*)) {
    Set *set = (Set*) malloc(sizeof(Set));
    set->head = NULL;
    set->compare = compare;
    set->size = 0;
    return set;
}

// Function to check if an item is in the set
bool setContains(Set *set, void *data) {


    if(set == NULL){
 
        return false;
    }
    SetNode *node = set->head;
    while (node != NULL) {

        if (set->compare(node->data, data)) {

          
            return true;
        }
      
        node = node->next;
    }
  
    return false;
}

// Function to add an item to the set
void addToSet(Set *set, void *data) {
    if(setContains(set, data)) return;

    SetNode *newSetNode = (SetNode*) malloc(sizeof(SetNode));
    newSetNode->data = data;
    newSetNode->next = set->head;
    set->head = newSetNode;
    set->size++;
}



bool compareSets(Set* set1, Set* set2) {
    if (set1->size != set2->size) {
        return false; // Sets have different sizes, they can't be equal
    }

    SetNode* node1 = set1->head;
    while (node1 != NULL) {
        if (!setContains(set2, node1->data)) {
            return false; // Found an element in set1 that is not present in set2
        }
        node1 = node1->next;
    }

    SetNode* node2 = set2->head;
    while (node2 != NULL) {
        if (!setContains(set1, node2->data)) {
            return false; // Found an element in set2 that is not present in set1
        }
        node2 = node2->next;
    }

    return true; // Sets are equal
}


// Function to destroy a set
void destroySet(Set *set) {
    SetNode *node = set->head;
    while (node != NULL) {
        SetNode *nextSetNode = node->next;
        free(node->data);
        free(node);
        node = nextSetNode;
    }
    free(set);
}

Set* unionSet(Set * a, Set * b){

    Set * unionSet = createSet(a->compare);

    SetNode* node = a->head;
    while (node != NULL) {
        addToSet(unionSet, node->data);
        node = node->next;
    }


    node = b->head;
    while (node != NULL){
        addToSet(unionSet, node->data);
        node = node->next;
    }


    return unionSet;
}




