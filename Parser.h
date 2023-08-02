#ifndef PARSER_H
#define PARSER_H


#include "CFG.h"
#include "DataStructures/Stack.h"
#include "DataStructures/Set.h"
#include <stdio.h>
#include <stdlib.h>




typedef struct {
    Rule rule;
    int dot_position;
    Symbol lookAhead;
} GrammarItem;


typedef struct {
    Set* items;
    int id;
} State;


typedef enum {
    ACTION_SHIFT,
    ACTION_REDUCE,
    ACTION_ACCEPT,
    ACTION_ERROR
} ActionType;

typedef struct {
    ActionType type;
    int value;
    Rule rule;
} Action;

typedef struct {
    Action** actionTable;    // 2D array: states x terminals
    int** gotoTable;         // 2D array: states x nonTerminals
    int numStates;
    int numTerminals;
    int numNonTerminals;
} LRTable;




typedef struct {
    Stack * stack;
    LRTable * table;

    int currentState;

    int currentNodeId; //used to give each AST Node a unique ID

    char ** functionKeywords;
    int numFunctionKeywords;

    Rule * originalRules;
    int numOriginalRules;
    Rule * newRules;
    int numNewRules;



    int helperCount; //used to make helper symbols
    Rule * finalRules;



    int totalFunctionDefinitionCount; //A negative number (magnitude represents num of total encounterd function Definitions)
    int currentFunctionDefinitionCount; //A negative number (magnitude represents num of this loops encounterd function Definitions)

} LRParser;


extern FILE * stdoutFP;

#endif //PARSER_H