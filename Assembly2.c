#include "Assembly.h"
#include "CFG.h"

#include "AST.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>






//helpers ///////////////////////////////////
void _printVariableMapKey(void * a){
    char * string = (char *)a;
    printf("%s ", string);
}

void _printVariableMapItem(void * b){
    VariableTableEntry entry = *(VariableTableEntry*)b;
    printf("Variable is type: ");

    if(entry.type == TYPE_INTEGER) printf("integer. ");
    if(entry.type == TYPE_DECIMAL) printf("decimal. ");

    printf("Memory Location = %d", entry.memoryLocation);
}

void printVariableMap(CodeGenerator * gen){

    Map * variableMap = gen->variableMap;

    printMap(variableMap, _printVariableMapKey, _printVariableMapItem);
    
    
}

FixedPoint float_to_fixed_point(float val){
    // Scale by 2^32
    double scaled_val = (double)val * ((int64_t)1 << 32);

    // Now we can just cast this to a int64_t
    int64_t result = (int64_t)scaled_val;

    // Extract the whole and fractional parts
    int32_t whole = result >> 32;
    int32_t fractional = (int32_t)result;

    FixedPoint fixedPoint;
    fixedPoint.whole = whole;
    fixedPoint.fractional = fractional;

   //printf("the result of turning %f into 64 bits is = %ld, whole part: %d, fractional part: %u\n", val, result, whole, fractional);

    return fixedPoint;
}

float fixed_to_float(FixedPoint val) {
    float whole = (float)val.whole;
    float fractional = ((float)val.fractional) / ((uint64_t)1 << 32);

    return whole + fractional;
}

FixedPoint double_to_fixed_point(double val){
    FixedPoint result;

    int32_t whole = (int32_t) val;

    double fractional = val - whole;

    uint32_t resultFractional = (uint32_t)(fractional * ((uint64_t)1 << 32));

    result.whole = whole;
    result.fractional = resultFractional;

    return result;
}

double fixed_to_double(FixedPoint val) {
    double whole = (double)val.whole;
    double fractional = ((double)val.fractional) / ((uint64_t)1 << 32);

    return whole + fractional;
}


void incrementStackPointer(CodeGenerator * gen, int amt){
    gen->stackPointer += amt;
    printf("  ADDI sp, sp, %d\n", amt);
    gen->stack_offset -= amt;
}

void setFramePointer(CodeGenerator * gen, int address){
    load_int(gen, address, registerS0);
    gen->framePointer = address;
}




int getSize(DataType type){
    if(type == TYPE_INTEGER)return 4;
    if(type == TYPE_DECIMAL)return 8;
}

bool compareInts(void * a, void * b){

    int first = *(int *)a;
    int sec = *(int *)b;

    return first == sec;
}

CodeGenerator * createCodeGenerator(){
    CodeGenerator * gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    Map * locationMap = createMap(compareInts);
   

    Stack * variableTableStack = createStack();


    gen->nodeLocationMap = locationMap;
    gen->stack_offset = 0;
    gen->next_label = 0;


    Register reg1 = {.in_use = false, .node = NULL, .name = "t0"};
    Register reg2 = {.in_use = false, .node = NULL, .name = "t1"};
    Register reg3 = {.in_use = false, .node = NULL, .name = "t2"};
    Register reg4 = {.in_use = false, .node = NULL, .name = "t3"};
    Register reg5 = {.in_use = false, .node = NULL, .name = "t4"};
    Register reg6 = {.in_use = false, .node = NULL, .name = "t5"};
    Register reg7 = {.in_use = false, .node = NULL, .name = "t6"};



    gen->registers[0] = reg1;
    gen->registers[1] = reg2;
    gen->registers[2] = reg3;
    gen->registers[3] = reg4;
    gen->registers[4] = reg5;
    gen->registers[5] = reg6;
    gen->registers[6] = reg7;

    



    return gen;

}

void addVariableToCurrentScope(CodeGenerator * gen, char * key, VariableTableEntry * entry){
    
    VariableTable * curScope = (VariableTable *)peek(gen->variableTableStack);
    
    insert(curScope->variables, key, entry);
    
    int size;
    DataType type = entry->type;
    if(type == TYPE_INTEGER)size = 4;
    if(type == TYPE_DECIMAL)size = 8;
    curScope->frameSize += size;

}

char *  addLabel(CodeGenerator *generator, char *label) {
    // Make a copy of the label so we can modify it if necessary
    char *newLabel = strdup(label);
    int labelLen = strlen(newLabel);
    
    // Check if the label already exists
    bool labelExists;
    do {
        labelExists = false;
        for(int i = 0; i < generator->numLabels; i++) {
            if(strcmp(generator->labels[i], newLabel) == 0) {
                labelExists = true;
                break;
            }
        }

        // If the label exists, append "_2" and check again
        if(labelExists) {
            char *oldLabel = newLabel;
            asprintf(&newLabel, "%s_2", oldLabel); 
            free(oldLabel); // Don't forget to free the old label
        }

    } while(labelExists);

    // Now we know newLabel is unique, so add it to the labels array
    generator->labels = (char**)realloc(generator->labels, (generator->numLabels + 1) * sizeof(char*));
    generator->labels[generator->numLabels] = newLabel; // Copy the new label string
    generator->numLabels++;
    return newLabel;
}

void enterScope(CodeGenerator * gen){
   //push new variable table onto the stack
    Map * newScope = createMap(compareStrings);
    VariableTable * table = (VariableTable *)malloc(sizeof(VariableTable));
    table->variables = newScope;
    table->frameSize = 0;
    push(gen->variableTableStack, table);


    // set the FP to the SP

    printf("  ADD fp, sp, x0\n");

}
void leaveScope(CodeGenerator * gen){
    VariableTable * table = pop(gen->variableTableStack);
    
    
    printf("  ADD sp, fp, x0\n");

    VariableTable * newTable = peek(gen->variableTableStack);
    int frameSize = newTable->frameSize;

    printf("  ADDI fp, fp, %d\n", frameSize);
    
}
void leaveScopeReturn(CodeGenerator * gen){
    printf("  ADD sp, fp, x0\n");
    VariableTable * table = peekAtIndex(gen->variableTableStack, 1);
    int frameSize = table->frameSize;
    printf("  ADDI fp, fp, %d\n", frameSize);
}


void addNewVariable(CodeGenerator * gen, Symbol identifierSym, DataType type){
    VariableTable * currentScope = peek(gen->variableTableStack);
    int size = getSize(type);

    currentScope->frameSize += size;
    incrementStackPointer(gen, -size);

    char * word = (char *)identifierSym.value;

    VariableTableEntry * entry = createVariableTableEntry(type, -(currentScope->frameSize), NULL);
    insert(currentScope->variables, word, entry);

    
}


void initialize(CodeGenerator * gen){
    gen->stackPointer = 0;
    incrementStackPointer(gen, 500);
    setFramePointer(gen, 500);
    gen->stack_offset = 0;
    enterScope(gen);
    printf("  JAL x0, MAIN\n");
    generate2WordAdditionFunction(gen);
    generateUnsigned1WordMultiplicationFunction(gen);
    generateUnsignedFixedPointMultiplicationFunction(gen);
    generateSigned1WordMultiplicationFunction(gen);
    generateSignedFixedPointMultiplicationFunction(gen);
    generate2WordSubtractionFunction(gen);
    generateUnsigned2WordDivisionFunction(gen);

}

FunctionArguments getFunctionArguments(AST_Node * node){
    
    Symbol sym = node->symbol;
    FunctionArguments args;
    args.numArgs = 0;
    args.totalSize = 0;



    //function declaration
    if(compareSymbols(&sym, &FunctionDefinitionSymbol)){

        AST_Node * signature = node->children[1];
        int numChildren = signature->num_children;

        args.argNames = (char**)malloc(sizeof(char*) * numChildren);
        args.argTypes = (DataType*)malloc(sizeof(DataType) * numChildren);



        for(int i = 0; i < numChildren; i++){
            AST_Node * childNode = signature->children[i];
            Symbol childSym = childNode->symbol;
            if(compareSymbols(&childSym, &FunctionArgumentSymbol)){

                AST_Node * typeNode = childNode->children[0];
                AST_Node * nameNode = childNode->children[1];
                char * name = nameNode->symbol.value;

                DataType type = dataTypeFromTypeSymbol(typeNode->symbol);
                int size = getSize(type);

                
                args.totalSize += size;

                args.argTypes[args.numArgs] = type;
                args.argNames[args.numArgs] = name;
                args.numArgs++;
            }
        }
    }

    //function call
}


VariableTableEntry * getVariableEntryGen(CodeGenerator * gen, Symbol identifierSym){
    

    int numScopes = gen->variableTableStack->numItems;
    char * word = (char *)identifierSym.value;

    for(int i = 0; i < numScopes; i++){
        Map * scope = peekAtIndex(gen, i);
        if(doesVariableExistInScope(scope, identifierSym)){
            return get(scope, word);
        }
    }

    return NULL;
}



//end helpers////////////////////////////////





Register registerX0 = (Register){.in_use = false, .name = "x0", .node = NULL, .wordIndex = 0, .value = 0};
Register registerS0 = {.in_use = true, .name = "s0", .node = NULL, .wordIndex = 0, .value = 0};

void load_int(CodeGenerator * gen, int value, Register * reg){

    char * reg_name  = reg->name;
    reg->value = value;

    if (value >= -2048 && value <= 2047) {
        printf("  addi %s, x0, %d\n", reg_name, value);
    } else {
        int upper_imm = value >> 12;
        int lower_imm = value & 0xfff;
        printf("  lui %s, %d\n", reg_name, upper_imm);
        printf("  addi %s, %s, %d\n", reg_name, reg_name, lower_imm);
    }
}



MultiWordValueLocation* get_value_location(CodeGenerator * gen, AST_Node * node){


    if(node == NULL)return NULL;
    int id = node->id;
    MultiWordValueLocation * location = get(gen->nodeLocationMap, &id);

    return location;

}

