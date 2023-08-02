#include "Assembly.h"
#include "CFG.h"
#include "ASTWalker.h"
#include "Parser.h"

#include "AST.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


int getSize(DataType type){
    if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN)return 4;
    if(type == TYPE_DECIMAL)return 8;
}


char *  addLabel(CodeGenerator *generator, char *label) {
    printf("in add label,  = %s, numlabels = %d\n ", label, generator->numLabels);
    // Make a copy of the label so we can modify it if necessary
    char *newLabel = strdup(label);
    int labelLen = strlen(newLabel);
    

    printf(" add label 1\n");
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
    printf("add label 3\n\n");

    // Now we know newLabel is unique, so add it to the labels array
    generator->labels = (char**)realloc(generator->labels, (generator->numLabels + 1) * sizeof(char*));
    generator->labels[generator->numLabels] = newLabel; // Copy the new label string
    generator->numLabels++;
    return newLabel;
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
    return args;
}




Register registerS0 = {.in_use = true, .name = "s0", .node = NULL, .wordIndex = 0, .value = 0};
Register registerX0 = (Register){.in_use = false, .name = "x0", .node = NULL, .wordIndex = 0, .value = 0};
Register registerSP = {.in_use = true, .name = "sp", .node = NULL, .wordIndex = 0, .value = 0};


void addVariableToCurrentScopeGen(CodeGenerator * gen, char * key, VariableTableEntry * entry){
    Map * curScope = (Map *)peek(gen->variableTableStack);
    
    if(get(curScope, key) != NULL)throwError(1);
    

    insert(curScope, key, entry);

}


void load_int(CodeGenerator * gen, int value, Register * reg){

    char * reg_name  = reg->name;
    reg->value = value;

    if (value >= -2048 && value <= 2047) {
        fprintf(stdoutFP,"  addi %s, x0, %d\n", reg_name, value);
    } else {
        uint32_t upper_imm = value >> 12;
        int lower_imm = value & 0xfff;
        fprintf(stdoutFP,"  lui %s, %u\n", reg_name, upper_imm);
        fprintf(stdoutFP,"  addi %s, %s, %d\n", reg_name, reg_name, lower_imm);
    }
}

void load_unsigned_int(CodeGenerator * gen, uint32_t value, Register * reg) {
    char * reg_name = reg->name;
    reg->value = value;

    uint32_t upper_imm = value >> 12;
    int32_t lower_imm = value & 0xFFF;

    if (lower_imm > 2047) {
        // If the lower 12 bits are larger than 2047, add 1 to the upper bits
        // and subtract 4096 from the lower bits to ensure they are in the correct range
        upper_imm += 1;
        lower_imm -= 4096;
    }

    fprintf(stdoutFP, "  lui %s, %u\n", reg_name, upper_imm);
    if (lower_imm != 0) {
        fprintf(stdoutFP, "  addi %s, %s, %d\n", reg_name, reg_name, lower_imm);
    }
}


void load_int2(CodeGenerator * gen, int value, Register * reg){
    char * reg_name  = reg->name;
    reg->value = value;


    fprintf(stdoutFP,"  li %s, %d\n", reg_name, value);

}



void setFramePointer(CodeGenerator * gen, int address){
    load_int(gen, address, &registerS0);
    gen->framePointer = address;
}

void incrementStackPointer(CodeGenerator * gen, int amt){
    gen->stackPointer += amt;
    fprintf(stdoutFP,"  ADDI sp, sp, %d\n", amt);
    gen->stack_offset -= amt;



    VariableTable * table = peek(gen->variableTableStack);
    table->frameSize -= amt;

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
            fprintf(stdoutFP,"  sw %s, 0(sp)\n", reg->name); // Store register value

            
            VariableTable * table = peek(gen->variableTableStack);
            //table->frameSize += 4;

            // Update location of the value formerly in the register
            ValueLocation location = {.type = LOCATION_MEMORY, .memory_offset = table->frameSize, .value = reg->value};
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
    printf("about to load a register for node id = %d,  locataion offset = %d\n", node->id, location.memory_offset);
    fprintf(stdoutFP,"  lw %s, %d(fp)\n", reg->name, -location.memory_offset); // Load word

    // The value is now in the register
    ValueLocation new_location = {.type = LOCATION_REGISTER, .register_name = reg->name};
   // set_value_location(gen, node, new_location);
    update_value_location(gen, node, new_location, wordIndex);

    return reg;
}

//make more efficient with temp regs
void callFunction(CodeGenerator * gen, char * funcName, char ** regNamesIn, int numArgs, char ** regNamesOut, int numOut){
    int numTemp = 7;
    
    incrementStackPointer(gen, (numArgs + numTemp + 1)  * -4);


    // Save the argument registers and ra on the stack
    for(int i = 0; i < numArgs; i++){
        fprintf(stdoutFP,"  SW a%d, %d(sp)\n", i, i*4);
    }

    for(int i = 0; i < numTemp; i++){
        fprintf(stdoutFP,"  SW t%d, %d(sp)\n", i, (numArgs  + i)*4);
    }
    fprintf(stdoutFP,"  SW ra, %d(sp)\n", (numArgs +numTemp)*4);

    // Move the specified registers to the argument registers
    for(int i = 0; i < numArgs; i++){
        fprintf(stdoutFP,"  ADD a%d, %s, zero\n", i, regNamesIn[i]);
    }

    fprintf(stdoutFP,"  JAL ra, %s\n", funcName);  // call the function

    //load temp reg back
    for(int i = 0; i < numTemp; i++){
        fprintf(stdoutFP,"  LW t%d, %d(sp)\n", i, (numArgs + i)*4);
    }

    // Move the result from the argument registers to the specified registers
    for(int i = 0; i < numOut; i++){
        fprintf(stdoutFP,"  ADD %s, a%d, zero\n", regNamesOut[i], i);
    }

    // Restore the argument registers and ra from the stack
    for(int i = 0; i < numArgs; i++){
        fprintf(stdoutFP,"  LW a%d, %d(sp)\n", i, i*4);
    }


    fprintf(stdoutFP,"  LW ra, %d(sp)\n", (numArgs + numTemp)*4);

    incrementStackPointer(gen, (numArgs + numTemp + 1)  * 4);

}

void generate2WordAdditionFunction(CodeGenerator * gen){
    //works for 2's comp and unsigned
    int labelNum = gen->next_label++;
    //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
    //output: a0 = result_upper , a1 = result_lower
    fprintf(stdoutFP,"Addition_2Word:\n");

    //step 1:  Add lower bits
    fprintf(stdoutFP,"  ADD t0, a1, a3\n");

    //step 2: Check for Overflow (if res < either of the inputs)
    fprintf(stdoutFP,"  BLTU t0, a1, overflow_detected%d\n",labelNum);
    fprintf(stdoutFP,"  BLTU t0, a3, overflow_detected%d\n",labelNum);
    fprintf(stdoutFP,"  JAL x0, no_overflow_detected%d\n",labelNum);

    //handle overflow by adding 1 to the upper part of an operand
    fprintf(stdoutFP,"overflow_detected%d:\n",labelNum);
    fprintf(stdoutFP,"  ADDI a0, a0, 1\n");

    //add the upper part, and set the correct registers before returning
    fprintf(stdoutFP,"no_overflow_detected%d:\n",labelNum);
    fprintf(stdoutFP,"  ADD a0, a0, a2\n");
    fprintf(stdoutFP,"  ADD a1, x0, t0\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");

}

