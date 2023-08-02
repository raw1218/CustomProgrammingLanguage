#ifndef ASSEMBLY_H
#define ASSEMBLY_H


#include "DataStructures/Map.h"
#include "DataStructures/Stack.h"
#include "AST.h"
#include <stdint.h>

#define NUM_REGISTERS 7


typedef struct {
    char * name;
    bool in_use;
    AST_Node * node; // Node whos value is currently in register
    int wordIndex; //0 for highest word


    int value; //The 32 bits in the register as a signed int
} Register;

extern Register registerX0;

typedef enum {
    LOCATION_REGISTER,
    LOCATION_MEMORY,
} LocationType;

typedef struct {
    LocationType type;
    union {
        char * register_name;
        int memory_offset; //(bytes) offset from appropriate stack frame
    };

    int value; //The 32 bits either in memory or register

} ValueLocation;

typedef struct {
    ValueLocation * locations;
    int word_count;
} MultiWordValueLocation;


typedef struct {
    Register registers[NUM_REGISTERS];
    Map * nodeLocationMap; //A map from AST Node id, to location
    int stack_offset; //(bytes) Used to keep track of stack offset (within frame) when spilling registers
    int next_label;
    int stackPointer;
    int framePointer;
    Map * variableMap; //Variable Table,  keys are variable names

    Map * stringTable;
    Map * functionTable;
    Stack * variableTableStack;

    char ** labels;
    int numLabels;
    int strCount;
    int MaxAddress;


} CodeGenerator;





typedef struct {
    DataType * argTypes;
    char ** argNames;
    int numArgs;
    int totalSize;
} FunctionArguments;





typedef struct {
    int32_t whole;
    uint32_t fractional;
} FixedPoint;


void initialize(CodeGenerator * gen, int sp);
void  generateCode(CodeGenerator * gen, AST_Node * node);
CodeGenerator * createCodeGenerator();
void printVariableMap(CodeGenerator * gen);








#endif //ASSEMBLY_H