void set_value_location(CodeGenerator * gen, AST_Node * node, MultiWordValueLocation * location){
    
    //printf("in set_value_locatoin\n");
   

    if(node == NULL)return;
    int id = node->id;

   

    
   // MultiWordValueLocation * locationPtr = (MultiWordValueLocation *)malloc(sizeof(MultiWordValueLocation));
    int * idPtr = (int*)malloc(sizeof(int));

    *idPtr = id;
   // *locationPtr = location;

  
    insert(gen->nodeLocationMap,idPtr,location);
   // printf("made it out of set_value_location\n");
    
}

void update_value_location(CodeGenerator * gen, AST_Node * node, ValueLocation location, int wordIndex){

  // printf("in update_value location, node id = %d, and word index = %d\n", node->id, wordIndex);
   
    MultiWordValueLocation* locations = get_value_location(gen, node);


    if(locations == NULL){


        
        MultiWordValueLocation * newLocation = (MultiWordValueLocation *)malloc(sizeof(MultiWordValueLocation));
       
        if(wordIndex != 0){
           // printf("in update value location,  location has not been set yet but the index is > 0 \n");
       
        }

       ValueLocation * newLocations = (ValueLocation*)malloc(sizeof(ValueLocation) * (wordIndex + 1));
        newLocations[wordIndex] = location;
        newLocation->word_count = wordIndex + 1;
        newLocation->locations = newLocations;

        set_value_location(gen, node, newLocation);
        return;
    }

    if(locations->word_count <= wordIndex){

       // printf("in update_value_location,  index is too high. wordIndex = %d, wordCount = %d\n", wordIndex, locations->word_count);

        locations->locations = (ValueLocation*)realloc(locations->locations, sizeof(ValueLocation) * (wordIndex + 1));

    }

    locations->locations[wordIndex] = location;
  // printf("made it out of update_value_location\n");

}   

Register * spill_register(CodeGenerator * gen, Register **doNotSpill, int not_spill_count) {
    // Just spill the first register that's not in the doNotSpill list for simplicity
    for (int i = 0; i < NUM_REGISTERS; i++) {
        int spill = 1; // assume that register i should be spilled

        // If register i is in doNotSpill list, mark it as not to spill
        for (int j = 0; j < not_spill_count; j++) {
            if (doNotSpill[j] != NULL && strcmp(gen->registers[i].name, doNotSpill[j]->name) == 0) {
                spill = 0; // register i is in doNotSpill list, so it shouldn't be spilled
            }
        }

        if (spill) {
            Register * reg = &gen->registers[i];

            incrementStackPointer(gen, -4);
            printf("  sw %s, 0(sp)\n", reg->name); // Store register value

            //printf("  addi sp, sp, -4\n"); // Adjust stack pointer

            // Update location of the value formerly in the register
            ValueLocation location = {.type = LOCATION_MEMORY, .memory_offset = gen->stack_offset, .value = reg->value};
            AST_Node * formerNode = reg->node;
            int word_index = reg->wordIndex;


            update_value_location(gen, formerNode, location, word_index);
           // set_value_location(gen, formerNode, location);
            gen->stack_offset += 4;         

            reg->in_use = false;
            reg->node = NULL;

            return reg;
        }
    }
    
    // If no register was found to spill, return NULL
    return NULL;
}


Register * allocate_register(CodeGenerator * gen, AST_Node * node, int wordIndex, Register ** doNotSpill, int doNotSpillCount, int value){

   // printf("in allocate register node id = %d,  word index = %d\n", node->id, wordIndex);


    // Look for a free register in the list
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (!gen->registers[i].in_use) {
            gen->registers[i].in_use = true;
            gen->registers[i].node = node;
            gen->registers[i].wordIndex = wordIndex;
            gen->registers[i].value = value;

            //update Location map
            ValueLocation location = {.type = LOCATION_REGISTER, .register_name = gen->registers[i].name, .value = value};
            //set_value_location(gen, node, location);
            update_value_location(gen, node, location, wordIndex);
            return &gen->registers[i];
        }
    }

    // If no free register was found, spill one
    Register *spilled = spill_register(gen, doNotSpill, doNotSpillCount);

    // Mark the spilled register as in use by the new node
    spilled->in_use = true;
    spilled->node = node;
    spilled->wordIndex = wordIndex;

    // Update location map
    ValueLocation location = {.type = LOCATION_REGISTER, .register_name = spilled->name, .value =value};
    //set_value_location(gen, node, location);

    update_value_location(gen, node, location, wordIndex);

    return spilled;
}

void free_register(CodeGenerator *gen, Register *reg) {
    // Mark the register as free
    reg->in_use = false;
}


Register * getRegisterByName(CodeGenerator * gen, char * name){
    for(int i = 0; i < NUM_REGISTERS; i++){
        Register reg = gen->registers[i];
        if(strcmp(name, reg.name) == 0) return &gen->registers[i];
    }
    return NULL;
}

Register * get_register(CodeGenerator *gen, AST_Node *node, int wordIndex, Register **doNotSpill, int doNotSpillCount) {
    
   //printf("getting register for value id = %d\n", node->id);
    MultiWordValueLocation *multiLocation = get_value_location(gen, node);


    if(multiLocation == NULL) printf("location is null\n");

    ValueLocation location = multiLocation->locations[wordIndex];


    // If the value is already in a register, just return that register
    if(location.type == LOCATION_REGISTER) {
        for(int i = 0; i < NUM_REGISTERS; i++) {
            if(strcmp(gen->registers[i].name, location.register_name) == 0) {
                
                return &gen->registers[i];
            }
        }
    }

    

    // If we reach this point, the value is in memory, and we need to load it into a register.
    // First, we need to find or make space in a register.
    Register *reg = allocate_register(gen, node, wordIndex, doNotSpill, doNotSpillCount, location.value);

    // Load the value into the register
    printf("  lw %s, %d(sp)\n", reg->name, gen->stack_offset - location.memory_offset); // Load word

    // The value is now in the register
    ValueLocation new_location = {.type = LOCATION_REGISTER, .register_name = reg->name};
   // set_value_location(gen, node, new_location);
    update_value_location(gen, node, new_location, wordIndex);

    return reg;
}







//make more efficient with temp regs
void callFunction(CodeGenerator * gen, char * funcName, char ** regNamesIn, int numArgs, char ** regNamesOut, int numOut){
    int numTemp = 7;
    
    
    printf("  ADDI sp, sp, -%d\n", (numArgs +numTemp + 1) * 4);  // allocate stack space


    // Save the argument registers and ra on the stack
    for(int i = 0; i < numArgs; i++){
        printf("  SW a%d, %d(sp)\n", i, i*4);
    }

    for(int i = 0; i < numTemp; i++){
        printf("  SW t%d, %d(sp)\n", i, (numArgs  + i)*4);
    }
    printf("  SW ra, %d(sp)\n", (numArgs +numTemp)*4);

    // Move the specified registers to the argument registers
    for(int i = 0; i < numArgs; i++){
        printf("  ADD a%d, %s, zero\n", i, regNamesIn[i]);
    }

    printf("  JAL ra, %s\n", funcName);  // call the function

    //load temp reg back
    for(int i = 0; i < numTemp; i++){
        printf("  LW t%d, %d(sp)\n", i, (numArgs + i)*4);
    }

    // Move the result from the argument registers to the specified registers
    for(int i = 0; i < numOut; i++){
        printf("  ADD %s, a%d, zero\n", regNamesOut[i], i);
    }

    // Restore the argument registers and ra from the stack
    for(int i = 0; i < numArgs; i++){
        printf("  LW a%d, %d(sp)\n", i, i*4);
    }


    printf("  LW ra, %d(sp)\n", (numArgs + numTemp)*4);

    printf("  ADDI sp, sp, %d\n", (numArgs +numTemp+ 1) * 4);  // deallocate stack space
}

void generate2WordAdditionFunction(CodeGenerator * gen){
    //works for 2's comp and unsigned
    int labelNum = gen->next_label++;
    //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
    //output: a0 = result_upper , a1 = result_lower
    printf("Addition_2Word:\n");
    addLabel(gen, "Addition_2Word");

    //step 1:  Add lower bits
    printf("  ADD t0, a1, a3\n");

    //step 2: Check for Overflow (if res < either of the inputs)
    printf("  BLTU t0, a1, overflow_detected%d\n",labelNum);
    printf("  BLTU t0, a3, overflow_detected%d\n",labelNum);
    printf("  JAL x0, no_overflow_detected%d\n",labelNum);

    //handle overflow by adding 1 to the upper part of an operand
    printf("overflow_detected%d:\n",labelNum);
    char lab1[30];
    sprintf(lab1, "overflow_detected%d", labelNum);
    addLabel(gen, lab1);
    
    printf("  ADDI a0, a0, 1\n");

    //add the upper part, and set the correct registers before returning
    printf("no_overflow_detected%d:\n",labelNum);
    char lab2[30];
    sprintf(lab2, "no_overflow_detected%d", labelNum);
    addLabel(gen, lab2);

    printf("  ADD a0, a0, a2\n");
    printf("  ADD a1, x0, t0\n");

    printf("  JALR x0, ra, 0\n");

}

void generate2WordSubtractionFunction(CodeGenerator * gen){
    //works for 2's comp and unsigned
    int labelNum = gen->next_label++;
    //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
    //output: a0 = result_upper , a1 = result_lower
    printf("Subtraction_2Word:\n");
    addLabel(gen, "Subtraction_2Word");

    //step 1:  Subtract lower bits
    printf("  SUB t0, a1, a3\n");

    //step 2: Check for Underflow (if result > a1)
    printf("  BGEU a1, t0, no_underflow_detected%d\n",labelNum);
    printf("  JAL x0, underflow_detected%d\n",labelNum);

    //handle underflow by subtracting 1 from the upper part of the minuend
    printf("underflow_detected%d:\n",labelNum);
    char lab1[30];
    sprintf(lab1, "underflow_detected%d", labelNum);
    addLabel(gen, lab1);

    printf("  ADDI a0, a0, -1\n");

    //subtract the upper part, and set the correct registers before returning
    printf("no_underflow_detected%d:\n",labelNum);
    char lab2[30];
    sprintf(lab2, "no_underflow_detected%d", labelNum);
    addLabel(gen, lab2);


    printf("  SUB a0, a0, a2\n");
    printf("  ADD a1, x0, t0\n");

    printf("  JALR x0, ra, 0\n");
}