void generate2WordSubtractionFunction(CodeGenerator * gen){
    //works for 2's comp and unsigned
    int labelNum = gen->next_label++;
    //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
    //output: a0 = result_upper , a1 = result_lower
    fprintf(stdoutFP,"Subtraction_2Word:\n");

    //step 1:  Subtract lower bits
    fprintf(stdoutFP,"  SUB t0, a1, a3\n");

    //step 2: Check for Underflow (if result > a1)
    fprintf(stdoutFP,"  BGEU a1, t0, no_underflow_detected%d\n",labelNum);
    fprintf(stdoutFP,"  JAL x0, underflow_detected%d\n",labelNum);

    //handle underflow by subtracting 1 from the upper part of the minuend
    fprintf(stdoutFP,"underflow_detected%d:\n",labelNum);
    fprintf(stdoutFP,"  ADDI a0, a0, -1\n");

    //subtract the upper part, and set the correct registers before returning
    fprintf(stdoutFP,"no_underflow_detected%d:\n",labelNum);
    fprintf(stdoutFP,"  SUB a0, a0, a2\n");
    fprintf(stdoutFP,"  ADD a1, x0, t0\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
}

void generateSigned1WordMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left, a1 = right
    //output: a0 = result_upper, a1 = result_lower

    int labelNum = gen->next_label++;
    fprintf(stdoutFP,"Multiplication_Signed_1Word:\n");

    //make copies of inputs
    fprintf(stdoutFP,"  ADD t0, a0, x0\n");
    fprintf(stdoutFP,"  ADD t1, a1, x0\n");

    fprintf(stdoutFP,"  BGE t0, x0, left_skip_next%d\n", labelNum);
    //if left is negative flip its bits and add one to get unsigned value
    fprintf(stdoutFP,"  XORI t0, t0, -1\n");
    fprintf(stdoutFP,"  ADDI t0, t0, 1\n");
    fprintf(stdoutFP,"left_skip_next%d:\n", labelNum);

    fprintf(stdoutFP,"  BGE t1, x0, right_skip_next%d\n", labelNum);
    //if right is positive flip bits and add one
    fprintf(stdoutFP,"  XORI t1, t1, -1\n");
    fprintf(stdoutFP,"  ADDI t1, t1, 1\n");
    fprintf(stdoutFP,"right_skip_next%d:\n", labelNum);


    //multiply positive nums and store result in t0(upper) and t1(lower)
    char * regIn[] = {"t0", "t1"};
    char * regOut[] = {"t0", "t1"};
    callFunction(gen, "Multiplication_Unsigned_1Word", regIn, 2, regOut, 2);


    //if the original input signs are different, then negate value
    fprintf(stdoutFP,"  XOR t2, a0, a1\n");
    fprintf(stdoutFP,"  BGE t2, x0, different_signs_skip_next%d\n", labelNum);

    //invert bits and add 1 to negate
    fprintf(stdoutFP,"  XORI t0, t0, -1\n");
    fprintf(stdoutFP,"  XORI t1, t1, -1\n");


    //add one
    fprintf(stdoutFP,"  ADDI t2, x0, 1\n");
    char * regIn2[] = {"t0", "t1", "x0", "t2"};
    char * regOut2[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn2, 4, regOut2, 2);



    fprintf(stdoutFP," different_signs_skip_next%d:\n", labelNum);
    //set output registers to result
    fprintf(stdoutFP,"  ADD a0, t0, x0\n");
    fprintf(stdoutFP,"  ADD a1, t1, x0\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
}

void generateUnsigned1WordMultiplicationFunction(CodeGenerator * gen){
   int labelNum = gen->next_label++;
   
   
    //input: a0 = left, a1 = right
    //output: a0 = result_upper, a1 = result_lower
    //temp registers: s0 = left upper, s1 = left lower, s2 = right upper, s3 = right lower, s4 = result upper, s5 = result lower
    fprintf(stdoutFP,"Multiplication_Unsigned_1Word:\n");

    //save contents of S registers
    fprintf(stdoutFP,"  ADDI sp, sp, -24\n");
    fprintf(stdoutFP,"  SW s0, 0(sp)\n");
    fprintf(stdoutFP,"  SW s1, 4(sp)\n");
    fprintf(stdoutFP,"  SW s2, 8(sp)\n");
    fprintf(stdoutFP,"  SW s3, 12(sp)\n");
    fprintf(stdoutFP,"  SW s4, 16(sp)\n");
    fprintf(stdoutFP,"  SW s5, 20(sp)\n");


    //Initialize the contents of the S registers
    fprintf(stdoutFP,"  ADD s0, x0, x0\n");
    fprintf(stdoutFP,"  ADD s1, a0, x0\n");

    fprintf(stdoutFP,"  ADD s2, x0, x0\n");
    fprintf(stdoutFP,"  ADD s3, a1, x0\n");

    fprintf(stdoutFP,"  ADD s4, x0, x0\n");
    fprintf(stdoutFP,"  ADD s5, x0, x0\n");


    //Main Loop
    fprintf(stdoutFP,"Multiplication_Unsigned_1Word_loop:\n");

    //if the right reg is zero, end the loop
    fprintf(stdoutFP,"  BEQ s3, x0, Multiplication_Unsigned_1Word_end\n");

    //else check the LSB of if the right reg
    fprintf(stdoutFP,"  ANDI t0, s3, 1\n");
    //if its zero, then skip the addition part
    fprintf(stdoutFP,"  BEQ t0, x0, Multiplication_Unsigned_1Word_loop_end\n");
    

    char * regIn[] = {"s0", "s1", "s4", "s5"};
    char * regOut[] = {"s4", "s5"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);




    //after the addition is over, we shift the left and right regs
    fprintf(stdoutFP,"Multiplication_Unsigned_1Word_loop_end:\n");
    fprintf(stdoutFP,"  SRLI s3, s3, 1\n");

        //shifting left reg is challenging because we need to account for overflow. 
        //check if most significant bit is 1
   
    fprintf(stdoutFP,"  SLLI s0, s0, 1\n"); //shift upper left 1 to the left
    fprintf(stdoutFP,"  BGE s1, x0, skip_next%d\n", labelNum);
    fprintf(stdoutFP," ADDI s0, s0, 1\n");
    fprintf(stdoutFP,"skip_next%d:\n", labelNum);
    fprintf(stdoutFP,"  SLLI s1, s1, 1\n");

    //Now the loop is over, and we can do it again
    fprintf(stdoutFP,"  JAL x0, Multiplication_Unsigned_1Word_loop\n");




    //when loop is over, need to reset regs and stack
    fprintf(stdoutFP,"Multiplication_Unsigned_1Word_end:\n");

    fprintf(stdoutFP,"  ADD a0, s4, x0\n");
    fprintf(stdoutFP,"  ADD a1, s5, x0\n");


    //return contents of S registers
    fprintf(stdoutFP,"  LW s0, 0(sp)\n");
    fprintf(stdoutFP,"  LW s1, 4(sp)\n");
    fprintf(stdoutFP,"  LW s2, 8(sp)\n");
    fprintf(stdoutFP,"  LW s3, 12(sp)\n");
    fprintf(stdoutFP,"  LW s4, 16(sp)\n");
    fprintf(stdoutFP,"  LW s5, 20(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 24\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
    
}

void generateSignedFixedPointMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left upper, a1 = left lower, a2 = right upper, a3 = right lower
    //output: a0 = result upper, a1 = result lower
    
    int labelNum = gen->next_label++;
    fprintf(stdoutFP,"Multiplication_Signed_FixedPoint:\n");

    //make copies of the inputs
    fprintf(stdoutFP,"  ADD t0, a0, x0\n");
    fprintf(stdoutFP,"  ADD t1, a1, x0\n");
    fprintf(stdoutFP,"  ADD t2, a2, x0\n");
    fprintf(stdoutFP,"  ADD t3, a3, x0\n");

    //if left is negative, negate
    fprintf(stdoutFP,"  BGE t0, x0, left_skip_next%d\n", labelNum);
    fprintf(stdoutFP,"  XORI t0, t0, -1\n");
    fprintf(stdoutFP,"  XORI t1, t1, -1\n");

    fprintf(stdoutFP,"  ADDI t5, x0, 1\n");
    char * regIn[] = {"t0", "t1", "x0", "t5"};
    char * regOut[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);
    
    fprintf(stdoutFP,"left_skip_next%d:\n", labelNum);


    //if right is negative, negate
    fprintf(stdoutFP,"  BGE t2, x0, right_skip_next%d\n", labelNum);
    fprintf(stdoutFP,"  XORI t2, t0, -1\n");
    fprintf(stdoutFP,"  XORI t3, t1, -1\n");

    fprintf(stdoutFP,"  ADDI t5, x0, 1\n");
    char * regIn2[] = {"t2", "t3", "x0", "t5"};
    char * regOut2[] = {"t2", "t3"};
    callFunction(gen, "Addition_2Word", regIn2, 4, regOut2, 2);
    
    fprintf(stdoutFP,"right_skip_next%d:\n", labelNum);


    //multiply unsigned values together
    char * regIn3[] = {"t0", "t1", "t2", "t3"};
    char * regOut3[] = {"t0", "t1"};
    callFunction(gen, "Multiplication_Unsigned_FixedPoint", regIn3, 4, regOut3, 2);


    //if input signs are different then negate answer
    fprintf(stdoutFP,"  XOR t3, a0, a2\n");
    fprintf(stdoutFP,"  BGE t3, x0, different_signs_skip_next%d\n", labelNum);
    
    fprintf(stdoutFP,"  XORI t0, t0, -1\n");
    fprintf(stdoutFP,"  XORI t1, t1, -1\n");
    fprintf(stdoutFP,"  ADDI t3, x0, 1\n");
    char * regIn4[] = {"t0", "t1", "x0", "t3"};
    char * regOut4[] = {"t0", "t1"};
    callFunction(gen, "Addition_2Word", regIn4, 4, regOut4, 2);

    fprintf(stdoutFP,"different_signs_skip_next%d:\n", labelNum);

    //set outputs and return
    fprintf(stdoutFP,"  ADD a0, t0, x0\n");
    fprintf(stdoutFP,"  ADD a1, t1, x0\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
}

void generateUnsignedFixedPointMultiplicationFunction(CodeGenerator * gen){
    //input: a0 = left upper, a1 = left lower, a2 = right upper, a3 = right lower
    //output: a0 = result upper, a1 = result lower

    //temp: s0 = result upper, s1 = result lower
    //      s2 = Term 1 upper, s3 = Term 1 lower
    //      s4 = Term 2 upper, s5 = Term 2 lower
    //      s6 = Term 3 upper, s7 = Term 3 lower
    //      s8 = Term 4 upper, s9 = Term 4 lower

    fprintf(stdoutFP,"Multiplication_Unsigned_FixedPoint:\n");

    //save s regs to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -40\n");
    fprintf(stdoutFP,"  SW s0, 0(sp)\n");
    fprintf(stdoutFP,"  SW s1, 4(sp)\n");
    fprintf(stdoutFP,"  SW s2, 8(sp)\n");
    fprintf(stdoutFP,"  SW s3, 12(sp)\n");
    fprintf(stdoutFP,"  SW s4, 16(sp)\n");
    fprintf(stdoutFP,"  SW s5, 20(sp)\n");
    fprintf(stdoutFP,"  SW s6, 24(sp)\n");
    fprintf(stdoutFP,"  SW s7, 28(sp)\n");
    fprintf(stdoutFP,"  SW s8, 32(sp)\n");
    fprintf(stdoutFP,"  SW s9, 36(sp)\n");   


    //initialize s regs
    fprintf(stdoutFP,"  ADD s0, x0, x0\n");
    fprintf(stdoutFP,"  ADD s1, x0, x0\n");

    //Term 1 (whole) = lower res of (left upper * right upper)
    //Term 2 (fractional) = lower res of (left upper * right_lower)
    //Term 3 (fractional) = lower res of (left lower * right_upper)
    //Term 4 (fractional) = upper res of (left lower * right lower)

    //////////////////////////////////////////
    //Get term 1
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -12\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW ra, 8(sp)\n");

    //load args
    fprintf(stdoutFP,"  ADD a0, a0, x0\n");
    fprintf(stdoutFP,"  ADD a1, a2, x0\n");
    fprintf(stdoutFP,"  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    fprintf(stdoutFP,"  ADD s2, a0, x0\n");
    fprintf(stdoutFP,"  ADD s3, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW ra, 8(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 2
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -12\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW ra, 8(sp)\n");

    //load args
    fprintf(stdoutFP,"  ADD a0, a0, x0\n");
    fprintf(stdoutFP,"  ADD a1, a3, x0\n");
    fprintf(stdoutFP,"  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    fprintf(stdoutFP,"  ADD s4, a0, x0\n");
    fprintf(stdoutFP,"  ADD s5, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW ra, 8(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 3
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -12\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW ra, 8(sp)\n");

    //load args
    fprintf(stdoutFP,"  ADD a0, a1, x0\n");
    fprintf(stdoutFP,"  ADD a1, a2, x0\n");
    fprintf(stdoutFP,"  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    fprintf(stdoutFP,"  ADD s6, a0, x0\n");
    fprintf(stdoutFP,"  ADD s7, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW ra, 8(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 12\n");


    //////////////////////////////////////////
    //Get term 4
    //////////////////////////////////////////
    //call mult function, need to load correct regs
    //save regs to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -12\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW ra, 8(sp)\n");

    //load args
    fprintf(stdoutFP,"  ADD a0, a1, x0\n");
    fprintf(stdoutFP,"  ADD a1, a3, x0\n");
    fprintf(stdoutFP,"  JAL ra, Multiplication_Unsigned_1Word\n");

    //when returns a a0 has upper result, a1 has lower result
    fprintf(stdoutFP,"  ADD s8, a0, x0\n");
    fprintf(stdoutFP,"  ADD s9, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW ra, 8(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 12\n");





    //Now all the temporary results are stored in s2-s9
    //Lower part of term1 is added to upper part of result
    //Lower part of terms 2 and 3 are added to lower part of result
    //Upper part of term4 is added to lower part of result
    
    //The add two word function has input and output:
        //input:  a0 = left_upper, a1 = left_lower, a2 = right_upper, a3 = right_lower
        //output: a0 = result_upper , a1 = result_lower

        //Add term 1 to the res
    fprintf(stdoutFP,"  ADDI sp, sp, -20\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW a2, 8(sp)\n");
    fprintf(stdoutFP,"  SW a3, 12(sp)\n");
    fprintf(stdoutFP,"  SW ra, 16(sp)\n");

    fprintf(stdoutFP,"  ADD a0, s0, x0\n"); //left is current result
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");
    fprintf(stdoutFP,"  ADD a2, s3, x0\n"); //right upper is term 1 lower
    fprintf(stdoutFP,"  ADD a3, x0, x0\n"); // right lower is zero

    fprintf(stdoutFP,"  JAL ra, Addition_2Word\n");
    fprintf(stdoutFP,"  ADD s0, a0, x0\n");
    fprintf(stdoutFP,"  ADD s1, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW a2, 8(sp)\n");
    fprintf(stdoutFP,"  LW a3, 12(sp)\n");
    fprintf(stdoutFP,"  LW ra, 16(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 20\n");

        //Add term 2 to the res
    fprintf(stdoutFP,"  ADDI sp, sp, -20\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW a2, 8(sp)\n");
    fprintf(stdoutFP,"  SW a3, 12(sp)\n");
    fprintf(stdoutFP,"  SW ra, 16(sp)\n");

    fprintf(stdoutFP,"  ADD a0, s0, x0\n"); //left is current result
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");
    fprintf(stdoutFP,"  ADD a2, s4, x0\n"); //right upper is term 2 upper
    fprintf(stdoutFP,"  ADD a3, s5, x0\n"); //right lower is term 2 lower

    fprintf(stdoutFP,"  JAL ra, Addition_2Word\n");
    fprintf(stdoutFP,"  ADD s0, a0, x0\n");
    fprintf(stdoutFP,"  ADD s1, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW a2, 8(sp)\n");
    fprintf(stdoutFP,"  LW a3, 12(sp)\n");
    fprintf(stdoutFP,"  LW ra, 16(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 20\n");

        //Add term 3 to the res
    fprintf(stdoutFP,"  ADDI sp, sp, -20\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW a2, 8(sp)\n");
    fprintf(stdoutFP,"  SW a3, 12(sp)\n");
    fprintf(stdoutFP,"  SW ra, 16(sp)\n");

    fprintf(stdoutFP,"  ADD a0, s0, x0\n"); //left is current result
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");
    fprintf(stdoutFP,"  ADD a2, s6, x0\n"); //right upper is term 3 upper
    fprintf(stdoutFP,"  ADD a3, s7, x0\n"); //right lower is term 3 lower

    fprintf(stdoutFP,"  JAL ra, Addition_2Word\n");
    fprintf(stdoutFP,"  ADD s0, a0, x0\n");
    fprintf(stdoutFP,"  ADD s1, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW a2, 8(sp)\n");
    fprintf(stdoutFP,"  LW a3, 12(sp)\n");
    fprintf(stdoutFP,"  LW ra, 16(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 20\n");



        //Add term 4 to the res
    fprintf(stdoutFP,"  ADDI sp, sp, -20\n");
    fprintf(stdoutFP,"  SW a0, 0(sp)\n");
    fprintf(stdoutFP,"  SW a1, 4(sp)\n");
    fprintf(stdoutFP,"  SW a2, 8(sp)\n");
    fprintf(stdoutFP,"  SW a3, 12(sp)\n");
    fprintf(stdoutFP,"  SW ra, 16(sp)\n");

    fprintf(stdoutFP,"  ADD a0, s0, x0\n"); //left is current result
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");
    fprintf(stdoutFP,"  ADD a2, x0, x0\n"); //right upper is zero
    fprintf(stdoutFP,"  ADD a3, s8, x0\n"); //right lower is term 4 upper

    fprintf(stdoutFP,"  JAL ra, Addition_2Word\n");
    fprintf(stdoutFP,"  ADD s0, a0, x0\n");
    fprintf(stdoutFP,"  ADD s1, a1, x0\n");


    fprintf(stdoutFP,"  LW a0, 0(sp)\n");
    fprintf(stdoutFP,"  LW a1, 4(sp)\n");
    fprintf(stdoutFP,"  LW a2, 8(sp)\n");
    fprintf(stdoutFP,"  LW a3, 12(sp)\n");
    fprintf(stdoutFP,"  LW ra, 16(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 20\n");


    //Now our final result is stored in reg s0, and s1.
    fprintf(stdoutFP,"  ADD a0, s0, x0\n");
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");


    fprintf(stdoutFP,"  LW s0, 0(sp)\n");
    fprintf(stdoutFP,"  LW s1, 4(sp)\n");
    fprintf(stdoutFP,"  LW s2, 8(sp)\n");
    fprintf(stdoutFP,"  LW s3, 12(sp)\n");
    fprintf(stdoutFP,"  LW s4, 16(sp)\n");
    fprintf(stdoutFP,"  LW s5, 20(sp)\n");
    fprintf(stdoutFP,"  LW s6, 24(sp)\n");
    fprintf(stdoutFP,"  LW s7, 28(sp)\n");
    fprintf(stdoutFP,"  LW s8, 32(sp)\n");
    fprintf(stdoutFP,"  LW s9, 36(sp)\n");    
    fprintf(stdoutFP,"  ADDI sp, sp, 40\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");

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
    fprintf(stdoutFP,"Division_Unsigned_2Word:\n");

    //Store S registers to stack
    fprintf(stdoutFP,"  ADDI sp, sp, -48\n");
    fprintf(stdoutFP,"  SW s0, 0(sp)\n");
    fprintf(stdoutFP,"  SW s1, 4(sp)\n");
    fprintf(stdoutFP,"  SW s2, 8(sp)\n");
    fprintf(stdoutFP,"  SW s3, 12(sp)\n");
    fprintf(stdoutFP,"  SW s4, 16(sp)\n");
    fprintf(stdoutFP,"  SW s5, 20(sp)\n");
    fprintf(stdoutFP,"  SW s6, 24(sp)\n");
    fprintf(stdoutFP,"  SW s7, 28(sp)\n");
    fprintf(stdoutFP,"  SW s8, 32(sp)\n");
    fprintf(stdoutFP,"  SW s9, 36(sp)\n");
    fprintf(stdoutFP,"  SW s10, 40(sp)\n");
    fprintf(stdoutFP,"  SW s11, 44(sp)\n");


    //Initialize Registers
    fprintf(stdoutFP,"  ADD s0, x0, x0\n");
    fprintf(stdoutFP,"  ADD s1, x0, x0\n");
    fprintf(stdoutFP,"  ADD s2, x0, x0\n");
    fprintf(stdoutFP,"  ADD s3, x0, x0\n");
    fprintf(stdoutFP,"  ADD s4, x0, x0\n");
    fprintf(stdoutFP,"  ADD s5, x0, x0\n");
    fprintf(stdoutFP,"  ADD s6, a0, x0\n");
    fprintf(stdoutFP,"  ADD s7, a1, x0\n");
    fprintf(stdoutFP,"  ADDI s8, x0, 1\n");
    fprintf(stdoutFP,"  ADD s9, x0, x0\n");
    fprintf(stdoutFP,"  ADD s10, a2, x0\n");
    fprintf(stdoutFP,"  ADD s11, a3, x0\n");




    //Until dividend >= divisor, shift dividend left or divisor right
    
    

    fprintf(stdoutFP,"Division_Unsigned_2Word_Shift_Loop:\n");
    fprintf(stdoutFP,"  BLTU s6, s10, Action%d\n", labelNum); // if upper dividend < upper divisor, then shift 
    fprintf(stdoutFP,"  BLTU s10, s6, Division_Unsigned_2Word_Shift_Loop_End\n");// else if upper divisor < upper dividend, dont shift
    fprintf(stdoutFP,"  BGEU s7, s11, Division_Unsigned_2Word_Shift_Loop_End\n");// else compare lower words
    
    fprintf(stdoutFP,"Action%d:\n", labelNum);
    
    //shift dividend to the left
    fprintf(stdoutFP,"   SLT t0, s7, x0\n"); //t0 = MSB of lower dividend
    fprintf(stdoutFP,"  SLLI s6, s6, 1\n");
    fprintf(stdoutFP,"  SLLI s7, s7, 1\n");
    fprintf(stdoutFP,"  ADD s6, s6, t0\n");
    fprintf(stdoutFP,"  ADDI s9, s9, 1\n"); //increment shift count

    fprintf(stdoutFP,"  JAL x0, Division_Unsigned_2Word_Shift_Loop\n");

    fprintf(stdoutFP,"Division_Unsigned_2Word_Shift_Loop_End:\n");

    
    fprintf(stdoutFP,"Division_Unsigned_2Word_Loop:\n");

    //If WD and Dividend Copy = 0,  or Iteration Count = 64 + 1 then break
    fprintf(stdoutFP,"  ADDI t0, x0, 129\n");
    fprintf(stdoutFP,"  BEQ s8, t0, Division_Unsigned_2Word_Loop_End\n");
    fprintf(stdoutFP,"  BNE s4, x0, loop_skip_next%d\n", labelNum);
    fprintf(stdoutFP,"  BNE s5, x0, loop_skip_next%d\n", labelNum);
    fprintf(stdoutFP,"  BNE s6, x0, loop_skip_next%d\n", labelNum);
    fprintf(stdoutFP,"  BEQ s7, x0, Division_Unsigned_2Word_Loop_End\n");


    fprintf(stdoutFP,"loop_skip_next%d:\n", labelNum);

    //WD = (WD << 1) + MSB of Dividend Copy
    fprintf(stdoutFP,"  SLT t0, s6, x0\n"); // t0 = MSB of Divident Copy
    
        //shift WD to left, (need to check MSB of lower word)
    fprintf(stdoutFP,"  SLT t1, s5, x0\n"); 
    fprintf(stdoutFP,"  SLLI s5, s5, 1\n");
    fprintf(stdoutFP,"  SLLI s4, s4, 1\n");
    fprintf(stdoutFP,"  ADD s4, s4, t1\n");

        //Add t0 to WD
    char * regIn[] = {"s4", "s5", "x0", "t0"};
    char * regOut [] = {"s4", "s5"};
    callFunction(gen, "Addition_2Word", regIn, 4, regOut, 2);

    //Dividend Copy = Dividend Copy << 1
    fprintf(stdoutFP,"   SLT t0, s7, x0\n"); //t0 = MSB of Divident Copy Lower
    fprintf(stdoutFP,"  SLLI s6, s6, 1\n");
    fprintf(stdoutFP,"  SLLI s7, s7, 1\n");
    fprintf(stdoutFP,"  ADD s6, s6, t0\n");


    //If WD >= Divisor (right):
    //    WD = WD - Divisor
    //    Quotient at the (Iteration Count) 'th bit (starting from MSB) = 1
    //Else:
    //      Quotient at the (Iteration Count) 'th bit (starting from MSB) = 0



    fprintf(stdoutFP,"  BLTU s4, a2, Division_Unsigned_2Word_Loop_Branch_False\n");
    fprintf(stdoutFP,"  BLTU a2, s4, Division_Unsigned_2Word_Loop_Branch_True\n");

        //if top words are equal compare bottom words
    fprintf(stdoutFP,"  BLTU s5, a3, Division_Unsigned_2Word_Loop_Branch_False\n");
    

    fprintf(stdoutFP,"Division_Unsigned_2Word_Loop_Branch_True:\n");
    //      WD = WD - Divisor

    char * regIn2[] = {"s4", "s5", "a2", "a3"};
    char * regOut2[] = {"s4", "s5"};
    callFunction(gen, "Subtraction_2Word", regIn2, 4, regOut2, 2);

    //      Quotient = Quotient with (Iteration Count)th bit = 1
    fprintf(stdoutFP,"  ADDI t0, x0, 97\n");
    fprintf(stdoutFP,"  BGEU s8, t0, Iteration_GT_96%d\n", labelNum);
    fprintf(stdoutFP,"  ADDI t0, x0, 65\n");
    fprintf(stdoutFP,"  BGEU s8, t0, Iteration_GT_64%d\n", labelNum);
    fprintf(stdoutFP,"  ADDI t0, x0, 33\n");
    fprintf(stdoutFP,"  BGEU s8, t0, Iteration_GT_32%d\n", labelNum);
    fprintf(stdoutFP,"  JAL x0, Iteration_LTE_32%d\n", labelNum);


    fprintf(stdoutFP,"Iteration_GT_96%d:\n", labelNum);

    fprintf(stdoutFP,"  ADDI t1, x0, 128\n");
    fprintf(stdoutFP,"  SUB t1, t1, s8\n"); //t1 = 128 - Iteration Count
    fprintf(stdoutFP,"  ADDI t2, x0, 1\n");
    fprintf(stdoutFP,"  SLL t1, t2, t1\n"); //t1 = 1 << (128 - Iteration Count)
    fprintf(stdoutFP,"  OR s3, s3, t1\n"); //Lower Quotient Bit = 1;
    fprintf(stdoutFP,"  JAL x0, Iteration_Branch_End%d\n", labelNum);

    fprintf(stdoutFP,"Iteration_GT_64%d:\n", labelNum);

    fprintf(stdoutFP,"  ADDI t1, x0, 96\n");
    fprintf(stdoutFP,"  SUB t1, t1, s8\n"); //t1 = 96 - Iteration Count
    fprintf(stdoutFP,"  ADDI t2, x0, 1\n");
    fprintf(stdoutFP,"  SLL t1, t2, t1\n"); //t1 = 1 << (96 - Iteration Count)
    fprintf(stdoutFP,"  OR s2, s2, t1\n"); //Lower Quotient Bit = 1;
    fprintf(stdoutFP,"  JAL x0, Iteration_Branch_End%d\n", labelNum);

    fprintf(stdoutFP,"Iteration_GT_32%d:\n", labelNum);

    fprintf(stdoutFP,"  ADDI t1, x0, 64\n");
    fprintf(stdoutFP,"  SUB t1, t1, s8\n"); //t1 = 64 - Iteration Count
    fprintf(stdoutFP,"  ADDI t2, x0, 1\n");
    fprintf(stdoutFP,"  SLL t1, t2, t1\n"); //t1 = 1 << (64 - Iteration Count)
    fprintf(stdoutFP,"  OR s1, s1, t1\n"); //Upper Quotient Bit = 1;
    fprintf(stdoutFP,"  JAL x0, Iteration_Branch_End%d\n", labelNum);

    fprintf(stdoutFP,"Iteration_LTE_32%d:\n",labelNum);

    fprintf(stdoutFP,"  ADDI t1, x0, 32\n");
    fprintf(stdoutFP,"  SUB t1, t1, s8\n"); //t1 = 32 - Iteration Count
    fprintf(stdoutFP,"  ADDI t2, x0, 1\n");
    fprintf(stdoutFP,"  SLL t1, t2, t0\n"); //t1 = 1 << (32 - Iteration Count)
    fprintf(stdoutFP,"  OR s0, s0, t1\n"); //Upper Upper Bit = 1;
    
    fprintf(stdoutFP,"Iteration_Branch_End%d:\n", labelNum);
    fprintf(stdoutFP,"  JAL x0, Division_Unsigned_2Word_Loop_Branch_End\n");


    fprintf(stdoutFP,"Division_Unsigned_2Word_Loop_Branch_False:\n");
    fprintf(stdoutFP,"Division_Unsigned_2Word_Loop_Branch_End:\n");



    fprintf(stdoutFP,"  ADDI s8, s8, 1\n");
    fprintf(stdoutFP,"  JAL x0, Division_Unsigned_2Word_Loop\n");

    fprintf(stdoutFP,"Division_Unsigned_2Word_Loop_End:\n");


    //shift output to the right based on number of initial shifts


    fprintf(stdoutFP,"Shift_Right_Loop%d:\n", labelNum);
    fprintf(stdoutFP,"  BGE x0, s9, Shift_Right_Loop_End%d\n", labelNum);
    fprintf(stdoutFP,"  ANDI t0, s0, 1\n"); //t0 = LSB of upper upper Quotient
    fprintf(stdoutFP,"  ANDI t1, s1, 1\n"); //t1 = LSB of upper Quotient
    fprintf(stdoutFP,"  ANDI t2, s2, 1\n"); //t2 = LSB of lower Quotient
    fprintf(stdoutFP,"  SLLI t0, t0, 31\n");
    fprintf(stdoutFP,"  SLLI t1, t1, 31\n");
    fprintf(stdoutFP,"  SLLI t2, t2, 31\n");
    
    fprintf(stdoutFP,"  SRLI s0, s0, 1\n");
    fprintf(stdoutFP,"  SRLI s1, s1, 1\n");
    fprintf(stdoutFP,"  SRLI s2, s2, 1\n");
    fprintf(stdoutFP,"  SRLI s3, s3, 1\n");



    fprintf(stdoutFP,"  OR s1, s1, t0\n");
    fprintf(stdoutFP,"  OR s2, s2, t1\n");
    fprintf(stdoutFP,"  OR s3, s3, t2\n");
    fprintf(stdoutFP,"  ADDI s9, s9, -1\n"); //decrement shift counter
    fprintf(stdoutFP,"  JAL x0, Shift_Right_Loop%d\n", labelNum);

    fprintf(stdoutFP,"Shift_Right_Loop_End%d:\n", labelNum);
    //set outputs
    fprintf(stdoutFP,"  ADD a0, s0, x0\n");
    fprintf(stdoutFP,"  ADD a1, s1, x0\n");
    fprintf(stdoutFP,"  ADD a2, s2, x0\n");
    fprintf(stdoutFP,"  ADD a3, s3, x0\n");

    //restore S registers from stack


    fprintf(stdoutFP,"  LW s0, 0(sp)\n");
    fprintf(stdoutFP,"  LW s1, 4(sp)\n");
    fprintf(stdoutFP,"  LW s2, 8(sp)\n");
    fprintf(stdoutFP,"  LW s3, 12(sp)\n");
    fprintf(stdoutFP,"  LW s4, 16(sp)\n");
    fprintf(stdoutFP,"  LW s5, 20(sp)\n");
    fprintf(stdoutFP,"  LW s6, 24(sp)\n");
    fprintf(stdoutFP,"  LW s7, 28(sp)\n");
    fprintf(stdoutFP,"  LW s8, 32(sp)\n");
    fprintf(stdoutFP,"  LW s9, 36(sp)\n");
    fprintf(stdoutFP,"  LW s10, 40(sp)\n");
    fprintf(stdoutFP,"  LW s11, 44(sp)\n");
    fprintf(stdoutFP,"  ADDI sp, sp, 48\n");

    fprintf(stdoutFP,"  JALR x0, ra, 0\n");

}

void generatePrintFunction(CodeGenerator * gen){
    //address in a0


    fprintf(stdoutFP,"Print:\n");

    fprintf(stdoutFP,"  addi sp, sp, -16\n");
    fprintf(stdoutFP,"  sw ra, 12(sp)\n");
    fprintf(stdoutFP,"  sw s0, 8(sp)\n");
    fprintf(stdoutFP,"  call printf\n");
    fprintf(stdoutFP,"  lw ra, 12(sp)\n");
    fprintf(stdoutFP,"  lw s0, 8(sp)\n");
    fprintf(stdoutFP,"  addi sp, sp, 16\n");


    fprintf(stdoutFP, "  JALR x0, ra, 0\n");
}

void generateOrFunction(CodeGenerator * gen){
    //a0 left, a1 right
    fprintf(stdoutFP,"OrFunction:\n");
    fprintf(stdoutFP,"  ADD t1, a0, a1\n");
    fprintf(stdoutFP,"  SLTU a0, x0, t1\n");
    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
}

void generateAndFunction(CodeGenerator * gen){
    fprintf(stdoutFP,"AndFunction:\n");
    fprintf(stdoutFP,"  ADD t1, a0, a1\n");
    fprintf(stdoutFP,"  ADDI t1, t1, -1\n");
    fprintf(stdoutFP,"  SLT a0, x0, t1\n");
    fprintf(stdoutFP,"  JALR x0, ra, 0\n");

}

void generateNotFunction(CodeGenerator * gen){
    fprintf(stdoutFP,"NotFunction:\n");
    fprintf(stdoutFP,"  SLTIU a0, a0, 1\n");
    fprintf(stdoutFP,"  JALR x0, ra, 0\n");
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



    int labelNum = gen->next_label++;
    fprintf(stdoutFP, "LT2Word:\n");
    
    fprintf(stdoutFP, "  BLT a0, a2, EndTrue%d\n", labelNum);
    fprintf(stdoutFP, "  BLT a2, a0, EndFalse%d\n", labelNum);
    fprintf(stdoutFP, "  SLT t0, a0, x0\n");  //t0 = are they negative
    fprintf(stdoutFP, "  XORI t1, t0, 1\n");   //t1 = not t0
    fprintf(stdoutFP, "  BLTU a1, a3, LowerLT%d\n", labelNum);
    fprintf(stdoutFP, "  BLTU a3, a1, LowerGT%d\n", labelNum);

    fprintf(stdoutFP, "  ADDI a0, x0, 0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "EndTrue%d:\n", labelNum);
    fprintf(stdoutFP, "  ADDI a0, x0, 1\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");
    
    fprintf(stdoutFP, "EndFalse%d:\n", labelNum);
    fprintf(stdoutFP, "  ADDI a0, x0, 0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "LowerLT%d:\n", labelNum);
    fprintf(stdoutFP, "  ADD a0, t0, x0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "LowerGT%d:\n", labelNum);
    fprintf(stdoutFP, "  ADD a0, t1, x0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");




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



    int labelNum = gen->next_label++;
    fprintf(stdoutFP, "LTE2Word:\n");
    
    fprintf(stdoutFP, "  BLT a0, a2, EndTrue%d\n", labelNum);
    fprintf(stdoutFP, "  BLT a2, a0, EndFalse%d\n", labelNum);
    fprintf(stdoutFP, "  SLT t0, a0, x0\n");  //t0 = are they negative
    fprintf(stdoutFP, "  XORI t1, t0, 1\n");   //t1 = not t0
    fprintf(stdoutFP, "  BLTU a1, a3, LowerLT%d\n", labelNum);
    fprintf(stdoutFP, "  BLTU a3, a1, LowerGT%d\n", labelNum);

    fprintf(stdoutFP, "  ADDI a0, x0, 1\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "EndTrue%d:\n", labelNum);
    fprintf(stdoutFP, "  ADDI a0, x0, 1\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");
    
    fprintf(stdoutFP, "EndFalse%d:\n", labelNum);
    fprintf(stdoutFP, "  ADDI a0, x0, 0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "LowerLT%d:\n", labelNum);
    fprintf(stdoutFP, "  ADD a0, t0, x0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");

    fprintf(stdoutFP, "LowerGT%d:\n", labelNum);
    fprintf(stdoutFP, "  ADD a0, t1, x0\n");
    fprintf(stdoutFP, "  JALR x0, ra, 0\n");




}

void generatePrintFractionalToDecimalInt(CodeGenerator * gen){
    //converts fractional word into integer word of its representation in binary 
    //ex .75 gets turned to 75

    //input: a0 = fractional word
    //output: a0 = integer word
    //temp regs: t0 - tempaddress , t1 - upper, t2 - lower, t3 - 10, t4- final res



/*
ConvertFractionToDecimalInteger:


printf("addi	sp,sp,-16\n");
printf("sw	ra,12(sp)\n");
printf("sw	s0,8(sp)\n");
printf("sw a1, 0(sp)\n");
printf("sw a0, 4(sp)\n");
printf("addi	s0,sp,16\n");

printf("lui	a0,%hi(.LC0)\n");
printf("addi	a0,a0,%lo(.LC0)\n");

printf("call	printf\n");

printf("lw	ra,12(sp)\n");
printf("lw	s0,8(sp)\n");
printf("lw  a1, 0(sp)\n");
printf("lw a0, 4(sp)\n");
printf("addi	sp,sp,16\n");


ADD t0, x0, x0
ADD t2, a0, x0
ADDI t3, x0, 10
ADDI t5, x0, 0
Loop8:
  ADDI t5, t5, 1
  BEQ t5, t3, End8
  BEQ t2, x0, End8
  ADDI sp, sp, -40
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW t0, 8(sp)
  SW t1, 12(sp)
  SW t2, 16(sp)
  SW t3, 20(sp)
  SW t4, 24(sp)
  SW t5, 28(sp)
  SW t6, 32(sp)
  SW ra, 36(sp)
  ADD a0, t2, zero
  ADD a1, t3, zero
  JAL ra, Multiplication_Unsigned_1Word
  LW t0, 8(sp)
  LW t1, 12(sp)
  LW t2, 16(sp)
  LW t3, 20(sp)
  LW t4, 24(sp)
  LW t5, 28(sp)
  LW t6, 32(sp)
  ADD t1, a0, zero
  ADD t2, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 36(sp)
  ADDI sp, sp, 40



	addi	sp,sp,-16
	sw	ra,12(sp)
	sw	s0,8(sp)
  sw a1, 0(sp)
  sw a0, 4(sp)
	addi	s0,sp,16

	lui	a0,%hi(.LCD)
	addi	a0,a0,%lo(.LCD)
  addi a1, t1, 0
	call	printf

	lw	ra,12(sp)
	lw	s0,8(sp)
  lw  a1, 0(sp)
  lw a0, 4(sp)
	addi	sp,sp,16


  ADD t0, t0, t1
  ADD t1, x0, x0
  JAL x0, Loop8
End8:
  JALR x0, ra, 0
	.align	4
	.globl	main
*/
   int labelNum = gen->next_label++;

   fprintf(stdoutFP,"ConvertFractionToDecimalInteger:\n");

    //round to five decimal places

fprintf(stdoutFP,"  add t0, a0, x0\n");
fprintf(stdoutFP,"  srai t1, t0, 16\n");
fprintf(stdoutFP,"  andi t1, t1, 1\n");
fprintf(stdoutFP,"  li t3,  0xFFFF0000\n");
fprintf(stdoutFP,"  and t0, t0, t3\n");
fprintf(stdoutFP,"  beq t1, x0, skip_rounding%d\n", labelNum);

fprintf(stdoutFP,"  li t3, 0x00010000\n");
fprintf(stdoutFP,"  add t0, t0, t3\n");
fprintf(stdoutFP,"  skip_rounding%d:\n", labelNum);


fprintf(stdoutFP,"  Add a0, t0, x0\n");


//print function idk why this works, but it breaks if its not here
    fprintf(stdoutFP,"  addi	sp,sp,-16\n");
    fprintf(stdoutFP,"  sw	ra,12(sp)\n");
    fprintf(stdoutFP,"  sw	s0,8(sp)\n");
    fprintf(stdoutFP,"  sw a1, 0(sp)\n");
    fprintf(stdoutFP,"  sw a0, 4(sp)\n");
    fprintf(stdoutFP,"  addi	s0,sp,16\n");

    fprintf(stdoutFP,"  lui	a0,%%hi(.LCEmpty)\n");
    fprintf(stdoutFP,"  addi	a0,a0,%%lo(.LCEmpty)\n");

    fprintf(stdoutFP,"  call	printf\n");

    fprintf(stdoutFP,"  lw	ra,12(sp)\n");
    fprintf(stdoutFP,"  lw	s0,8(sp)\n");
    fprintf(stdoutFP,"  lw  a1, 0(sp)\n");
    fprintf(stdoutFP,"  lw a0, 4(sp)\n");
    fprintf(stdoutFP,"  addi	sp,sp,16\n");



    //intitialize
   fprintf(stdoutFP,"  ADD t0, x0, x0\n");
   fprintf(stdoutFP,"  ADD t2, a0, x0\n");
   fprintf(stdoutFP,"  ADDI t3, x0, 10\n");
   fprintf(stdoutFP, "  ADDI t5, x0, 0\n");
   



    fprintf(stdoutFP,"Loop%d:\n", labelNum);
    fprintf(stdoutFP, "  ADDI t5, t5, 1\n");
    
    fprintf(stdoutFP, "  ADDI t3, x0, 6\n");
    fprintf(stdoutFP, "  BEQ t5, t3, End%d\n", labelNum);
    fprintf(stdoutFP, "  ADDI t3, x0, 10\n");

    //if lower is zero, then end
    fprintf(stdoutFP,"  BEQ t2, x0, End%d\n", labelNum);

    
    //multiply lower by 10
    char * regIn[] = {"t2", "t3"};
    char * regOut[] = {"t1", "t2"};
    callFunction(gen, "Multiplication_Unsigned_1Word", regIn, 2, regOut, 2);

    //print upper

    fprintf(stdoutFP,"  addi	sp,sp,-16\n");
    fprintf(stdoutFP,"  sw	ra,12(sp)\n");
    fprintf(stdoutFP,"  sw	s0,8(sp)\n");
    fprintf(stdoutFP,"  sw a1, 0(sp)\n");
    fprintf(stdoutFP,"  sw a0, 4(sp)\n");
    fprintf(stdoutFP,"  addi	s0,sp,16\n");

    fprintf(stdoutFP,"  lui	a0,%%hi(.LCD)\n");
    fprintf(stdoutFP,"  addi	a0,a0,%%lo(.LCD)\n");
    fprintf(stdoutFP,"  add a1, x0, t1\n");

    fprintf(stdoutFP,"  call	printf\n");

    fprintf(stdoutFP,"  lw	ra,12(sp)\n");
    fprintf(stdoutFP,"  lw	s0,8(sp)\n");
    fprintf(stdoutFP,"  lw  a1, 0(sp)\n");
    fprintf(stdoutFP,"  lw a0, 4(sp)\n");
    fprintf(stdoutFP,"  addi	sp,sp,16\n");



    //add upper to result, then reset upper
    fprintf(stdoutFP,"  ADD t0, t0, t1\n");
    fprintf(stdoutFP,"  ADD t1, x0, x0\n");



    fprintf(stdoutFP,"  JAL x0, Loop%d\n", labelNum);


    fprintf(stdoutFP,"End%d:\n",labelNum);


    fprintf(stdoutFP,"  JALR x0, ra, 0\n");


}




























void enterScope(CodeGenerator * gen){
   //push new variable table onto the stack
    Map * newScope = createMap(compareStrings);
    VariableTable * table = (VariableTable *)malloc(sizeof(VariableTable));
    table->variables = newScope;
    table->frameSize = 0;
    push(gen->variableTableStack, table);


    

    // set the FP to the SP

    fprintf(stdoutFP,"  ADD fp, sp, x0\n");

}
void leaveScope(CodeGenerator * gen){
    
    VariableTable * newTable = peekAtIndex(gen->variableTableStack, 1);
    int frameSize = newTable->frameSize;
    VariableTable * table = pop(gen->variableTableStack);
    //int frameSize = table->frameSize;
    
    
    fprintf(stdoutFP,"  ADD sp, fp, x0\n");



    fprintf(stdoutFP,"  ADDI fp, fp, %d\n", frameSize);
    
}
void leaveScopeReturn(CodeGenerator * gen){
    fprintf(stdoutFP,"  ADD sp, fp, x0\n");
    VariableTable * table = peekAtIndex(gen->variableTableStack, 1);
    int frameSize = table->frameSize;
    //printf("  ADDI fp, fp, %d\n", frameSize);
    fprintf(stdoutFP,"  LW fp, 0(fp)\n");
}

void addNewVariable(CodeGenerator * gen, Symbol identifierSym, DataType type){
    VariableTable * currentScope = peek(gen->variableTableStack);
    int size = getSize(type);

    //currentScope->frameSize += size;
    incrementStackPointer(gen, -size);

    char * word = (char *)identifierSym.value;

    VariableTableEntry * entry = createVariableTableEntry(type, -(currentScope->frameSize), NULL);
    insert(currentScope->variables, word, entry);

    
}


void initialize(CodeGenerator * gen, int sp){
    gen->stackPointer = sp;

    
    gen->stack_offset = 0;





    generate2WordAdditionFunction(gen);
    generateUnsigned1WordMultiplicationFunction(gen);
    generateUnsignedFixedPointMultiplicationFunction(gen);
    generateSigned1WordMultiplicationFunction(gen);
    generateSignedFixedPointMultiplicationFunction(gen);
    generate2WordSubtractionFunction(gen);
    generateUnsigned2WordDivisionFunction(gen);
    generatePrintFunction(gen);
    generateOrFunction(gen);
    generateAndFunction(gen);
    generateNotFunction(gen);
    generate2WordLessThanFunction(gen);
    generate2WordLessThanEqualFunction(gen);
    generatePrintFractionalToDecimalInt(gen);

    

    fprintf(stdoutFP, "	.align	4\n");
    fprintf(stdoutFP, "	.globl	main\n");
    fprintf(stdoutFP, "main:\n");
    enterScope(gen);    
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

        
        fprintf(stdoutFP,"  ADD %s, %s, %s\n", result_reg->name, left_reg->name, right_reg->name);

    
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


void castIntToDec(CodeGenerator* gen, Register * intReg, Register * decUpperReg, Register * decLowerReg){

    fprintf(stdoutFP,"  ADD %s, %s, x0\n",decUpperReg->name, intReg->name);
    fprintf(stdoutFP,"  ADD %s, x0, x0\n", decLowerReg->name);
}

void castDecToInt(CodeGenerator * gen, Register * decUpperReg, Register * decLowerReg, Register * intReg){

    Register * doNotSpill[] = {decUpperReg, decLowerReg, intReg, NULL, NULL};
    //if dec upper is negative && dec lower > 0 
 
    //  then int = dec upper + 1
    //else int =  dec upper

    Register * temp_reg1 = allocate_register(gen, NULL, 0, doNotSpill, 3, -1);
    doNotSpill[3] = temp_reg1;
    Register * temp_reg2 = allocate_register(gen, NULL, 0, doNotSpill, 4, -1);
    fprintf(stdoutFP,"  ADD %s, %s, x0\n", intReg->name, decUpperReg->name);

    fprintf(stdoutFP,"  SLT %s, %s, x0\n", temp_reg1->name, decUpperReg->name); //temp reg 1 has 
    
    fprintf(stdoutFP,"  SLTIU %s, %s, 1\n", temp_reg2->name, decLowerReg->name); // temp 2 = dec_lower > 0
    fprintf(stdoutFP,"  AND %s, %s, %s\n", temp_reg1->name, temp_reg1->name, temp_reg2->name);

    fprintf(stdoutFP,"  ADD %s, %s, %s\n", intReg->name, intReg->name, temp_reg1->name);

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


    if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN){
        
        resultLower = allocate_register(gen, node, 0, doNotSpill, 0, -1);
        doNotSpill[0] = resultLower;
        
        valueLower = get_register(gen, valueNode, 0, doNotSpill, 1);
        fprintf(stdoutFP,"  ADD %s, %s, x0\n", resultLower->name, valueLower->name);
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

        fprintf(stdoutFP,"  ADD %s, %s, x0\n", resultUpper->name, valueUpper->name);
        fprintf(stdoutFP,"  ADD %s, %s, x0\n", resultLower->name, valueLower->name);

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

        if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN){

            fprintf(stdoutFP,"  SW %s, 0(sp)\n", resultLower->name);

        }


        if(type == TYPE_DECIMAL){

            fprintf(stdoutFP,"  SW %s, 0(sp)\n", resultUpper->name);
            fprintf(stdoutFP,"  SW %s, 4(sp)\n", resultLower->name);

        }
        
    }


}

void identifierNode(CodeGenerator * gen, AST_Node * node){
    
    char * word = node->symbol.value;
    Register * doNotSpill[] = {NULL, NULL, NULL};

    int totalOffset = 0;

    int i = 0;

    while(true){

        VariableTable * table = peekAtIndex(gen->variableTableStack, i++);
        VariableTableEntry * entry = get(table->variables, word);
        totalOffset += table->frameSize;



        if(entry != NULL){
            
            DataType type = entry->type;

            int offset = entry->memoryLocation;
            Register * resultUpper, * resultLower;

            int finalOffset = offset + totalOffset;
            if(i == 1)finalOffset = offset;
          


            printf("in identifier node. node id = %d,  i = %d, final offset  = %d\n",node->id,  i, finalOffset);


            if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN){
                resultLower = allocate_register(gen, node, 0, doNotSpill, 0, -1);
                fprintf(stdoutFP,"  LW %s, %d(fp)\n", resultLower->name, finalOffset);
            }

            if(type == TYPE_DECIMAL){
                resultUpper = allocate_register(gen, node, 0, doNotSpill, 0, -1);
                doNotSpill[0] = resultUpper;
                resultLower = allocate_register(gen, node, 1, doNotSpill, 1, -1);
                
                fprintf(stdoutFP,"  LW %s, %d(fp)\n", resultUpper->name, finalOffset);
                offset +=4;
                fprintf(stdoutFP,"  LW %s, %d(fp)\n", resultLower->name, finalOffset + 4);

            }

            return;
            
        }

    

   

    }

}

void functionDeclarationNode(CodeGenerator * gen, AST_Node * node){
    printf("in function definition node\n");
    char * key = functionDefinitionToKey(node);
    FunctionArguments functionArgs = getFunctionArguments(node);

    printf("function node 0.5\n");

    char * label = addLabel(gen, key);

    printf("function ode 0.51\n");
    FunctionTableEntry * entry = get(gen->functionTable, key);


    printf("function node -.75\n");


    entry->label = label;

    printf("function def 1\n");

    char * endLabel;
    asprintf(&endLabel, "%s_end", label);
    endLabel = addLabel(gen, endLabel);

    fprintf(stdoutFP,"JAL x0, %s\n", endLabel);
    fprintf(stdoutFP,"%s:\n", label);


    //incrementStackPointer(gen, -4);
    //printf("  ADDI sp, sp, -4");
    //printf("  SW fp, 0(sp)\n");
    enterScope(gen);

    printf("function def 2\n");
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
                fprintf(stdoutFP,"  SW a%d, %d(sp)\n", regNum, j * 4);
                regNum++;
                
            }

            else{
                //data is in the stack
                int numArgsInStack = functionArgs.totalSize / 4;
                if(numArgsInStack > 6) numArgsInStack = 6;
                //int additionalOffset = 4 + (numArgsInStack * 4);
                int offset = ((regNum - 6) * 4);
                fprintf(stdoutFP,"  LW a0, %d(fp)\n", offset);
                fprintf(stdoutFP,"  SW a0, %d(sp)\n", j * 4);
                regNum++;
            }

            
        }


    }


    printf("about to generate body node\n");
    /////////////Now process function body
    AST_Node * bodyNode = node->children[5];
    printf("got body node\n");
    generateCode(gen, bodyNode);
    printf("sucess generate body node\n");


    leaveScope(gen);

    fprintf(stdoutFP,"%s:\n", endLabel);
    printf("made it out of function definition node\n");
}

void returnNode(CodeGenerator * gen, AST_Node * node){
  
    AST_Node * valueNode = node->children[2];
    DataType type = valueNode->resultType;

    Register * valueLower, * valueUpper;
    Register * doNotSpill[] = {NULL,NULL, NULL, NULL};
    
    generateCode(gen, valueNode);

    if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN){

        valueLower = get_register(gen, valueNode, 0, doNotSpill, 0);
        fprintf(stdoutFP,"  ADD a0, %s, x0\n", valueLower->name);
        free_register(gen, valueLower);
        
    }

    if(type == TYPE_DECIMAL){


        valueLower = get_register(gen, valueNode, 1, doNotSpill, 0);
        doNotSpill[0] = valueLower;
        valueUpper = get_register(gen, valueNode, 0, doNotSpill, 1);

        fprintf(stdoutFP,"  ADD a0, %s, x0\n", valueUpper->name);
        fprintf(stdoutFP,"  ADD a1, %s, x0\n", valueLower->name);

        free_register(gen, valueUpper);
        free_register(gen, valueLower);
        
    }

    //leaveScopeReturn(gen);
    fprintf(stdoutFP,"JALR x0, ra, 0\n");


    
    
}

void functionCallNode(CodeGenerator * gen, AST_Node * node){
    char * key = functionDefinitionToKey(node->declarationNode);
    FunctionTableEntry * entry = get(gen->functionTable, key);
    DataType resultType = entry->returnType;
    
    FunctionArguments functionArgs = getFunctionArguments(entry->declarationNode);



    //must load function args into correct locations
    int numChildren = node->num_children;
    AST_Node ** functionCallArgs = (AST_Node **)malloc(sizeof(AST_Node*) * numChildren);
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
    Register * doNotSpill[] = {NULL, NULL, NULL, NULL};

    tempRegUpper = allocate_register(gen, NULL, 0, doNotSpill, 0, -1);
    doNotSpill[0] = tempRegUpper;
    tempRegLower = allocate_register(gen, NULL, 0, doNotSpill, 1, -1);
    free_register(gen, tempRegUpper);
    free_register(gen, tempRegLower);


    //store a reg's to stack before overwriting them
    

    int numWords = functionArgs.totalSize / 4;
    int stackIncrementation = numWords;
    if(stackIncrementation > 7)stackIncrementation = 7;
   // printf("  ADDI sp, sp, %d\n", (stackIncrementation + 1)* -4);
    incrementStackPointer(gen, (stackIncrementation + 1)* -4);
    fprintf(stdoutFP,"  SW ra, 0(sp)\n");
    for(int i = 0; i < numWords && i < 7; i++){
        fprintf(stdoutFP,"  SW a%d, %d(sp)\n", i, (i + 1)*4);
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
        fprintf(stdoutFP,"  SW %s, 0(sp)\n", reg->name);
        stackWords++;

        wordIndex++;

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

        fprintf(stdoutFP,"  ADD a%d, %s, x0\n", i, reg->name);
        wordIndex++;

        if(wordIndex >= argWords){
            wordIndex = 0;
            argIndex++;
        }

        free_register(gen, reg);

    }


    incrementStackPointer(gen, -4);
    fprintf(stdoutFP,"  SW fp, 0(sp)\n");
    fprintf(stdoutFP,"  ADD fp, sp, x0\n");
    
    

    fprintf(stdoutFP,"  JAL ra, %s\n", entry->label);

    fprintf(stdoutFP,"  ADD sp, fp, x0\n");
    fprintf(stdoutFP,"  LW fp, 0(fp)\n");




    





    Register * resultRegUpper, * resultRegLower;

    if(resultType == TYPE_INTEGER || resultType == TYPE_STRING || resultType == TYPE_BOOLEAN){
        
        resultRegLower = allocate_register(gen, node, 0, doNotSpill, 0, -1);
        fprintf(stdoutFP,"  ADD %s, a0, x0\n", resultRegLower->name);
    }

    if(resultType == TYPE_DECIMAL){
        resultRegUpper = allocate_register(gen, node, 0, doNotSpill, 0, -1);
        doNotSpill[0] = resultRegUpper;
        resultRegLower = allocate_register(gen ,node, 1, doNotSpill, 1, -1);

        fprintf(stdoutFP,"  ADD %s, a0, x0\n", resultRegUpper->name);
        fprintf(stdoutFP,"  ADD %s, a1, x0\n",resultRegLower->name);
    }



    fprintf(stdoutFP,"  LW ra, 4(sp)\n");
    for(int i = 0; i < numWords && i < 7; i++){
        fprintf(stdoutFP,"  LW a%d, %d(sp)\n", i, (i + 1)*4);
    }
   // printf("  ADDI sp, sp, %d\n", (stackIncrementation + 1) * 4);
    incrementStackPointer(gen,(stackIncrementation + 1) * 4 );





    


}

void stringNode(CodeGenerator * gen, AST_Node * node){
    printf("in string node\n");
    Symbol sym = node->symbol;
    
    //value is a pointer to the address
    int * labelNum = get(gen->stringTable,sym.value);



    Register * reg = allocate_register(gen, node, 0, NULL, 0, -1);
    fprintf(stdoutFP, "  LUI %s, %%hi(.LC%d)\n", reg->name, *labelNum);
    fprintf(stdoutFP, "  ADDI %s, %s, %%lo(.LC%d)\n", reg->name, reg->name, *labelNum);

    printf("out of string node\n");
}

void printStatementNode(CodeGenerator * gen, AST_Node * node){
    printf("in print staement node\n");
    int labelNum = gen->next_label++;

    printf("in print statement node\n");
    AST_Node * valueNode = node->children[1];

    generateCode(gen, valueNode);

    printf("got value node\n");
    Register * doNotSpill[] = {NULL, NULL, NULL};
    Register * valueReg = get_register(gen, valueNode, 0, doNotSpill, 0);
    doNotSpill[0] = valueReg;

    DataType type = valueNode->resultType;
    if(type == TYPE_INTEGER){
        Register * addressReg  = allocate_register(gen, NULL, 0, doNotSpill, 1, -1);
        fprintf(stdoutFP,"  LUI %s, %%hi(.LCD)\n", addressReg->name);
        fprintf(stdoutFP, "  ADDI %s, %s, %%lo(.LCD)\n", addressReg->name, addressReg->name);
        char * regIn[] = {addressReg->name, valueReg->name};
        callFunction(gen, "printf", regIn, 2, NULL, 0);
        free_register(gen, addressReg);
    }

    else if(type == TYPE_DECIMAL){




        Register * valueRegLower = get_register(gen, valueNode, 1, doNotSpill, 1);
        doNotSpill[1] = valueRegLower;
        Register * addressReg  = allocate_register(gen, NULL, 0, doNotSpill, 2, -1);
        fprintf(stdoutFP,"  LUI %s, %%hi(.LCF)\n", addressReg->name);
        fprintf(stdoutFP, "  ADDI %s, %s, %%lo(.LCF)\n", addressReg->name, addressReg->name);
        char * regIn[] = {addressReg->name, valueReg->name};
        callFunction(gen, "printf", regIn, 2, NULL, 0);
        free_register(gen, addressReg);


        char * regIn2[] = {valueRegLower->name};
        callFunction(gen, "ConvertFractionToDecimalInteger", regIn2, 1, NULL, 0);





    }

    else if(type == TYPE_STRING){
        char * regIn[] = {valueReg->name};
        callFunction(gen, "printf", regIn, 1, NULL, 0);
    }


    else if(type == TYPE_BOOLEAN){

        Register * addressReg = allocate_register(gen, NULL, 0, doNotSpill,1, -1);

        

        fprintf(stdoutFP,"  BEQ %s, x0, PrintFalse%d\n", valueReg->name, labelNum);



        fprintf(stdoutFP,"PrintTrue%d:\n", labelNum);
        fprintf(stdoutFP,"  LUI %s, %%hi(.LCBT)\n", addressReg->name);
        fprintf(stdoutFP,"  ADDI %s, %s, %%lo(.LCBT)\n", addressReg->name, addressReg->name);
        char * regIn[] = {addressReg->name};
        callFunction(gen, "printf", regIn, 1, NULL, 0);
        fprintf(stdoutFP,"  JAL x0, PrintEnd%d\n", labelNum);




        fprintf(stdoutFP,"PrintFalse%d:\n", labelNum);
        fprintf(stdoutFP,"  LUI %s, %%hi(.LCBF)\n", addressReg->name);
        fprintf(stdoutFP,"  ADDI %s, %s, %%lo(.LCBF)\n", addressReg->name, addressReg->name);

        callFunction(gen, "printf", regIn, 1, NULL, 0);
        fprintf(stdoutFP,"  JAL x0, PrintEnd%d\n", labelNum);

        fprintf(stdoutFP,"PrintEnd%d:\n", labelNum);
        free_register(gen, addressReg);

    }


    


    free_register(gen, valueReg);
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
    AST_Node * leftChild = node->children[1];


    generateCode(gen, leftChild);


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

    
    int labelNum = gen->next_label++;

    fprintf(stdoutFP,"  BEQ %s, x0, ConditionFalse%d\n", ifReg->name, labelNum);
    

    enterScope(gen);


    generateCode(gen, thenNode);

    leaveScope(gen);

    fprintf(stdoutFP,"ConditionFalse%d:\n", labelNum);


}

void ifElseStatementNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * ifNode = node->children[1];
    AST_Node * thenNode = node->children[3];
    AST_Node * elseNode = node->children[5];

    printf("in ifelse, about to gen for if\n");
    generateCode(gen, ifNode);
    printf("finished making if\n");

    Register * doNotSpill[] = {NULL, NULL, NULL};

    Register * ifReg = get_register(gen, ifNode, 0, doNotSpill, 0);
    doNotSpill[0] = ifReg;

    
    int labelNum = gen->next_label++;

    fprintf(stdoutFP,"  BEQ %s, x0, ConditionFalse%d\n", ifReg->name, labelNum);
    

    enterScope(gen);


    generateCode(gen, thenNode);
    fprintf(stdoutFP, "  JAL x0, End%d\n", labelNum);

    leaveScope(gen);

    fprintf(stdoutFP,"ConditionFalse%d:\n", labelNum);

    enterScope(gen);

    generateCode(gen, elseNode);


    fprintf(stdoutFP, "End%d:\n", labelNum);
    leaveScope(gen);


}

void assignmentNode(CodeGenerator * gen, AST_Node * node){
    AST_Node * valueNode = node->children[2];
    char * word = node->children[0]->symbol.value;
    printf("assignment node word = %s\n", word);

    generateCode(gen, valueNode);

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL};
    Register * valueRegUpper,  * valueRegLower, * resultRegUpper, * resultRegLower;

    DataType type = valueNode->resultType;
    if(type == TYPE_BOOLEAN || type == TYPE_INTEGER || type == TYPE_DECIMAL){
        valueRegLower = get_register(gen, valueNode, 0, doNotSpill, 0);
        doNotSpill[0] = valueRegLower;
        resultRegLower = allocate_register(gen, node, 0, doNotSpill, 1, -1);
        fprintf(stdoutFP, "  ADD %s, %s, x0\n", resultRegLower->name, valueRegLower->name);

        free_register(gen, valueRegLower);


    }


    if(type == TYPE_DECIMAL){
        valueRegUpper = get_register(gen, valueNode, 0, doNotSpill, 0);
        doNotSpill[0] = valueRegUpper;
        valueRegLower = get_register(gen, valueNode, 1, doNotSpill, 1);
        doNotSpill[1] = valueRegLower;
        resultRegUpper = allocate_register(gen, valueNode, 0, doNotSpill, 2, -1);
        doNotSpill[2] = resultRegUpper;
        resultRegLower = allocate_register(gen, valueNode, 1, doNotSpill, 3, -1);
        fprintf(stdoutFP, "  ADD %s, %s, x0\n", resultRegLower->name, valueRegLower->name);
        fprintf(stdoutFP, "  ADD %s, %s, x0\n", resultRegUpper->name, valueRegUpper->name);
        free_register(gen, valueRegUpper);
        free_register(gen, valueRegLower);


    }



    int totalOffset = 0;

    int i = 0;
    printf("about to go into while loop");
    while(true){

        VariableTable * table = peekAtIndex(gen->variableTableStack, i);
        VariableTableEntry * entry = get(table->variables, word);

        i++;


        if(entry != NULL){

            totalOffset += table->frameSize;
            DataType type = entry->type;

            int offset = entry->memoryLocation;
            int finalOffset = offset+totalOffset;


            printf("in assignment. i = %d, node id = %d, finaloffset  = %d, offset = %d, totalofset = %d\n", i, node->id, finalOffset, offset, totalOffset);


            if(type == TYPE_INTEGER || type == TYPE_STRING || type == TYPE_BOOLEAN){
                    
                    fprintf(stdoutFP,"  SW %s, %d(fp)\n", resultRegLower->name, finalOffset);
            
            }

            if(type == TYPE_DECIMAL){

                fprintf(stdoutFP,"  SW %s, %d(fp)\n", resultRegUpper->name, finalOffset);
                offset +=4;
                fprintf(stdoutFP,"  SW %s, %d(fp)\n", resultRegLower->name, finalOffset + 4);

            }

            return;
            
        }


        if(i!=1)totalOffset += table->frameSize;

    }

}

void unaryNegationNode(CodeGenerator * gen, AST_Node * node){


    AST_Node * valueNode = node->children[1];

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL};

    Register * valueUpper, * valueLower, * resultUpper, * resultLower;




    generateCode(gen, valueNode);
    
    if(valueNode->resultType == TYPE_INTEGER){
        
        valueLower = get_register(gen, valueNode, 0, doNotSpill, 0);
        doNotSpill[0] = valueLower;
        resultLower = allocate_register(gen, node, 0, doNotSpill, 1, -1);
        
        fprintf(stdoutFP,"  SUB %s, x0, %s\n", resultLower->name, valueLower->name);
        free_register(gen, valueLower);

    }




    if(valueNode->resultType == TYPE_DECIMAL){
        
        valueUpper = get_register(gen, valueNode,0, doNotSpill, 0);
        doNotSpill[0] = valueUpper;
        valueLower = get_register(gen, valueNode, 1, doNotSpill, 1);
        doNotSpill[1] = valueLower;
        resultUpper = allocate_register(gen, node, 0 , doNotSpill, 2, -1);
        doNotSpill[2] = resultUpper;
        resultLower = allocate_register(gen, node, 1, doNotSpill, 3, -1);
        doNotSpill[3] = resultLower;
        Register * tempReg = allocate_register(gen, NULL, 0, doNotSpill, 4, -1);




        fprintf(stdoutFP,"  XORI %s, %s, -1\n", resultUpper->name, valueUpper->name);
        fprintf(stdoutFP,"  XORI %s, %s, -1\n", resultLower->name, valueLower->name);

        fprintf(stdoutFP,"  ADDI %s, x0, 1\n", tempReg->name);





        char * regIn[] = {resultUpper->name, resultLower->name, "x0", tempReg->name};
        char * regOut[] = {resultUpper->name, resultLower->name};

        callFunction(gen, "Addition_2Word", regIn, 2, regOut, 2);


        free_register(gen, valueUpper);
        free_register(gen, valueLower);
        free_register(gen, tempReg);


    }

}

void whileLoopNode(CodeGenerator * gen, AST_Node * node){

    AST_Node * conditionNode = node->children[1];
    AST_Node * bodyNode = node->children[2];

    Register * doNotSpill[] = {NULL, NULL};

    int labelNum = gen->next_label++;

    enterScope(gen);

    fprintf(stdoutFP, "WhileLoopCondition%d:\n", labelNum);
    generateCode(gen, conditionNode);

    Register * conditionReg = get_register(gen, conditionNode, 0, NULL, 0);


    fprintf(stdoutFP, "  BEQ %s, x0, WhileLoopEnd%d\n", conditionReg->name,  labelNum);
    
    fprintf(stdoutFP, "WhileLoopBody%d:\n", labelNum);

    generateCode(gen, bodyNode);

    fprintf(stdoutFP, "  JAL x0, WhileLoopCondition%d\n", labelNum);

    fprintf(stdoutFP, "WhileLoopEnd%d:\n", labelNum);

    leaveScope(gen);


}

void relationalExpressionNode(CodeGenerator * gen, AST_Node * node){
    int numChildren = node->num_children;

    int numTerms = ((numChildren - 3) / 2) + 2;

    Register * doNotSpill[] = {NULL, NULL, NULL, NULL, NULL};
    Register * resultReg = allocate_register(gen, node, 0, doNotSpill, 0, -1);


 


    fprintf(stdoutFP,"  ADDI %s, x0, 0\n", resultReg->name);  //result set to zero, it keeps track of all false results


    for(int i = 0; i < numTerms - 1; i++){
  
        int firstIndex = i * 2;
        int secondIndex = firstIndex + 2;


        AST_Node * firstNode = node->children[firstIndex];
        AST_Node * secondNode = node->children[secondIndex];

        Symbol comparisonSymbol = node->children[firstIndex + 1]->symbol;


        generateCode(gen, firstNode);
        generateCode(gen, secondNode);

        
        Register * leftUpper, * leftLower, * rightUpper, * rightLower , * tempReg;

        leftUpper = get_register(gen, firstNode, 0, doNotSpill, 0);
        doNotSpill[0] = leftUpper;
        rightUpper = get_register(gen, secondNode, 0, doNotSpill, 1);
        doNotSpill[1] = rightUpper;

        if(firstNode->resultType == TYPE_INTEGER) leftLower = &registerX0;
        else if(firstNode->resultType == TYPE_DECIMAL) leftLower = get_register(gen, firstNode, 1, doNotSpill, 2);
        doNotSpill[2] = leftLower;

        if(secondNode->resultType == TYPE_INTEGER) rightLower = &registerX0;
        else if(secondNode->resultType == TYPE_DECIMAL) rightLower = get_register(gen, secondNode, 1, doNotSpill, 3);
        doNotSpill[3] = rightLower;

        tempReg = allocate_register(gen, NULL, 0, doNotSpill, 4, -1);
        doNotSpill[4] = tempReg;

        char * regIn[] = {leftUpper->name, leftLower->name, rightUpper->name, rightLower->name};
        char * regOut[] = {tempReg->name};


        if(compareSymbols(&comparisonSymbol, &LessSymbol)){

            callFunction(gen, "LT2Word", regIn, 4, regOut, 1);

        }
        if(compareSymbols(&comparisonSymbol, &LessEqualSymbol)){

            callFunction(gen, "LTE2Word", regIn, 4, regOut, 1);

        }

        if(compareSymbols(&comparisonSymbol, &GreaterSymbol)){

            callFunction(gen, "LTE2Word", regIn, 4, regOut, 1);
            fprintf(stdoutFP,"  XORI %s, %s, 1\n", tempReg->name, tempReg->name);

        }

        if(compareSymbols(&comparisonSymbol, &GreaterEqualSymbol)){

            callFunction(gen, "LT2Word", regIn, 4, regOut, 1);
            fprintf(stdoutFP,"  XORI %s, %s, 1\n", tempReg->name, tempReg->name);

        }



        resultReg = get_register(gen, node, 0, doNotSpill, 5);
        

        fprintf(stdoutFP,"  XORI %s, %s ,1\n", tempReg->name, tempReg->name);
        fprintf(stdoutFP,"  ADD %s, %s, %s\n", resultReg->name, resultReg->name, tempReg->name);

    
        free_register(gen, tempReg);
        free_register(gen, leftLower);
        free_register(gen, leftUpper);
        free_register(gen,rightLower);
        free_register(gen, rightUpper);
    }



    resultReg = get_register(gen, node, 0, doNotSpill, 0);
    //if resultReg = 0, then its true

    fprintf(stdoutFP,"  SLTIU %s, %s, 1\n", resultReg->name, resultReg->name);



}















void _printVariableMapKey(void * a){
    char * string = (char *)a;
    printf("%s ", string);
}

void _printVariableMapItem(void * b){
    VariableTableEntry entry = *(VariableTableEntry*)b;
    printf("Variable is type: ");

    if(entry.type == TYPE_INTEGER) printf("integer. ");
    if(entry.type == TYPE_DECIMAL) printf("decimal. ");
    if(entry.type == TYPE_STRING) printf("string. ");
    if(entry.type == TYPE_BOOLEAN) printf("boolean. ");

    printf("Memory Location = %d", entry.memoryLocation);
}

void printVariableMap(CodeGenerator * gen){


    Map * variableMap = gen->variableMap;
    VariableTable * tab = pop(gen->variableTableStack);
    variableMap = tab->variables;
    
    printMap(variableMap, _printVariableMapKey, _printVariableMapItem);
    
    
}

FixedPoint float_to_fixed_point(float val) {
    // Separate the whole and fractional parts
    int32_t whole = (int32_t)val;
    float fractional = val - whole;

    // Scale the fractional part by 2^32
    uint64_t scaled_fractional = (uint64_t)(fractional * ((uint64_t)1 << 32));

    FixedPoint fixedPoint;
    fixedPoint.whole = whole;
    fixedPoint.fractional = (uint32_t)scaled_fractional;

    printf("in float to fixed point. val = %f, whole = %d, fractional = %u\n", val, whole, fixedPoint.fractional);

    return fixedPoint;
}

float fixed_to_float(FixedPoint val) {
    float whole = (float)val.whole;
    float fractional = ((float)val.fractional) / ((uint64_t)1 << 32);

    return whole + fractional;
}

FixedPoint double_to_fixed_point(double val){
    printf("in double to fixed point\n");

    FixedPoint result;

    int32_t whole = (int32_t) val;

    double fractional = val - whole;
    printf("whole = %d, fractional = %lf\n", whole, fractional);

    uint32_t resultFractional = (uint32_t)(fractional * ((uint64_t)1 << 32));
    printf("resultFractional = %u\n", resultFractional);

    result.whole = whole;
    result.fractional = resultFractional;

 printf("in double to fixed point. val = %lf, whole = %d, fractional = %u\n", val, result.whole, result.fractional);

    return result;
}

double fixed_to_double(FixedPoint val) {
    double whole = (double)val.whole;
    double fractional = ((double)val.fractional) / ((uint64_t)1 << 32);

    return whole + fractional;
}






void  generateCode(CodeGenerator * gen, AST_Node * node){
  //  printf("generating code for id = %d\n", node->id);

    Symbol sym = node->symbol;
    int numChildren = node->num_children;
    Register* doNotSpill[2] = {NULL, NULL};



    //Statements
    if(compareSymbols(&sym, &StatementsSymbol)){
        statementsNode(gen, node);
    }

    //If node is Number_Integer
    if(compareSymbols(&sym, &NumberIntegerSymbol)){

        //printf("generating assembly for number integer\n");
     
        int * valPtr = (int *)sym.value;
        int val = *valPtr;
        Register * reg = allocate_register(gen, node, 0, NULL, 0, val);

        load_int(gen, val, reg);
       
        //printf("  addi %s, x0, %d\n", reg->name, val);
        return;
    }


    if(compareSymbols(&sym, &TrueSymbol)){
        Register * reg = allocate_register(gen, node, 0, NULL, 0, -1);
        load_int(gen, 1, reg);
    }

    if(compareSymbols(&sym, &FalseSymbol)){
        Register * reg = allocate_register(gen, node, 0, NULL, 0, -1);
        load_int(gen, 0, reg);
    }

    //string
    if(compareSymbols(&sym, &StringSymbol))stringNode(gen, node);
      

    if(compareSymbols(&sym, &NumberFloatSymbol)){

       // printf("made it into numberFloat\n");
        double * valPtr = (double*)sym.value;
        double val = *valPtr;

        int intVal = *(int*)sym.value;
        FixedPoint fixed = double_to_fixed_point(val);

        Register * reg = allocate_register(gen, node, 0, NULL, 0, fixed.whole);
        doNotSpill[0] = reg;
        
        Register * reg2 = allocate_register(gen, node, 1, doNotSpill, 1, fixed.fractional);

        load_int(gen, fixed.whole, reg);
        load_unsigned_int(gen, fixed.fractional, reg2);  

        return;
    }

    if(compareSymbols(&sym, &IdentifierSymbol)) identifierNode(gen, node);

    if (compareSymbols(&sym, &MultiplicativeExpressionSymbol)){
        
        Symbol operation = node->children[1]->symbol;
        if(compareSymbols(&operation, &StarSymbol)) multiplyNode(gen, node);
        if(compareSymbols(&operation, &SlashSymbol)) divideNode(gen, node);
    }

 
    //Additive Expression
    if (compareSymbols(&sym, &AdditiveExpressionSymbol)){
        addNode(gen, node);
    }
    

    //Initialized Variable Declaration
    if(compareSymbols(&sym, &InitializedVariableDeclarationSymbol)){
        initializedDeclarationNode(gen, node);
    }
    
    if(compareSymbols(&sym, &VariableDeclarationSymbol))declarationNode(gen, node);

    if(compareSymbols(&sym, &FunctionDefinitionSymbol)) functionDeclarationNode(gen, node);

    if(compareSymbols(&sym, &ReturnStatementSymbol)) returnNode(gen, node);

    if(compareSymbols(&sym, &FunctionCallSymbol)) functionCallNode(gen, node);

    if(compareSymbols(&sym, &PrintStatementSymbol)) printStatementNode(gen, node);

    if(compareSymbols(&sym, &OrExpressionSymbol)) orExpressionNode(gen, node);

    if(compareSymbols(&sym, &AndExpressionSymbol)) andExpressionNode(gen, node);

    if(compareSymbols(&sym, &NotExpressionSymbol)) notExpressionNode(gen, node);

    if(compareSymbols(&sym, &IfStatementSymbol)) ifStatementNode(gen, node);

    if(compareSymbols(&sym, &IfElseStatementSymbol)) ifElseStatementNode(gen, node);

    if(compareSymbols(&sym, &variableAssignmentSymbol)) assignmentNode(gen, node);

    if(compareSymbols(&sym, &UnaryNegationExpressionSymbol)) unaryNegationNode(gen, node);

    if(compareSymbols(&sym, &WhileStatementSymbol)) whileLoopNode(gen, node);

    if(compareSymbols(&sym, &RelationalExpressionSymbol)) relationalExpressionNode(gen, node);
}




//compare functions for maps
bool compareInts(void * a, void * b){

    int first = *(int *)a;
    int sec = *(int *)b;

    return first == sec;
}

CodeGenerator * createCodeGenerator(){
    CodeGenerator * gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    Map * locationMap = createMap(compareInts);
    Map * variableMap = createMap(compareStrings);

    gen->nodeLocationMap = locationMap;
    gen->variableMap = variableMap;
    gen->stack_offset = 0;
    gen->next_label = 0;
    gen->labels = (char **) malloc(sizeof(char*));


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




    //create variable table stack
    Stack * tableStack = createStack();
    Map * initialVariableTable = createMap(compareStrings);
    Map * functionTable =createMap(compareStrings);

    push(tableStack, initialVariableTable);


    gen->variableTableStack = tableStack;
    gen->functionTable = functionTable;

    



    return gen;

}