void generateSigned1WordMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left, a1 = right
    //output: a0 = result_upper, a1 = result_lower

    int labelNum = gen->next_label++;
    printf("Multiplication_Signed_1Word:\n");
    addLabel(gen, "Multiplication_Signed_1Word");

    //make copies of inputs
    printf("  ADD t0, a0, x0\n");
    printf("  ADD t1, a1, x0\n");

    printf("  BGE t0, x0, left_skip_next%d\n", labelNum);
    //if left is negative flip its bits and add one to get unsigned value
    printf("  XORI t0, t0, -1\n");
    printf("  ADDI t0, t0, 1\n");

    printf("left_skip_next%d:\n", labelNum);
    char lab1[30];
    sprintf(lab1, "left_skip_next%d", labelNum);
    addLabel(gen, lab1);

    printf("  BGE t1, x0, right_skip_next%d\n", labelNum);
    //if right is positive flip bits and add one
    printf("  XORI t1, t1, -1\n");
    printf("  ADDI t1, t1, 1\n");

    printf("right_skip_next%d:\n", labelNum);
    char lab2[30];
    sprintf(lab2, "right_skip_next%d", labelNum);
    addLabel(gen, lab2);


    //multiply positive nums and store result in t0(upper) and t1(lower)
    char * regIn[] = {"t0", "t1"};
    char * regOut[] = {"t0", "t1"};
    callFunction(gen, "Multiplication_Unsigned_1Word", regIn, 2, regOut, 2);


    //if the original input signs are different, then negate value
    printf("  XOR t2, a0, a1\n");
    printf("  BGE t2, x0, different_signs_skip_next%d\n", labelNum);

    //invert bits and add 1 to negate
    printf("  XORI t0, t0, -1\n");
    printf("  XORI t1, t1, -1\n");


    //add one
    printf("  ADDI t2, x0, 1\n");
    char * regIn2[] = {"t0", "t1", "x0", "t2"};
    char * regOut2[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn2, 4, regOut2, 2);



    printf("different_signs_skip_next%d:\n", labelNum);
    char lab3[30];
    sprintf(lab3, "different_signs_skip_next%d", labelNum);
    addLabel(gen, lab3);
    //set output registers to result
    printf("  ADD a0, t0, x0\n");
    printf("  ADD a1, t1, x0\n");

    printf("  JALR x0, ra, 0\n");
}


void generateConvertFractionalToDecimalInt(CodeGenerator * gen){
    //converts fractional word into integer word of its representation in binary 
    //ex .75 gets turned to 75

    //input: a0 = fractional word
    //output: a0 = integer word
    //temp regs: t0 - result , t1 - upper, t2 - lower, t3 - 10

    /*
    
    ADD t0, x0, x0          :  intitialize result to zero
    ADD t2, a0, x0          :  initilize lower to input

    Loop:
        BEQ t2, x0, End


    
    
    End:
    
    */


   int labelNum = gen->numLabels++;

   printf("ConvertFractionToDecimalInteger:\n");

    //intitialize
   printf("  ADD t0, x0, x0\n");
   printf("  ADD t2, a0, x0\n");
   printf("  ADDI t3, x0, 10\n");



    printf("Loop%d:\n", labelNum);

    //if lower is zero, then end
    printf("  BEQ t2, x0, End%d\n", labelNum);
    
    //multiply lower by 10
    char * regIn[] = {"t2", "t3"};
    char * regOut[] = {"t1", "t2"};
    callFunction(gen, "Multiplication_Unsigned_1Word", regIn, 2, regOut, 2);

    //multiply result by 10
    char * regIn2[] = {"t0", "t3"};
    char * regOut2[] = {"x0", "t0"};

    //add upper to result, then reset upper
    printf("  ADD t0, t0, t1\n");
    printf("  ADD t1, x0, x0\n");

    printf("  JAL x0, Loop%d\n", labelNum);


    printf("End%d:\n",labelNum);

    printf("  ADD a0, t0, x0\n");
    printf("  JALR x0, ra, 0\n");


    
}


void generateUnsigned1WordMultiplicationFunction(CodeGenerator * gen){
   int labelNum = gen->next_label++;
   
   
    //input: a0 = left, a1 = right
    //output: a0 = result_upper, a1 = result_lower
    //temp registers: s0 = left upper, s1 = left lower, s2 = right upper, s3 = right lower, s4 = result upper, s5 = result lower
    printf("Multiplication_Unsigned_1Word:\n");
    addLabel(gen, "Multiplication_Unsigned_1Word");

    //save contents of S registers
    printf("  ADDI sp, sp, -24\n");
    printf("  SW s0, 0(sp)\n");
    printf("  SW s1, 4(sp)\n");
    printf("  SW s2, 8(sp)\n");
    printf("  SW s3, 12(sp)\n");
    printf("  SW s4, 16(sp)\n");
    printf("  SW s5, 20(sp)\n");


    //Initialize the contents of the S registers
    printf("  ADD s0, x0, x0\n");
    printf("  ADD s1, a0, x0\n");

    printf("  ADD s2, x0, x0\n");
    printf("  ADD s3, a1, x0\n");

    printf("  ADD s4, x0, x0\n");
    printf("  ADD s5, x0, x0\n");


    //Main Loop
    printf("Multiplication_Unsigned_1Word_loop:\n");
    addLabel(gen, "Multiplication_Unsigned_1Word_loop")

    //if the right reg is zero, end the loop
    printf("  BEQ s3, x0, Multiplication_Unsigned_1Word_end\n");

    //else check the LSB of if the right reg
    printf("  ANDI t0, s3, 1\n");
    //if its zero, then skip the addition part
    printf("  BEQ t0, x0, Multiplication_Unsigned_1Word_loop_end\n");
    

    char * regIn[] = {"s0", "s1", "s4", "s5"};
    char * regOut[] = {"s4", "s5"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);



    //after the addition is over, we shift the left and right regs
    printf("Multiplication_Unsigned_1Word_loop_end:\n");
    addLabel(gen, "Multiplication_Unsigned_1Word_loop_end");
    printf("  SRLI s3, s3, 1\n");

        //shifting left reg is challenging because we need to account for overflow. 
        //check if most significant bit is 1
   
    printf("  SLLI s0, s0, 1\n"); //shift upper left 1 to the left
    printf("  BGE s1, x0, skip_next%d\n", labelNum);
    printf(" ADDI s0, s0, 1\n");
    printf("skip_next%d:\n", labelNum);
    printf("  SLLI s1, s1, 1\n");

    //Now the loop is over, and we can do it again
    printf("  JAL x0, Multiplication_Unsigned_1Word_loop\n");




    //when loop is over, need to reset regs and stack
    printf("Multiplication_Unsigned_1Word_end:\n");
    addLabel(gen, "Multiplication_Unsigned_1Word_end");

    printf("  ADD a0, s4, x0\n");
    printf("  ADD a1, s5, x0\n");


    //return contents of S registers
    printf("  LW s0, 0(sp)\n");
    printf("  LW s1, 4(sp)\n");
    printf("  LW s2, 8(sp)\n");
    printf("  LW s3, 12(sp)\n");
    printf("  LW s4, 16(sp)\n");
    printf("  LW s5, 20(sp)\n");
    printf("  ADDI sp, sp, 24\n");

    printf("  JALR x0, ra, 0\n");
    
}

void generateSignedFixedPointMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left upper, a1 = left lower, a2 = right upper, a3 = right lower
    //output: a0 = result upper, a1 = result lower
    
    int labelNum = gen->next_label++;
    printf("Multiplication_Signed_FixedPoint:\n");
    addLabel(gen, "Multiplication_Signed_FixedPoint");

    //make copies of the inputs
    printf("  ADD t0, a0, x0\n");
    printf("  ADD t1, a1, x0\n");
    printf("  ADD t2, a2, x0\n");
    printf("  ADD t3, a3, x0\n");

    //if left is negative, negate
    printf("  BGE t0, x0, left_skip_next%d\n", labelNum);
    printf("  XORI t0, t0, -1\n");
    printf("  XORI t1, t1, -1\n");

    printf("  ADDI t5, x0, 1\n");
    char * regIn[] = {"t0", "t1", "x0", "t5"};
    char * regOut[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);
    
    printf("left_skip_next%d:\n", labelNum);
    char lab1[30];
    sprintf(lab1, "left_skip_next%d", labelNum);
    addLabel(gen, lab1);


    //if right is negative, negate
    printf("  BGE t2, x0, right_skip_next%d\n", labelNum);
    printf("  XORI t2, t0, -1\n");
    printf("  XORI t3, t1, -1\n");

    printf("  ADDI t5, x0, 1\n");
    char * regIn2[] = {"t2", "t3", "x0", "t5"};
    char * regOut2[] = {"t2", "t3"};
    callFunction(gen, "Addition_2Word", regIn2, 4, regOut2, 2);
    
    printf("right_skip_next%d:\n", labelNum);
    char lab2[30];
    sprintf(lab2, "right_skip_next%d", labelNum);
    addLabel(gen, lab2);


    //multiply unsigned values together
    char * regIn3[] = {"t0", "t1", "t2", "t3"};
    char * regOut3[] = {"t0", "t1"};
    callFunction(gen, "Multiplication_Unsigned_FixedPoint", regIn3, 4, regOut3, 2);


    //if input signs are different then negate answer
    printf("  XOR t3, a0, a2\n");
    printf("  BGE t3, x0, different_signs_skip_next%d\n", labelNum);
    
    printf("  XORI t0, t0, -1\n");
    printf("  XORI t1, t1, -1\n");
    printf("  ADDI t3, x0, 1\n");
    char * regIn4[] = {"t0", "t1", "x0", "t3"};
    char * regOut4[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn4, 4, regOut4, 2);

    printf("different_signs_skip_next%d:\n", labelNum);
    char lab3[30];
    sprintf(lab3, "different_signs_skip_next%d", labelNum);
    addLabel(gen, lab3);

    //set outputs and return
    printf("  ADD a0, t0, x0\n");
    printf("  ADD a1, t1, x0\n");

    printf("  JALR x0, ra, 0\n");
}

void generateUnsignedFixedPointMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left upper, a1 = left lower, a2 = right upper, a3 = right lower
    //output: a0 = result upper, a1 = result lower

    //temp: s0 = result upper, s1 = result lower
    //      s2 = Term 1 upper, s3 = Term 1 lower
    //      s4 = Term 2 upper, s5 = Term 2 lower
    //      s6 = Term 3 upper, s7 = Term 3 lower
    //      s8 = Term 4 upper, s9 = Term 4 lower

    printf("Multiplication_Unsigned_FixedPoint:\n");
    addLabel(gen, "Multiplication_Unsigned_FixedPoint");

    //save s regs to stack
    printf("  ADDI sp, sp, -40\n");
    printf("  SW s0, 0(sp)\n");
    printf("  SW s1, 4(sp)\n");
    printf("  SW s2, 8(sp)\n");
    printf("  SW s3, 12(sp)\n");
    printf("  SW s4, 16(sp)\n");
    printf("  SW s5, 20(sp)\n");
    printf("  SW s6, 24(sp)\n");
    printf("  SW s7, 28(sp)\n");
    printf("  SW s8, 32(sp)\n");
    printf("  SW s9, 36(sp)\n");   


    //initialize s regs
    printf("  ADD s0, x0, x0\n");
    printf("  ADD s1, x0, x0\n");

    //Term 1 (whole) = lower res of (left upper * right upper)
    //Term 2 (fractional) = lower res of (left upper * right_lower)
    //Term 3 (fractional) = lower res of (left lower * right_upper)
    //Term 4 (fractional) = upper res of (left lower * right lower)

    //////////////////////////////////////////
    //Get term 1
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    printf("  ADDI sp, sp, -12\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW ra, 8(sp)\n");

    //load args
    printf("  ADD a0, a0, x0\n");
    printf("  ADD a1, a2, x0\n");
    printf("  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    printf("  ADD s2, a0, x0\n");
    printf("  ADD s3, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW ra, 8(sp)\n");
    printf("  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 2
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    printf("  ADDI sp, sp, -12\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW ra, 8(sp)\n");

    //load args
    printf("  ADD a0, a0, x0\n");
    printf("  ADD a1, a3, x0\n");
    printf("  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    printf("  ADD s4, a0, x0\n");
    printf("  ADD s5, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW ra, 8(sp)\n");
    printf("  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 3
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    printf("  ADDI sp, sp, -12\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW ra, 8(sp)\n");

    //load args
    printf("  ADD a0, a1, x0\n");
    printf("  ADD a1, a2, x0\n");
    printf("  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    printf("  ADD s6, a0, x0\n");
    printf("  ADD s7, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW ra, 8(sp)\n");
    printf("  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 4
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    printf("  ADDI sp, sp, -12\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW ra, 8(sp)\n");

    //load args
    printf("  ADD a0, a1, x0\n");
    printf("  ADD a1, a3, x0\n");
    printf("  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    printf("  ADD s8, a0, x0\n");
    printf("  ADD s9, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW ra, 8(sp)\n");
    printf("  ADDI sp, sp, 12\n");





    //Now all the temporary results are stored in s2-s9
    //Lower part of term1 is added to upper part of result
    //Lower part of terms 2 and 3 are added to lower part of result
    //Upper part of term4 is added to lower part of result
    
    //The add two word function has input and output:
        //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
        //output: a0 = result_upper , a1 = result_lower

        //Add term 1 to the res
    printf("  ADDI sp, sp, -20\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW a2, 8(sp)\n");
    printf("  SW a3, 12(sp)\n");
    printf("  SW ra, 16(sp)\n");

    printf("  ADD a0, s0, x0\n"); //left is current result
    printf("  ADD a1, s1, x0\n");
    printf("  ADD a2, s3, x0\n"); //right upper is term 1 lower
    printf("  ADD a3, x0, x0\n"); // right lower is zero

    printf("  JAL ra, Addition_2Word\n");
    printf("  ADD s0, a0, x0\n");
    printf("  ADD s1, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW a2, 8(sp)\n");
    printf("  LW a3, 12(sp)\n");
    printf("  LW ra, 16(sp)\n");
    printf("  ADDI sp, sp, 20\n");

        //Add term 2 to the res
    printf("  ADDI sp, sp, -20\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW a2, 8(sp)\n");
    printf("  SW a3, 12(sp)\n");
    printf("  SW ra, 16(sp)\n");

    printf("  ADD a0, s0, x0\n"); //left is current result
    printf("  ADD a1, s1, x0\n");
    printf("  ADD a2, s4, x0\n"); //right upper is term 2 upper
    printf("  ADD a3, s5, x0\n"); //right lower is term 2 lower

    printf("  JAL ra, Addition_2Word\n");
    printf("  ADD s0, a0, x0\n");
    printf("  ADD s1, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW a2, 8(sp)\n");
    printf("  LW a3, 12(sp)\n");
    printf("  LW ra, 16(sp)\n");
    printf("  ADDI sp, sp, 20\n");

        //Add term 3 to the res
    printf("  ADDI sp, sp, -20\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW a2, 8(sp)\n");
    printf("  SW a3, 12(sp)\n");
    printf("  SW ra, 16(sp)\n");

    printf("  ADD a0, s0, x0\n"); //left is current result
    printf("  ADD a1, s1, x0\n");
    printf("  ADD a2, s6, x0\n"); //right upper is term 3 upper
    printf("  ADD a3, s7, x0\n"); //right lower is term 3 lower

    printf("  JAL ra, Addition_2Word\n");
    printf("  ADD s0, a0, x0\n");
    printf("  ADD s1, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW a2, 8(sp)\n");
    printf("  LW a3, 12(sp)\n");
    printf("  LW ra, 16(sp)\n");
    printf("  ADDI sp, sp, 20\n");



        //Add term 4 to the res
    printf("  ADDI sp, sp, -20\n");
    printf("  SW a0, 0(sp)\n");
    printf("  SW a1, 4(sp)\n");
    printf("  SW a2, 8(sp)\n");
    printf("  SW a3, 12(sp)\n");
    printf("  SW ra, 16(sp)\n");

    printf("  ADD a0, s0, x0\n"); //left is current result
    printf("  ADD a1, s1, x0\n");
    printf("  ADD a2, x0, x0\n"); //right upper is zero
    printf("  ADD a3, s8, x0\n"); //right lower is term 4 upper

    printf("  JAL ra, Addition_2Word\n");
    printf("  ADD s0, a0, x0\n");
    printf("  ADD s1, a1, x0\n");


    printf("  LW a0, 0(sp)\n");
    printf("  LW a1, 4(sp)\n");
    printf("  LW a2, 8(sp)\n");
    printf("  LW a3, 12(sp)\n");
    printf("  LW ra, 16(sp)\n");
    printf("  ADDI sp, sp, 20\n");


    //Now our final result is stored in reg s0, and s1.
    printf("  ADD a0, s0, x0\n");
    printf("  ADD a1, s1, x0\n");


    printf("  LW s0, 0(sp)\n");
    printf("  LW s1, 4(sp)\n");
    printf("  LW s2, 8(sp)\n");
    printf("  LW s3, 12(sp)\n");
    printf("  LW s4, 16(sp)\n");
    printf("  LW s5, 20(sp)\n");
    printf("  LW s6, 24(sp)\n");
    printf("  LW s7, 28(sp)\n");
    printf("  LW s8, 32(sp)\n");
    printf("  LW s9, 36(sp)\n");    
    printf("  ADDI sp, sp, 40\n");

    printf("  JALR x0, ra, 0\n");

}

void generateUnsigned2WordDivisionFunction(CodeGenerator * gen){
    //input: a0 = left upper, a1 = left lower, a2 = right upper, a3 = right lower
    //output: a0 = res upper upper, a1 = res upper, a2 = res lower, a3 = res lower lower
    //temp: s0 = Quotient Upper Upper, s1 = Quotient Upper, s2 = Quotient Lower, s3 = Quotient lower lower
    //      s4 = Working Dividend (WD) Upper, s5 = WD Lower
    //      s6 = Dividend upper, s7 = Dividend Lower
    //      s8 = Iteration Count, s9 = shift count
    //      s10 = Divisor Upper, s11 = Divisor Lower


    /*
        Initialize Variables, Quotient = 0, WD = 0, Copy of Dividend (left), Iteration Count = 1

        Until dividend >= divisor, shift dividend left or divisor right

        Loop:

            If WD and Dividend Copy = 0,  or Iteration Count = 64 + 1 then break

            WD = (WD << 1) + MSB of Dividend Copy
            Dividend Copy = Dividend Copy << 1

            If WD >= Divisor (right):
                WD = WD - Divisor
                Quotient at the (Iteration Count) 'th bit (starting from MSB) = 1

            Else:
                Quotient at the (Iteration Count) 'th bit (starting from MSB) = 0
  
            Iteration Count ++

        Shift answer back based on shift count 

    */


   
    int labelNum = gen->next_label++;
    printf("Division_Unsigned_2Word:\n");
    addLabel(gen, "Division_Unsigned_2Word");

    //Store S registers to stack
    printf("  ADDI sp, sp, -48\n");
    printf("  SW s0, 0(sp)\n");
    printf("  SW s1, 4(sp)\n");
    printf("  SW s2, 8(sp)\n");
    printf("  SW s3, 12(sp)\n");
    printf("  SW s4, 16(sp)\n");
    printf("  SW s5, 20(sp)\n");
    printf("  SW s6, 24(sp)\n");
    printf("  SW s7, 28(sp)\n");
    printf("  SW s8, 32(sp)\n");
    printf("  SW s9, 36(sp)\n");
    printf("  SW s10, 40(sp)\n");
    printf("  SW s11, 44(sp)\n");


    //Initialize Registers
    printf("  ADD s0, x0, x0\n");
    printf("  ADD s1, x0, x0\n");
    printf("  ADD s2, x0, x0\n");
    printf("  ADD s3, x0, x0\n");
    printf("  ADD s4, x0, x0\n");
    printf("  ADD s5, x0, x0\n");
    printf("  ADD s6, a0, x0\n");
    printf("  ADD s7, a1, x0\n");
    printf("  ADDI s8, x0, 1\n");
    printf("  ADD s9, x0, x0\n");
    printf("  ADD s10, a2, x0\n");
    printf("  ADD s11, a3, x0\n");




    //Until dividend >= divisor, shift dividend left or divisor right
    
    

    printf("Division_Unsigned_2Word_Shift_Loop:\n");
    addLabel(gen,"Division_Unsigned_2Word_Shift_Loop");

    printf("  BLTU s6, s10, Action%d\n", labelNum); // if upper dividend < upper divisor, then shift 
    printf("  BLTU s10, s6, Division_Unsigned_2Word_Shift_Loop_End\n");// else if upper divisor < upper dividend, dont shift
    printf("  BGEU s7, s11, Division_Unsigned_2Word_Shift_Loop_End\n");// else compare lower words
    
    printf("Action%d:\n", labelNum);
    char lab1[30];
    sprintf(lab1, "Action%d", labelNum);
    addLabel(gen, lab1);
    
    //shift dividend to the left
    printf("   SLT t0, s7, x0\n"); //t0 = MSB of lower dividend
    printf("  SLLI s6, s6, 1\n");
    printf("  SLLI s7, s7, 1\n");
    printf("  ADD s6, s6, t0\n");
    printf("  ADDI s9, s9, 1\n"); //increment shift count

    printf("  JAL x0, Division_Unsigned_2Word_Shift_Loop\n");

    printf("Division_Unsigned_2Word_Shift_Loop_End:\n");
    addLabel(gen, "Division_Unsigned_2Word_Shift_Loop_End");

    
    printf("Division_Unsigned_2Word_Loop:\n");
    addLabel(gen, "Division_Unsigned_2Word_Loop");

    //If WD and Dividend Copy = 0,  or Iteration Count = 64 + 1 then break
    printf("  ADDI t0, x0, 129\n");
    printf("  BEQ s8, t0, Division_Unsigned_2Word_Loop_End\n");
    printf("  BNE s4, x0, loop_skip_next%d\n", labelNum);
    printf("  BNE s5, x0, loop_skip_next%d\n", labelNum);
    printf("  BNE s6, x0, loop_skip_next%d\n", labelNum);
    printf("  BEQ s7, x0, Division_Unsigned_2Word_Loop_End\n");


    printf("loop_skip_next%d:\n", labelNum);
    char lab2[30];
    sprintf(lab2, "loop_skip_next%d", labelNum);
    addLabel(gen, lab2);

    //WD = (WD << 1) + MSB of Dividend Copy
    printf("  SLT t0, s6, x0\n"); // t0 = MSB of Divident Copy
    
        //shift WD to left, (need to check MSB of lower word)
    printf("  SLT t1, s5, x0\n"); 
    printf("  SLLI s5, s5, 1\n");
    printf("  SLLI s4, s4, 1\n");
    printf("  ADD s4, s4, t1\n");

        //Add t0 to WD
    char * regIn[] = {"s4", "s5", "x0", "t0"};
    char * regOut [] = {"s4", "s5"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);

    //Dividend Copy = Dividend Copy << 1
    printf("   SLT t0, s7, x0\n"); //t0 = MSB of Divident Copy Lower
    printf("  SLLI s6, s6, 1\n");
    printf("  SLLI s7, s7, 1\n");
    printf("  ADD s6, s6, t0\n");


    //If WD >= Divisor (right):
    //    WD = WD - Divisor
    //    Quotient at the (Iteration Count) 'th bit (starting from MSB) = 1
    //Else:
    //      Quotient at the (Iteration Count) 'th bit (starting from MSB) = 0



    printf("  BLTU s4, a2, Division_Unsigned_2Word_Loop_Branch_False\n");
    printf("  BLTU a2, s4, Division_Unsigned_2Word_Loop_Branch_True\n");

        //if top words are equal compare bottom words
    printf("  BLTU s5, a3, Division_Unsigned_2Word_Loop_Branch_False\n");
    

    printf("Division_Unsigned_2Word_Loop_Branch_True:\n");
    addLabel(gen, "Division_Unsigned_2Word_Loop_Branch_True:");
    //      WD = WD - Divisor

    char * regIn2[] = {"s4", "s5", "a2", "a3"};
    char * regOut2[] = {"s4", "s5"};
    callFunction(gen, "Subtraction_2Word", regIn2, 4, regOut2, 2);

    //      Quotient = Quotient with (Iteration Count)th bit = 1
    printf("  ADDI t0, x0, 97\n");
    printf("  BGEU s8, t0, Iteration_GT_96%d\n", labelNum);
    printf("  ADDI t0, x0, 65\n");
    printf("  BGEU s8, t0, Iteration_GT_64%d\n", labelNum);
    printf("  ADDI t0, x0, 33\n");
    printf("  BGEU s8, t0, Iteration_GT_32%d\n", labelNum);
    printf("  JAL x0, Iteration_LTE_32%d\n", labelNum);


    printf("Iteration_GT_96%d:\n", labelNum);
    char lab3[30];
    sprintf(lab3, "Iteration_GT_96%d", labelNum);
    addLabel(gen, lab3);

    printf("  ADDI t1, x0, 128\n");
    printf("  SUB t1, t1, s8\n"); //t1 = 128 - Iteration Count
    printf("  ADDI t2, x0, 1\n");
    printf("  SLL t1, t2, t1\n"); //t1 = 1 << (128 - Iteration Count)
    printf("  OR s3, s3, t1\n"); //Lower Quotient Bit = 1;
    printf("  JAL x0, Iteration_Branch_End%d\n", labelNum);

    printf("Iteration_GT_64%d:\n", labelNum);
    char lab4[30];
    sprintf(lab4, "Iteration_GT_64%d", labelNum);
    addLabel(gen, lab4);

    printf("  ADDI t1, x0, 96\n");
    printf("  SUB t1, t1, s8\n"); //t1 = 96 - Iteration Count
    printf("  ADDI t2, x0, 1\n");
    printf("  SLL t1, t2, t1\n"); //t1 = 1 << (96 - Iteration Count)
    printf("  OR s2, s2, t1\n"); //Lower Quotient Bit = 1;
    printf("  JAL x0, Iteration_Branch_End%d\n", labelNum);

    printf("Iteration_GT_32%d:\n", labelNum);
    char lab5[30];
    sprintf(lab5, "Iteration_GT_32%d", labelNum);
    addLabel(gen, lab5);

    printf("  ADDI t1, x0, 64\n");
    printf("  SUB t1, t1, s8\n"); //t1 = 64 - Iteration Count
    printf("  ADDI t2, x0, 1\n");
    printf("  SLL t1, t2, t1\n"); //t1 = 1 << (64 - Iteration Count)
    printf("  OR s1, s1, t1\n"); //Upper Quotient Bit = 1;
    printf("  JAL x0, Iteration_Branch_End%d\n", labelNum);

    printf("Iteration_LTE_32%d:\n",labelNum);
    char lab6[30];
    sprintf(lab6, "Iteration_LTE_32%d", labelNum);
    addLabel(gen, lab6);

    printf("  ADDI t1, x0, 32\n");
    printf("  SUB t1, t1, s8\n"); //t1 = 32 - Iteration Count
    printf("  ADDI t2, x0, 1\n");
    printf("  SLL t1, t2, t0\n"); //t1 = 1 << (32 - Iteration Count)
    printf("  OR s0, s0, t1\n"); //Upper Upper Bit = 1;
    
    printf("Iteration_Branch_End%d:\n", labelNum);
    char lab7[30];
    sprintf(lab7, "Iteration_Branch_End%d", labelNum);
    addLabel(gen, lab7);
    printf("  JAL x0, Division_Unsigned_2Word_Loop_Branch_End\n");


    printf("Division_Unsigned_2Word_Loop_Branch_False:\n");
    addLabel(gen, "Division_Unsigned_2Word_Loop_Branch_False");
    printf("Division_Unsigned_2Word_Loop_Branch_End:\n");
    addLabel(gen, "Division_Unsigned_2Word_Loop_Branch_End");



    printf("  ADDI s8, s8, 1\n");
    printf("  JAL x0, Division_Unsigned_2Word_Loop\n");

    printf("Division_Unsigned_2Word_Loop_End:\n");
    addLabel(gen, "Division_Unsigned_2Word_Loop_End");



    //shift output to the right based on number of initial shifts


    printf("Shift_Right_Loop%d:\n", labelNum);
    char lab8[30];
    sprintf(lab8, "Shift_Right_Loop%d", labelNum);
    addLabel(gen, lab8);

    printf("  BGE x0, s9, Shift_Right_Loop_End%d\n", labelNum);
    printf("  ANDI t0, s0, 1\n"); //t0 = LSB of upper upper Quotient
    printf("  ANDI t1, s1, 1\n"); //t1 = LSB of upper Quotient
    printf("  ANDI t2, s2, 1\n"); //t2 = LSB of lower Quotient
    printf("  SLLI t0, t0, 31\n");
    printf("  SLLI t1, t1, 31\n");
    printf("  SLLI t2, t2, 31\n");
    
    printf("  SRLI s0, s0, 1\n");
    printf("  SRLI s1, s1, 1\n");
    printf("  SRLI s2, s2, 1\n");
    printf("  SRLI s3, s3, 1\n");



    printf("  OR s1, s1, t0\n");
    printf("  OR s2, s2, t1\n");
    printf("  OR s3, s3, t2\n");
    printf("  ADDI s9, s9, -1\n"); //decrement shift counter
    printf("  JAL x0, Shift_Right_Loop%d\n", labelNum);

    printf("Shift_Right_Loop_End%d:\n", labelNum);
    char lab9[30];
    sprintf(lab9, "Shift_Right_Loop_End%d", labelNum);
    addLabel(gen, lab9);
    //set outputs
    printf("  ADD a0, s0, x0\n");
    printf("  ADD a1, s1, x0\n");
    printf("  ADD a2, s2, x0\n");
    printf("  ADD a3, s3, x0\n");

    //restore S registers from stack


    printf("  LW s0, 0(sp)\n");
    printf("  LW s1, 4(sp)\n");
    printf("  LW s2, 8(sp)\n");
    printf("  LW s3, 12(sp)\n");
    printf("  LW s4, 16(sp)\n");
    printf("  LW s5, 20(sp)\n");
    printf("  LW s6, 24(sp)\n");
    printf("  LW s7, 28(sp)\n");
    printf("  LW s8, 32(sp)\n");
    printf("  LW s9, 36(sp)\n");
    printf("  LW s10, 40(sp)\n");
    printf("  LW s11, 44(sp)\n");
    printf("  ADDI sp, sp, 48\n");

    printf("  JALR x0, ra, 0\n");

}

void generatePrintFunction(CodeGenerator * gen){
    //address in a0

    printf("  Print:\n");

    printf("  ADDI sp, sp, -4");
    printf("  SW a7, 0(sp)\n");

    printf("  ADDI a7, x0, 4");
    printf("  ecall\n");


    printf("  LW a7, 0(sp)\n");
    printf("  ADDI sp, sp, 4\n");
}

void generateOrFunction(CodeGenerator * gen){
    //a0 left, a1 right

    printf("  ADD t1, a0, a1\n");
    printf("  SLTU a0, x0, t1\n");
    printf("  JALR x0, ra, 0\n");
}

void generateAndFunction(CodeGenerator * gen){

    printf("  ADD t1, a0, a1\n");
    printf("  ADDI t1, t1, -1\n");
    printf("  SLT a0, x0, t1\n");
    printf("  JALR x0, ra, 0\n");

}

void generateNotFunction(CodeGenerator * gen){
    printf("  SLTUI a0, a0, 1\n");
}




void generate2WordLessThanFunction(CodeGenerator * gen){
    //a0  =  leftUpper, a1 = leftLower, a2 = rightUpper, a3 =rightLower
    // s1 = are they positive


    //compare upper
    //if left upper < right Upper, true
    //if left upper > right Upper, false
    //if left upper = right Upper, compare Lower words

    //compare lower
    // if left lower < right lower,  return are they positive
    // if left lower > right lower,  return are they negative
    // if left lower = right lower ,  return false



    int labelNum = gen->numLabels++;
    printf("2WordLT:\n");
    
    printf("  BLT a0, a2, EndTrue%d\n", labelNum);
    printf("  BLT a2, a0, EndFalse%d\n", labelNum);
    printf("  SLT t0, a0, x0\n");  //t0 = are they negative
    printf("  XORI t1, t0, 1\n");   //t1 = not t0
    printf("  BLTU a1, a3, LowerLT%d\n", labelNum);
    printf("  BLTU a3, a1, LowerGT%d\n", labelNum);

    printf("  ADDI a0, x0, 0\n");
    printf("  JALR x0, ra, 0\n");

    printf("EndTrue%d:\n", labelNum);
    printf("  ADDI a0, x0, 1\n");
    printf("  JALR x0, ra, 0\n");
    
    printf("EndFalse%d:\n", labelNum);
    printf("  ADDI a0, x0, 0\n");
    printf("  JALR x0, ra, 0\n");

    printf("LowerLT%d:\n", labelNum);
    printf("  ADD a0, t0, x0\n");
    printf("  JALR x0, ra, 0\n");

    printf("LowerGT%d:\n", labelNum);
    printf("  ADD a0, t1, x0\n");
    printf("  JALR x0, ra, 0\n");




}

void generate2WordLessThanEqualFunction(CodeGenerator * gen){

   //a0  =  leftUpper, a1 = leftLower, a2 = rightUpper, a3 =rightLower
    // s1 = are they positive


    //compare upper
    //if left upper < right Upper, true
    //if left upper > right Upper, false
    //if left upper = right Upper, compare Lower words

    //compare lower
    // if left lower < right lower,  return are they positive
    // if left lower > right lower,  return are they negative
    // if left lower = right lower ,  return true



    int labelNum = gen->numLabels++;
    printf("2WordLT:\n");
    
    printf("  BLT a0, a2, EndTrue%d\n", labelNum);
    printf("  BLT a2, a0, EndFalse%d\n", labelNum);
    printf("  SLT t0, a0, x0\n");  //t0 = are they negative
    printf("  XORI t1, t0, 1\n");   //t1 = not t0
    printf("  BLTU a1, a3, LowerLT%d\n", labelNum);
    printf("  BLTU a3, a1, LowerGT%d\n", labelNum);

    printf("  ADDI a0, x0, 1\n");
    printf("  JALR x0, ra, 0\n");

    printf("EndTrue%d:\n", labelNum);
    printf("  ADDI a0, x0, 1\n");
    printf("  JALR x0, ra, 0\n");
    
    printf("EndFalse%d:\n", labelNum);
    printf("  ADDI a0, x0, 0\n");
    printf("  JALR x0, ra, 0\n");

    printf("LowerLT%d:\n", labelNum);
    printf("  ADD a0, t0, x0\n");
    printf("  JALR x0, ra, 0\n");

    printf("LowerGT%d:\n", labelNum);
    printf("  ADD a0, t1, x0\n");
    printf("  JALR x0, ra, 0\n");




}










void statementsNode(CodeGenerator * gen, AST_Node * node){
    for(int i = 0; i < node->num_children; i++){
        AST_Node * child = node->children[i];
        generateCode(gen, child);
    }
}

void divideNode(CodeGenerator * gen, AST_Node * node){

    if(node->num_children != 3 || !compareSymbols(&node->children[1]->symbol, &SlashSymbol)){
        printf("incorrectly calling divide: num children = %d\n", node->num_children);
    }

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    AST_Node * left = node->children[0];
    AST_Node * right = node->children[2];

    generateCode(gen, left);
    generateCode(gen, right);

   

    //x / x = float
    node->resultType = TYPE_DECIMAL;\
    

    Register *left_lower_reg, *left_upper_reg, *right_lower_reg, *right_upper_reg;

    Register *result_lower_reg, *result_upper_reg, *result_lower_lower_reg, *result_upper_upper_reg;



        //load lower values into regs (x0 if its an integer)
    if(left->resultType == TYPE_DECIMAL) left_lower_reg = get_register(gen, left, 1,  doNotSpill, 0);
    else if(left->resultType == TYPE_INTEGER) left_lower_reg = &registerX0;
    else printf("somethings wrong in divideNode\n");
    doNotSpill[0] = left_lower_reg;



    if(right->resultType == TYPE_DECIMAL) right_lower_reg = get_register(gen, right, 1,  doNotSpill, 1);
    else if(right->resultType == TYPE_INTEGER) right_lower_reg = &registerX0;
    else printf("somethings wrong in divideNode\n");
    doNotSpill[1] = right_lower_reg;


    //load upper values into regs
    left_upper_reg = get_register(gen, left, 0, doNotSpill, 2);
    doNotSpill[2] = left_upper_reg;
    right_upper_reg = get_register(gen, right, 0, doNotSpill, 3);
    doNotSpill[3] = right_upper_reg;

    //get result registers


    result_upper_reg = allocate_register(gen, node, 0, doNotSpill, 4, -1);
    doNotSpill[4] = result_lower_lower_reg;
    result_lower_reg = allocate_register(gen, node, 1, doNotSpill, 5, -1);
    doNotSpill[5] = result_lower_reg;
    


    char * regNameIn[] = {left_upper_reg->name, left_lower_reg->name, right_upper_reg->name, right_lower_reg->name};
    char * regNameOut[] = {"x0", result_upper_reg->name, result_lower_reg->name,  "x0"};
    callFunction(gen, "Division_Unsigned_2Word", regNameIn, 4, regNameOut, 4);



    free_register(gen, left_lower_reg);
    free_register(gen, left_upper_reg);
    free_register(gen, right_upper_reg);
    free_register(gen, right_lower_reg);
 

}

void multiplyNode(CodeGenerator * gen, AST_Node * node){
    if(node->num_children != 3 || !compareSymbols(&node->children[1]->symbol, &StarSymbol)){
        printf("incorrectly calling multiply: num children = %d\n", node->num_children);
    }

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL, NULL, NULL};

    AST_Node * left = node->children[0];
    AST_Node * right = node->children[2];

    generateCode(gen, left);
    generateCode(gen, right);

    if(left->resultType == TYPE_INTEGER && right->resultType == TYPE_INTEGER){
        //int * int = int
        node->resultType = TYPE_INTEGER;

      //  Register *left_reg, *right_reg, *result_reg_upper, *result_reg_lower;


        Register *left_reg = get_register(gen, node->children[0], 0,  doNotSpill, 0);
        doNotSpill[0] = left_reg;
        Register *right_reg = get_register(gen, node->children[2], 0,  doNotSpill, 1);
        doNotSpill[1] = right_reg;

        Register * result_lower = allocate_register(gen, node, 0, doNotSpill, 2, -1);
        doNotSpill[2] = result_lower;



        char * regIn[] = {left_reg->name, right_reg->name};
        char * regOut[] = {"x0", result_lower->name};
        callFunction(gen, "Multiplication_Signed_1Word", regIn, 2, regOut, 2);

        free_register(gen, left_reg);
        free_register(gen, right_reg);
        return;
    }

    if(left->resultType == TYPE_DECIMAL || right->resultType == TYPE_DECIMAL){
        //float * x = float
        node->resultType == TYPE_DECIMAL;
        Register *left_lower_reg, *left_upper_reg, *right_lower_reg, *right_upper_reg;
        Register *result_lower_reg, *result_upper_reg;

        //load lower values into regs (x0 if its an integer)
        if(left->resultType == TYPE_DECIMAL) left_lower_reg = get_register(gen, left, 1,  doNotSpill, 0);
        else if(left->resultType == TYPE_INTEGER) left_lower_reg = &registerX0;
        else printf("somethings wrong in multiplyNode\n");
        doNotSpill[0] = left_lower_reg;

        if(right->resultType == TYPE_DECIMAL) right_lower_reg = get_register(gen, right, 1,  doNotSpill, 1);
        else if(right->resultType == TYPE_INTEGER) right_lower_reg = &registerX0;
        else printf("somethings wrong in multiplyNode\n");
        doNotSpill[1] = right_lower_reg;
        
        //load upper values into regs
        left_upper_reg = get_register(gen, left, 0, doNotSpill, 2);
        doNotSpill[2] = left_upper_reg;
        right_upper_reg = get_register(gen, right, 0, doNotSpill, 3);
        doNotSpill[3] = right_upper_reg;

        //get result registers
        result_lower_reg = allocate_register(gen, node, 1, doNotSpill, 4, -1);
        doNotSpill[4] = result_lower_reg;
        result_upper_reg = allocate_register(gen, node, 0, doNotSpill, 5, -1);

        char * regNameIn[] = {left_upper_reg->name, left_lower_reg->name, right_upper_reg->name, right_lower_reg->name};
        char * regNameOut[] = {result_upper_reg->name, result_lower_reg->name};
        callFunction(gen, "Multiplication_Signed_FixedPoint", regNameIn, 4, regNameOut, 2);


        free_register(gen, left_lower_reg);
        free_register(gen, left_upper_reg);
        free_register(gen, right_lower_reg);
        free_register(gen, right_upper_reg);

    }
}

void addNode(CodeGenerator * gen, AST_Node * node){
    
    if(node->num_children != 3 || !compareSymbols(&node->children[1]->symbol, &PlusSymbol)){
        printf("incorrectly calling addNode: num children = %d\n", node->num_children);
    }

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL, NULL, NULL};

    AST_Node * left = node->children[0];
    AST_Node * right = node->children[2];

    generateCode(gen, left);
    generateCode(gen, right);

    if(left->resultType == TYPE_INTEGER && right->resultType == TYPE_INTEGER){
        //adding two integers
        node->resultType = TYPE_INTEGER;

        Register *left_reg = get_register(gen, node->children[0], 0,  doNotSpill, 0);
        doNotSpill[0] = left_reg;
        Register *right_reg = get_register(gen, node->children[2], 0,  doNotSpill, 1);
        doNotSpill[1] = right_reg;
        Register *result_reg = allocate_register(gen, node, 0, doNotSpill, 2, -1);

        
        printf("  ADD %s, %s, %s\n", result_reg->name, left_reg->name, right_reg->name);

    
        free_register(gen, left_reg);
        free_register(gen, right_reg);
        return;
    }

    if(left->resultType == TYPE_DECIMAL || right->resultType == TYPE_DECIMAL){
        //float + x = float
        node->resultType = TYPE_DECIMAL;
        Register *left_lower_reg, *left_upper_reg, *right_lower_reg, *right_upper_reg;
        Register *result_lower_reg, *result_upper_reg;

        //load lower values into regs (x0 if its an integer)
        if(left->resultType == TYPE_DECIMAL) left_lower_reg = get_register(gen, left, 1,  doNotSpill, 0);
        else if(left->resultType == TYPE_INTEGER) left_lower_reg = &registerX0;
        else printf("somethings wrong in add_node\n");
        doNotSpill[0] = left_lower_reg;

        if(right->resultType == TYPE_DECIMAL) right_lower_reg = get_register(gen, right, 1,  doNotSpill, 1);
        else if(right->resultType == TYPE_INTEGER) right_lower_reg = &registerX0;
        else printf("somethings wrong in add_node\n");
        doNotSpill[1] = right_lower_reg;


        //load upper values into regs
        left_upper_reg = get_register(gen, left, 0, doNotSpill, 2);
        doNotSpill[2] = left_upper_reg;
        right_upper_reg = get_register(gen, right, 0, doNotSpill, 3);
        doNotSpill[3] = right_upper_reg;

        //get result registers
        result_lower_reg = allocate_register(gen, node, 1, doNotSpill, 4, -1);
        doNotSpill[4] = result_lower_reg;
        result_upper_reg = allocate_register(gen, node, 0, doNotSpill, 5, -1);

        char * regNameIn[] = {left_upper_reg->name, left_lower_reg->name, right_upper_reg->name, right_lower_reg->name};
        char * regNameOut[] = {result_upper_reg->name, result_lower_reg->name};
        callFunction(gen, "Addition_2Word", regNameIn, 4, regNameOut, 2);

        free_register(gen, left_lower_reg);
        free_register(gen, left_upper_reg);
        free_register(gen, right_lower_reg);
        free_register(gen, right_upper_reg);


    }

}

void castIntToDec(CodeGenerator* gen, Register * intReg, Register * decUpperReg, Register * decLowerReg){

    printf("  ADD %s, %s, x0\n",decUpperReg->name, intReg->name);
    printf("  ADD %s, x0, x0\n", decLowerReg->name);
}

void castDecToInt(CodeGenerator * gen, Register * decUpperReg, Register * decLowerReg, Register * intReg){

    Register * doNotSpill[] = {decUpperReg, decLowerReg, intReg, NULL, NULL};
    //if dec upper is negative && dec lower > 0 
 
    //  then int = dec upper + 1
    //else int =  dec upper

    Register * temp_reg1 = allocate_register(gen, NULL, 0, doNotSpill, 3, -1);
    doNotSpill[3] = temp_reg1;
    Register * temp_reg2 = allocate_register(gen, NULL, 0, doNotSpill, 4, -1);
    printf("  ADD %s, %s, x0\n", intReg->name, decUpperReg->name);

    printf("  SLT %s, %s, x0\n", temp_reg1->name, decUpperReg->name); //temp reg 1 has 
    
    printf("  SLTUI %s, %s, 1\n", temp_reg2->name, decLowerReg->name); // temp 2 = dec_lower > 0
    printf("  AND %s, %s, %s\n", temp_reg1->name, temp_reg1->name, temp_reg2->name);

    printf("  ADD %s, %s, %s\n", intReg->name, intReg->name, temp_reg1->name);

    free_register(gen, temp_reg1);
    free_register(gen, temp_reg2);

}


void initializedDeclarationNode(CodeGenerator * gen, AST_Node * node){

    //need to handle nonrecursive and recursive chains.
    int numChildren = node->num_children;
    Symbol typeSymbol = node->children[numChildren - 3]->symbol;
    DataType type = dataTypeFromTypeSymbol(typeSymbol);
    node->resultType = type;
    AST_Node * valueNode = node->children[numChildren - 1];
    generateCode(gen, valueNode);

    // get register that holds value / result
    Register * doNotSpill[] = {NULL, NULL, NULL, NULL};
    Register * resultUpper, * resultLower, * valueUpper, * valueLower;


    if(type == TYPE_INTEGER){
        
        resultLower = allocate_register(gen, node, 0, doNotSpill, 0, -1);
        doNotSpill[0] = resultLower;
        
        valueLower = get_register(gen, valueNode, 0, doNotSpill, 1);
        printf("  ADD %s, %s, x0\n", resultLower->name, valueLower->name);
        free_register(gen, valueLower);

    }


    if (type == TYPE_DECIMAL){

        resultUpper = allocate_register(gen, node, 0, doNotSpill, 0, -1);
        doNotSpill[0] = resultUpper;
        resultLower = allocate_register(gen ,node, 1, doNotSpill, 1, -1);
        doNotSpill[1] = resultLower;

        valueUpper = get_register(gen, valueNode, 0, doNotSpill, 2);
        doNotSpill[2] = valueUpper;
        valueLower = get_register(gen, valueNode, 1, doNotSpill, 3);

        printf("  ADD %s, %s, x0\n", resultUpper->name, valueUpper->name);
        printf("  ADD %s, %s, x0\n", resultLower->name, valueLower->name);

        free_register(gen, valueLower);
        free_register(gen, valueUpper);
    }

    //for each variable, make an entry in the table and store it in memory
    
    int numIdentifiers = 1 + ((numChildren - 5) / 2);

    for(int i = 0; i < numIdentifiers; i++){
        
        int identifierIndex = i * 2;
        Symbol identifierSym = node->children[identifierIndex]->symbol;
        char * word = (char *)identifierSym.value;  
        addNewVariable(gen, identifierSym, type);
        //incrememnt stack pointer, store variable, then add entry

        if(type == TYPE_INTEGER){

            printf("  SW %s, 0(sp)\n", resultLower->name);

        }


        if(type == TYPE_DECIMAL){

            printf("  SW %s, 0(sp)\n", resultUpper->name);
            printf("  SW %s, 4(sp)\n", resultLower->name);

        }
        
    }


}


void declarationNode(CodeGenerator * gen, AST_Node * node){
   
    //need to handle unrecursive chained uninit. declarations
    int numChildren = node->num_children;
    
    Symbol  typeSymbol = node->children[numChildren - 1]->symbol;
    DataType type = dataTypeFromTypeSymbol(typeSymbol);

    for(int i = 0; i < numChildren - 2; i+=2){
        Symbol identifierSym = node->children[i]->symbol;
        addNewVariable(gen, identifierSym, type);

    }

}


void functionDeclarationNode(CodeGenerator * gen, AST_Node * node){
    
    char * key = functionDefinitionToKey(node);
    FunctionArguments functionArgs = getFunctionArguments(node);
    printf("got function args. num args = %d\n", functionArgs.numArgs);

    

    char * label = addLabel(gen, key);
    FunctionTableEntry * entry = get(gen->functionTable, key);
    entry->label = label;

    char * endLabel;
    asprintf(&endLabel, "%s_end", label);
    endLabel = addLabel(gen, endLabel);

    printf("JAL x0, %s\n", endLabel);
    printf("%s:\n", label);



    enterScope(gen);

    

    //function arguments should be in correct position
    //args are in a0 - a6, additional args are on stack

    //push args onto stack
    int remainingRegSpace = 7 * 4; //space left in regs
    int regNum = 0;
    for(int i = 0; i < functionArgs.numArgs; i++){
        DataType argType = functionArgs.argTypes[i];
        char * argName = functionArgs.argNames[i];
        Symbol identifierSym = IdentifierSymbol;
        identifierSym.value = argName;


        int argWords = getSize(argType) / 4;

        addNewVariable(gen, identifierSym, argType);

        for(int j = 0; j < argWords; j++){

            //see if argument is in reg or on the stack
            if(regNum <= 6){
                //data is in reg
                printf("  SW a%d, %d(sp)\n", regNum, j * 4);
                regNum++;
                
            }

            else{
                //data is in the stack
                int offset = (regNum - 6) * 4;
                printf("  LW a0, %d(fp)\n", offset);
                printf("  SW a0, %d(sp)\n", j * 4);
                regNum++;
            }

            
        }


    }



    /////////////Now process function body
    AST_Node * bodyNode = node->children[5];
    generateCode(gen, bodyNode);

    leaveScope(gen);

    printf("%s:\n", endLabel);
}

void returnNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * valueNode = node->children[2];
    DataType type = valueNode->resultType;

    Register * valueLower, * valueUpper;
    Register * doNotSpill[] = {NULL,NULL, NULL, NULL};
    
    generateCode(gen, valueNode);

    if(type == TYPE_INTEGER){

        valueLower = get_register(gen, valueNode, 0, doNotSpill, 0);
        printf("  ADD a0, %s, x0\n", valueLower->name);
        free_register(gen, valueLower);
        
    }

    if(type == TYPE_DECIMAL){

        valueLower = get_register(gen, valueNode, 1, doNotSpill, 0);
        doNotSpill[0] = valueLower;
        valueUpper = get_register(gen, valueNode, 0, doNotSpill, 1);

        printf("  ADD a0, %s, x0\n", valueUpper->name);
        printf("  ADD a1, %s, x0\n", valueLower->name);

        free_register(gen, valueUpper);
        free_register(gen, valueLower);
        
    }

    leaveScopeReturn(gen);
    printf("JALR x0, ra, 0\n");
    
}

void functionCallNode(CodeGenerator * gen, AST_Node * node){
    char * key = functionCallToKey(node);
    FunctionTableEntry * entry = get(gen->functionTable, key);
    
    FunctionArguments functionArgs = getFunctionArguments(entry->declarationNode);



    //must load function args into correct locations
    int numChildren = node->num_children;
    AST_Node * functionCallArgs = (AST_Node *)malloc(sizeof(AST_Node) * numChildren);
    int argIndex = 0;
    for(int i = 0; i < numChildren; i++){
        AST_Node * childNode = node->children[i];
        Symbol sym = childNode->symbol;
        if(compareSymbols(&sym, &FunctionCallArgumentSymbol)) {
            functionCallArgs[argIndex++] = childNode->children[0];
            generateCode(gen, childNode->children[0]);
                                                            }
    }


    Register * tempRegUpper, * tempRegLower;
    Register * doNotSpill[] = {NULL, NULL};

    tempRegUpper = allocate_register(gen, NULL, 0, doNotSpill, 0, -1);
    doNotSpill[0] = tempRegUpper;
    tempRegLower = allocate_register(gen, NULL, 0, doNotSpill, 1, -1);
    free_register(gen, tempRegUpper);
    free_register(gen, tempRegLower);


    //store a reg's to stack before overwriting them
    

    int numWords = functionArgs.totalSize / 4;
    int stackIncrementation = numWords;
    if(stackIncrementation > 7)stackIncrementation = 7;
    printf("  ADDI sp, sp, %d\n", (stackIncrementation + 1)* -4);
    printf("  SW ra, 0(sp)\n");
    for(int i = 0; i < numWords && i < 7; i++){
        printf("  SW a%d %d(sp)", i, (i + 1)*4);
    }


    argIndex = functionArgs.numArgs - 1;
    int wordIndex = 0;
    int stackWords = 0;
    for(int i = 0; i < numWords - 7; i++){
        //add extra arguments to stack in reverse order first
        DataType type = functionArgs.argTypes[argIndex];
        int argWords = getSize(type) / 4;
        int reversedWordIndex = (argWords - 1) - wordIndex;
        Register * reg = get_register(gen, functionCallArgs[argIndex], reversedWordIndex, NULL, 0);
        
        incrementStackPointer(gen, -4);
        printf("  SW %s, 0(sp)\n", reg->name);
        stackWords++;

        wordIndex++

        if(reversedWordIndex == 0){
            wordIndex = 0; 
            argIndex--;
        }

        free_register(gen, reg);
    }


   argIndex = 0;
   wordIndex = 0;
    for(int i = 0; i < 7 && argIndex < functionArgs.numArgs; i++){
        //store 
        DataType type = functionArgs.argTypes[argIndex];
        int argWords = getSize(type) / 4;
        Register * reg = get_register(gen, functionCallArgs[argIndex], wordIndex, NULL, 0);

        printf("  ADD a%d, %s, x0\n", i, reg->name);
        wordIndex++;

        if(wordIndex >= argWords){
            wordIndex = 0;
            argIndex++;
        }

        free_register(gen, reg);

    }









    printf("  LW ra, 0(sp)\n");
    for(int i = 0; i < numWords && i < 7; i++){
        printf("  LW a%d %d(sp)", i, (i + 1)*4);
    }
    printf("  ADDI sp, sp, %d\n", (stackIncrementation + 1) -4);

    printf("  JALR x0, ra, 0\n");


}


void identifierNode(CodeGenerator * gen, AST_Node * node){
    
    char * word = node->symbol.value;
    Register * doNotSpill[] = {NULL, NULL, NULL};

    int totalOffset = 0;

    int i = 0;

    while(true){

        VariableTable * table = peekAtIndex(gen->variableTableStack, i);
        VariableTableEntry * entry = get(table->variables, word);

        if(entry != NULL){
            DataType type = entry->type;

            int offset = entry->memoryLocation;
            Register * resultUpper, * resultLower;

            if(type == TYPE_INTEGER){
                resultLower = allocate_register(gen, node, 0, doNotSpill, 0, -1);
                printf("  LW %s, %d(fp)\n", resultLower->name, totalOffset + offset);
            }

            if(type == TYPE_DECIMAL){
                resultUpper = allocate_register(gen, node, 0, doNotSpill, 0, -1);
                doNotSpill[0] = resultUpper;
                resultLower = allocate_register(gen, node, 1, doNotSpill, 1, -1);
                
                printf("  LW %s, %d(fp)\n", resultUpper->name, totalOffset + offset);
                offset +=4;
                printf("  LW %s, %d(fp)\n", resultLower->name, totalOffset + offset);

            }

            return;
            
        }

        totalOffset += table->frameSize;

    }

}

void stringNode(CodeGenerator * gen, AST_Node * node){
    
    Symbol sym = node->symbol;
    
    //value is a pointer to the address
    int * stringAddress = get(gen->stringTable,sym.value);
    
    Register * reg = allocate_register(gen, node, 0, NULL, 0, -1);
    load_int(gen, *stringAddress, reg);

    
}

void printStatementNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * valueNode = node->children[1];

    generateCode(gen, valueNode);

    Register * stringReg = get_register(gen, valueNode, 0, NULL, 0);
    
    char * RegIn[] = {stringReg->name};
    callFunction(gen, "Print", RegIn, 1, NULL, 0);

    free_register(stringReg);
}


void orExpressionNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * leftChild = node->children[0];
    AST_Node * rightChild = node->children[2];

    generateCode(gen, leftChild);
    generateCode(gen, rightChild);

    Register * doNotSpill[] ={NULL, NULL, NULL};
    
    Register * leftReg = get_register(gen, leftChild, 0, doNotSpill, 0);
    doNotSpill[0] = leftReg;
    Register * rightReg = get_register(gen, rightChild, 0, doNotSpill, 1);
    doNotSpill[1] = rightReg;
    Register * resultReg = allocate_register(gen, node, 0, doNotSpill, 2, -1);

    char * regIn[] = {leftReg->name, rightReg->name};
    char * regOut[] = {resultReg->name};
    callFunction(gen, "OrFunction", regIn, 2 ,regOut, 1);

    free_register(gen, leftReg);
    free_register(gen, rightReg);

}


void andExpressionNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * leftChild = node->children[0];
    AST_Node * rightChild = node->children[2];

    generateCode(gen, leftChild);
    generateCode(gen, rightChild);

    Register * doNotSpill[] ={NULL, NULL, NULL};
    
    Register * leftReg = get_register(gen, leftChild, 0, doNotSpill, 0);
    doNotSpill[0] = leftReg;
    Register * rightReg = get_register(gen, rightChild, 0, doNotSpill, 1);
    doNotSpill[1] = rightReg;
    Register * resultReg = allocate_register(gen, node, 0, doNotSpill, 2, -1);

    char * regIn[] = {leftReg->name, rightReg->name};
    char * regOut[] = {resultReg->name};
    callFunction(gen, "AndFunction", regIn, 2 ,regOut, 1);

    free_register(gen, leftReg);
    free_register(gen, rightReg);

}

void notExpressionNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * leftChild = node->children[0];
    AST_Node * rightChild = node->children[2];

    generateCode(gen, leftChild);
    generateCode(gen, rightChild);

    Register * doNotSpill[] ={NULL, NULL, NULL};
    
    Register * leftReg = get_register(gen, leftChild, 0, doNotSpill, 0);
    doNotSpill[0] = leftReg;
    Register * resultReg = allocate_register(gen, node, 0, doNotSpill, 1, -1);

    char * regIn[] = {leftReg->name};
    char * regOut[] = {resultReg->name};
    callFunction(gen, "NotFunction", regIn, 1 ,regOut, 1);

    free_register(gen, leftReg);

}





void ifStatementNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * ifNode = node->children[1];
    AST_Node * thenNode = node->children[3];

    generateCode(gen, ifNode);

    Register * doNotSpill[] = {NULL, NULL, NULL};

    Register * ifReg = get_register(gen, ifNode, 0, doNotSpill, 0);
    doNotSpill[0] = ifReg;
    Register * tempReg = get_register(gen, NULL, 0, doNotSpill, 1);

    
    int labelNum = gen->numLabels++;

    printf("  BEQ %s, x0, ConditionFalse%d\n", ifReg->name, labelNum);
    

    enterScope(gen);


    generateCode(gen, thenNode);

    leaveScope(gen);

    printf("ConditionFalse%d:\n", labelNum);


}

void ifElseStatementNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * ifNode = node->children[1];
    AST_Node * thenNode = node->children[3];
    AST_Node * elseNode = node->children[5];

    generateCode(gen, ifNode);

    Register * doNotSpill[] = {NULL, NULL, NULL};

    Register * ifReg = get_register(gen, ifNode, 0, doNotSpill, 0);
    doNotSpill[0] = ifReg;
    Register * tempReg = get_register(gen, NULL, 0, doNotSpill, 1);

    
    int labelNum = gen->numLabels++;

    printf("  BEQ %s, x0, ConditionFalse%d\n", ifReg->name, labelNum);
    

    enterScope(gen);


    generateCode(gen, thenNode);

    leaveScope(gen);

    printf("ConditionFalse%d:\n", labelNum);

    enterScope(gen);

    generateCode(gen, elseNode);

    leaveScope(gen);


}



