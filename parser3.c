#include "Parser.h"
#include "AST.h"
#include "DataStructures/Queue.h"
#include "Assembly.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Lexer2.h"
#include "ASTWalker.h"
#include <omp.h>

typedef enum {
    LR_STACK_STATE,
    LR_STACK_NODE,
} LRStackItemType;

typedef struct {
    LRStackItemType type;
    union {
        int state;
        AST_Node * node;
    };

} LRStackItem;




int symbolToInt(Symbol symbol, LRParser * parser){


    if(compareSymbols(&symbol, &FunctionKeywordSymbol)){

        if(symbol.value == NULL)return symbol.terminal;



        //find index
        for(int i = 0; i < parser->numFunctionKeywords; i++){
            if(strcmp(symbol.value, parser->functionKeywords[i]) == 0){
                    return NUM_NON_TERMINALS + NUM_TERMINALS  + i;

            }
        }

        printf("big fucking problem\n");
    }


    if(symbol.isHelper){
        return symbol.nonTerminal + NUM_NON_TERMINALS + NUM_TERMINALS + parser->numFunctionKeywords;
    }

    return symbol.isTerminal ? symbol.terminal : symbol.nonTerminal + NUM_TERMINALS;
}

LRParser * createParser(LRTable * table){

    printf("in create parser\n");

    LRParser * parser = (LRParser*)malloc(sizeof(LRParser));
    Stack * stack = (Stack*)malloc(sizeof(Stack));

//initialize stack
    LRStackItem* item = (LRStackItem*)malloc(sizeof(LRStackItem));

    printf("in createparser 1\n");
    item->type = LR_STACK_STATE;
    item->state = 0;
    push(stack, item);
    parser->stack = stack;

    parser->currentState = 0;
    parser->table = table;


    parser->currentNodeId = 0;
    parser->numFunctionKeywords = 0;



    parser->newRules = NULL;
    parser->numNewRules = 0;
    parser->helperCount = 0;
    parser->currentFunctionDefinitionCount = 0;
    parser->totalFunctionDefinitionCount = 0;
    parser->numOriginalRules = 100;


    printf("made it this far\n");

    parser->originalRules = createRules2(&(parser->numOriginalRules));
    

    printf("made parser\n");
    return parser;

    
}







GrammarItem* createGrammarItem(Rule rule, int dot_position, Symbol lookahead) {
    GrammarItem* grammarItem = (GrammarItem*)malloc(sizeof(GrammarItem));


    grammarItem->rule = rule;
    grammarItem->dot_position = dot_position;
    grammarItem->lookAhead = lookahead;

    return grammarItem;
}


bool compareGrammarItems(void *a, void *b) {
    GrammarItem *item1 = (GrammarItem*)a;
    GrammarItem *item2 = (GrammarItem*)b;
    
    if (item1->dot_position != item2->dot_position) {
        return false;
    }

    if(!compareSymbolsAndVal(&item1->lookAhead, &item2->lookAhead))return false;


    return compareRules(&(item1->rule), &(item2->rule));
}

bool compareStates(void *a, void *b) {
    State *state1 = (State*)a;
    State *state2 = (State*)b;
    
    return compareSets(state1->items, state2->items);
}

LRTable* createLRTable(int numStates, int numTerminals, int numNonTerminals) {
    printf("creating LR Table num states = %d, num term = %d, nonterm = %d\n", numStates, numTerminals, numNonTerminals);
    LRTable* table = malloc(sizeof(LRTable));
    table->numStates = numStates;
    table->numTerminals = numTerminals;
    table->numNonTerminals = numNonTerminals;


    printf("about to make the action table\n");
    // Initialize actionTable with ERROR actions
    table->actionTable = malloc(sizeof(Action*) * (numNonTerminals + numTerminals));
    for (int i = 0; i < (numNonTerminals + numTerminals); i++) {
        table->actionTable[i] = malloc(sizeof(Action) * (numStates));
        for (int j = 0; j < numStates; j++) {
            Action action = (Action){ACTION_ERROR, -1};
            table->actionTable[i][j] = action;
        }
    }


    printf("about to make the goto table\n");
    // Initialize gotoTable with -1, indicating no transition
    table->gotoTable = malloc(sizeof(int*) * (numNonTerminals + numTerminals));
    for (int i = 0; i < (numNonTerminals + numTerminals); i++) {
        table->gotoTable[i] = malloc(sizeof(int) * (numStates));
        for (int j = 0; j < numStates; j++) {
            table->gotoTable[i][j] = -1;
        }
    }

    return table;
}


State* createState(Set* items, int id) {
    State* state = (State*) malloc(sizeof(State));
    state->items = items;
    state->id = id;
    return state;
}


void printActionTable(LRTable* table) {
    printf("ACTION Table:\n");

    // Print column headers (terminals)
    printf("%-4s", "");
    for (int i = 0; i < table->numStates; i++) {
        printf("%-4d", i);
    }
    printf("\n");

    // Print horizontal line for table
    for (int i = 0; i <= table->numStates; i++) {
        printf("----");
    }
    printf("\n");

    // Print row headers (states) and corresponding entries in the ACTION table
    for (int i = 0; i < (table->numTerminals + table->numNonTerminals); i++) {
        printf("%-4d|", i);
        for (int j = 0; j < (table->numStates); j++) {
            Action action = table->actionTable[i][j];
            switch (action.type) {
                case ACTION_SHIFT:
                    printf("S%-3d", action.value);
                    break;
                case ACTION_REDUCE:
                    printf("R%-3d", action.value);
                    break;
                case ACTION_ACCEPT:
                    printf("%-4s", "A");
                    break;
                default:
                    printf("%-4s", "E");
            }
        }
        printf("\n");
    }
}







void _printRule(Rule rule, int dot_position) {
    printf("LHS: ");
    printSymbol(rule.lhs);
    printf("\n");

    printf("RHS: ");
    for (int i = 0; i < rule.rhs_len; i++) {
        if (i == dot_position) {
            printf("     .      ");
        }
        printSymbol(rule.rhs[i]);
    }

    // Print the dot at the end if the dot position is at the end of RHS
    if (dot_position == rule.rhs_len) {
        printf("     .      ");
    }

    printf("\n\n");
}

void printRule(Rule rule){
    _printRule(rule, -1);
}


void printGOTOTable(LRTable* table) {
    printf("GOTO Table:\n");

    // Print column headers (states)
    printf("%-10s", "");
    for (int i = 0; i < table->numStates; i++) {
        printf("%-10d", i);
    }
    printf("\n");

    // Print row headers (symbols) and corresponding entries in the GOTO table
    for (int i = 0; i < table->numTerminals + table->numNonTerminals; i++) {
        printf("%-10d", i); 
        for (int j = 0; j < table->numStates; j++) {
            printf("%-10d", table->gotoTable[i][j]);
        }
        printf("\n");
    }
}


void printGrammarItem(GrammarItem item){
    _printRule(item.rule,  item.dot_position );
    printf("lookahead = ");
    printSymbol(item.lookAhead);
    printf("\n\n");
}

void printState(State state) {
    
    printf("State ID: %d\n", state.id);
    printf("%d Items:\n", state.items->size);
    SetNode* currentNode = state.items->head;
    while (currentNode != NULL) {
        GrammarItem* item = (GrammarItem*)currentNode->data;
        printGrammarItem(*item);
        currentNode = currentNode->next;
    }
    printf("\n");
}




Set* calculateFirst(Symbol* symbol, Rule* rules, int rulesCount) {
    // Create a set to store the first set
  
  
    Set* firstSet = createSet(compareSymbolsAndVal);
           
    // If the symbol is a terminal, add it to the first set and return
    if (symbol->isTerminal) {
        addToSet(firstSet, symbol);
    
        return firstSet;
    }

            

    // If the symbol is a non-terminal, find all the rules with the non-terminal symbol on the LHS
    for (int i = 0; i < rulesCount; i++) {

          
        if (compareSymbolsAndVal(&rules[i].lhs, symbol)){
            // Calculate the first set of the first symbol on the RHS and union it with the current first set
          
           if(compareSymbolsAndVal(&rules[i].lhs, &rules[i].rhs[0])) continue;

            Set* tempFirstSet = calculateFirst(&rules[i].rhs[0], rules, rulesCount);
            Set* oldFirstSet = firstSet;
            firstSet = unionSet(oldFirstSet, tempFirstSet);
   
        }
    }




    

    

    return firstSet;
}

Set* calculateFollow(Symbol symbol, Rule* rules, int rulesCount) {
    // Create a set to store the follow set
    Set* followSet = createSet(compareSymbolsAndVal);

    // If the symbol is the start symbol, add the end symbol to the follow set
    if (compareSymbols(&symbol, &StartSymbol)) {

        addToSet(followSet, &EndSymbol);
    }

    // Loop through all rules
    for (int i = 0; i < rulesCount; i++) {
        Rule rule = rules[i];
        // Loop through the RHS of each rule
        for (int j = 0; j < rule.rhs_len; j++) {
            // If the symbol is found in the RHS of the rule
            //if (!rule.rhs[j].isTerminal && rule.rhs[j].nonTerminal == symbol.nonTerminal) {
            Symbol rhs = rule.rhs[j];
            if(!rhs.isTerminal && compareSymbolsAndVal(&rhs, &symbol )){
                
                // If symbol is not the last symbol in the RHS
                if (j < rule.rhs_len - 1) {
                    // Add FIRST set of next symbol in the RHS to the FOLLOW set of symbol
                    Set* firstSet = calculateFirst(&rule.rhs[j + 1], rules, rulesCount);
                    Set* oldFollowSet = followSet;
                    followSet = unionSet(followSet, firstSet);
                }
                // If symbol is the last symbol in the RHS
                else {
                    // Add FOLLOW set of the LHS symbol of the rule to the FOLLOW set of symbol
                    Set* lhsFollowSet = calculateFollow(rule.lhs, rules, rulesCount);
                    Set* oldFollowSet = followSet;
                    followSet = unionSet(followSet, lhsFollowSet);
                }
            }
        }
    }

    return followSet;
}


Set * getLookAheadSet(GrammarItem item, Rule * rules, int numRules){

  
    
    Set * lookaheadSet = createSet(compareSymbolsAndVal);

    
    int symbolIndex = item.dot_position + 1;

    //if there is no symbol after what follows the dot
    if(symbolIndex >= item.rule.rhs_len){
        Symbol lookahead = item.lookAhead;
        Symbol * ptr = (Symbol *)malloc(sizeof(Symbol));
        *ptr = lookahead;
        addToSet(lookaheadSet, ptr);
        return lookaheadSet;
    }


    //else calculate the FIRST Set
    else {

        Symbol sym = item.rule.rhs[symbolIndex];
        Symbol * symPtr = (Symbol *)malloc(sizeof(Symbol));
        *symPtr = sym;
        
        Set * firstSet = calculateFirst(symPtr, rules, numRules);

        return firstSet;
    }

}




Set* getClosure(Set *set, Rule *rules, int num_rules) {

  

    
    // Create a new set and copy all items from the input set
    Set* closureSet = createSet(compareGrammarItems);
    SetNode *current = set->head;

    while (current != NULL) {
        GrammarItem * item = (GrammarItem *)current->data;
      addToSet(closureSet, item);
     /*   printf("adding initial items :\n");
        printGrammarItem(*item);
    */
        current = current->next;
  
    }

  

    

    // Iterate over all items in the new set
    bool added;
    do {

        added = false;
        current = closureSet->head;

        while (current != NULL) {
            GrammarItem * ptr = (GrammarItem*)current->data;
            GrammarItem item = *ptr;

            // If the dot is before a nonterminal, add all rules with that nonterminal on the LHS
            if (item.dot_position < item.rule.rhs_len && !item.rule.rhs[item.dot_position].isTerminal) {
                Symbol nonTerminalSymbol = item.rule.rhs[item.dot_position];
                Symbol lookaheadSymbol = item.lookAhead;



                Set * lookaheadSet = getLookAheadSet(item, rules, num_rules);

                // Make sure the lhs symbol is a NonTerminal before comparing
                for (int i = 0; i < num_rules; i++) 
                {
                    Symbol lhs = rules[i].lhs;
                    //if (!rules[i].lhs.isTerminal && rules[i].lhs.nonTerminal == nonTerminalSymbol.nonTerminal) {
                    if (!lhs.isTerminal && compareSymbolsAndVal(&lhs, &nonTerminalSymbol)){  
                        

                        // for each terminal in lookahead Set
                        SetNode * node = lookaheadSet->head;
                   
                        while(node != NULL){

                            Symbol * symPtr = (Symbol *)node->data;

                            
                            

                          


                            GrammarItem * newItem = createGrammarItem(rules[i], 0, *symPtr);
                            if (!setContains(closureSet, newItem)) {
                                addToSet(closureSet, newItem);
                                added = true;
                        }



                        node = node->next;
                        }



            
                    }
                } 
            }

           
            current = current->next;
        }
    
    
    } while(added);

   
    return closureSet;
}



State * getStateByID(Set * set, int id){

    if(id >= set->size)return NULL;
    

    
    SetNode * node = set->head;

    while(node != NULL){

        State * state = (State *)node->data;
        if(state->id == id)return state;
        node = node->next;
    }

    return NULL;
}


int getIdByState(Set * set, State inState){

    SetNode * node = set->head;
    
    while(node != NULL){

        State * state = (State *)node->data;
        bool comp = compareStates(state, &inState);
        
        if(comp) return state->id;
        node = node->next;
    }

    return -1;
}


State* GOTO(State* state, Symbol* symbol, Rule* rules, int rulesCount) {
   
  


    // Create a new set of grammar items for the new state
    Set* newStateItems = createSet(compareGrammarItems);
    
    // Iterate through all grammar items in the current state
    SetNode *current = state->items->head;
    while (current != NULL) {
        GrammarItem *item = (GrammarItem*)(current->data);



        // If the dot is before the given symbol, create a new grammar item with the dot moved one position to the right
        if (item->dot_position < item->rule.rhs_len &&
            compareSymbolsAndVal(&(item->rule.rhs[item->dot_position]), symbol)) {


            GrammarItem *newItem = createGrammarItem(item->rule, item->dot_position + 1, item->lookAhead);
            addToSet(newStateItems, newItem);
        }

        current = current->next;
    }

    // Create a new state with the new set of grammar items
    State* newState = createState(newStateItems, -1);  // Set ID as -1 for now, to be updated later

    // Compute the closure of the new state


   
   
    Set* closure = getClosure(newState->items, rules, rulesCount);

    // Replace the items of the new state with its closure
    
    newState->items = closure;

 



    if(newState->items->size <= 0){
        

       // if(functionKeyword)printf("returning null\n");
        return NULL;}

  

 


    int stateID = state->id;

  //  if(functionKeyword)printf("returning state id = %d\n", stateID);
    return newState;
}

Set* generateCanonicalCollectionAndFillGOTOTable(LRParser * parser) {
    
    printf("in generate canonical\n");

    LRTable * table = parser->table;

    Rule * rules = parser->finalRules;
    int num_rules = parser->numNewRules + parser->numOriginalRules;


    // Initialize the set of states
    Set* states = createSet(compareStates);

    // Initialize the queue of states to be processed
    Queue* queue = createQueue();

    // Create the start state
    GrammarItem* startItem = createGrammarItem(rules[0], 0, EndSymbol);
   
    Set* startItems = createSet(compareGrammarItems);
    addToSet(startItems, startItem);

    Set * closure = getClosure(startItems, rules, num_rules);

  

    State* startState = createState(getClosure(startItems, rules, num_rules), 0);

 
   
    // Add the start state to the set of states and the queue
    addToSet(states, startState);
    enqueue(queue, startState);

    // While there are still states to be processed
    while (!isQueueEmpty(queue)) {
        // Dequeue the next state
        State* state = dequeue(queue);
        omp_set_num_threads(4);
        
        #pragma omp parallel
        {

            #pragma omp sections
            {

            

                #pragma omp section
                {
        // Process each terminal symbol
        for (int i = 0; i < NUM_TERMINALS; i++) {

          //  printf("terminal i = %d\n", i);
            Symbol symbol = createSymbol(true, i);
            // Generate the next state using GOTO function
            State* nextState = GOTO(state, &symbol, rules, num_rules);



            if(nextState != NULL){

                if(setContains(states,nextState)){

                    int nextID = getIdByState(states, *nextState);
                    table->gotoTable[i][state->id] = nextID;
            
                }


                else{
                    // Add the next state to the set of states and the queue
                    nextState->id = states->size;

                    #pragma omp critical
                    {
                    addToSet(states, nextState);
                    enqueue(queue, nextState);
                    }

                    // Fill the GOTO table
                    table->gotoTable[i][state->id] = nextState->id;
      

                }
            }


        }

                }

                #pragma omp section
                {
        // Process each non-terminal symbol
        for (int i = 0; i < NUM_NON_TERMINALS; i++) {
           // printf("nonterminal i = %d\n", i);

            int j = i + NUM_TERMINALS;
            Symbol symbol = createSymbol(false, i);
            // Generate the next state using GOTO function
            State* nextState = GOTO(state, &symbol, rules, num_rules);

            // If the next state is not null and it's not already in the set of states
            if(nextState != NULL){

                if(setContains(states,nextState)){

                    int nextID = getIdByState(states, *nextState);
                  
                    
                    table->gotoTable[j][state->id] = nextID;

                }

                else{
                    nextState->id = states->size;

                    #pragma omp critical
                    {
                    addToSet(states, nextState);
                    enqueue(queue, nextState);
                    }

                    // Fill the GOTO table
                    // The non-terminals entries start after the terminal ones
                    table->gotoTable[j][state->id] = nextState->id;


                }
            }

        }
                }

                #pragma omp section
                {
        //Process each FunctionKeyword Symbol
        for(int i = 0; i < parser->numFunctionKeywords; i++){
                        //printf("funckeyword i = %d\n", i);

            char * word = parser->functionKeywords[i];
           int j = i + NUM_TERMINALS + NUM_NON_TERMINALS;


           Symbol symbol = FunctionKeywordSymbol;
           symbol.value = word;
           symbol.functionKeywordIndex = i;
           //printf(" about to hit GOTO, processing funcgionKeyword index = %d\n", i);
           State * nextState = GOTO(state, &symbol, rules, num_rules); 

            // If the next state is not null and it's not already in the set of states
            if(nextState != NULL){

                if(setContains(states,nextState)){

                    int nextID = getIdByState(states, *nextState);
                  
                    
                    table->gotoTable[j][state->id] = nextID;

                }

                else{
                    nextState->id = states->size;

                    #pragma omp critical
                    {
                    addToSet(states, nextState);
                    enqueue(queue, nextState);
                    }

                    // Fill the GOTO table
                    // The non-terminals entries start after the terminal ones
                    table->gotoTable[j][state->id] = nextState->id;


                }
            }
        }
                }



                #pragma omp section
                {
        //Process each Helper Symbol
        for(int i = 0; i < parser->helperCount; i++){


            //printf("herlper = %d\n", i);
           Symbol symbol = {.compareValue = false, .isHelper = true, .isTerminal = false, .nonTerminal = i };
            int j = symbolToInt(symbol, parser);

           // printf("\n\n\nPROCESSING EACH HELPER SYMBOL i = %d,  j = %d\n\n\n", i, j);


           State * nextState = GOTO(state, &symbol, rules, num_rules); 
           

           
           

            // If the next state is not null and it's not already in the set of states
            if(nextState != NULL){

                if(setContains(states,nextState)){

                    int nextID = getIdByState(states, *nextState);
                   // printf("next ID = %d", nextID);
                    
                    table->gotoTable[j][state->id] = nextID;

                }

                else{
                    nextState->id = states->size;
                    #pragma omp critical
                    {
                    addToSet(states, nextState);
                    enqueue(queue, nextState);
                    }

                    // Fill the GOTO table
                    // The non-terminals entries start after the terminal ones
                    table->gotoTable[j][state->id] = nextState->id;
                    

                }
            }
        }
    
                }
    
            }//end omp sections
            #pragma omp barrier
    } // end omp parallel

    }

    // Cleanup queue
    // ... (insert your cleanup code here)

    return states;
}

void fillActionTable(LRParser * parser, Set* states) {
   

   printf("in fill action table\n");
    Rule * rules = parser->finalRules;
    int num_rules = parser->numNewRules + parser->numOriginalRules;
    LRTable * table = parser->table;

    // Iterate over each state in the set and add shift
    SetNode* node = states->head;
    while (node != NULL) {

       
        State* state = (State*) node->data;

        // First, process each terminal symbol
        for (int i = 0; i < table->numTerminals; i++) {
      
            // Check if there is a valid transition
            int gotoStateID = table->gotoTable[i][state->id];
            if (gotoStateID != -1) {
                // If so, the action should be SHIFT
                Action action = (Action){ACTION_SHIFT, gotoStateID};
                table->actionTable[i][state->id] = action;

            }}
             

        // Then process each non terminal
        for (int j = 0; j < table->numNonTerminals; j++) {
            int i = j + NUM_TERMINALS;
            
            // Check if there is a valid transition
            int gotoStateID = table->gotoTable[i][state->id];
            if (gotoStateID != -1) {

                // If so, the action should be SHIFT
                Action action = (Action){ACTION_SHIFT, gotoStateID};
                table->actionTable[i][state->id] = action;

            }}


        // Then process each functionKeyword symbol
        for (int j = 0; j < parser->numFunctionKeywords; j++) {
            int i = j + NUM_TERMINALS + NUM_NON_TERMINALS;
            

            // Check if there is a valid transition
            int gotoStateID = table->gotoTable[i][state->id];
            if (gotoStateID != -1) {

                // If so, the action should be SHIFT
                Action action = (Action){ACTION_SHIFT, gotoStateID};
                table->actionTable[i][state->id] = action;

            }}

        
        // Then proces each helper Symbol;
        for (int j = 0; j < parser->helperCount; j++) {
            int i = j + NUM_TERMINALS + NUM_NON_TERMINALS + parser->numFunctionKeywords;
            
            Symbol sym = {.isHelper = true, .isTerminal = false,.nonTerminal = j};

            i = symbolToInt(sym, parser);
            // Check if there is a valid transition
            int gotoStateID = table->gotoTable[i][state->id];
            if (gotoStateID != -1) {
              //  printf("\n\n\nin fill action table. processing helper symbol. i = %d, j = %d, goto state = %d\n\n\n", j, i, gotoStateID);
                // If so, the action should be SHIFT
                Action action = (Action){ACTION_SHIFT, gotoStateID};
                table->actionTable[i][state->id] = action;

            }}


            //Go through each item in the state and See if there can be reduces

            
                
                SetNode * itemNode = state->items->head;
                while(itemNode != NULL){


                    GrammarItem * item = (GrammarItem *)itemNode->data;
                    Symbol lhsSym = item->rule.lhs;
                    Symbol lookaheadSym = item->lookAhead;

                    
                    if(item->dot_position >= item->rule.rhs_len){



                        
                        Symbol left = item->rule.lhs;
                        Symbol lookAhead = item->lookAhead;

                        if(left.isHelper){
                            printf("\n\n\nfound reduction for a helper.");
                            printf("State id = %d, symbol: ", state->id); printSymbol(left); printf("\n");
                            printf("look ahead = :"), printSymbol(lookAhead);printf("\n");
                        }
                        
                     
                        int lookAheadIndex = symbolToInt(lookAhead,parser);
                        Action action = (Action){ACTION_REDUCE, symbolToInt(left, parser)};
                        action.rule = item->rule;

                       /* printf("about to add reduce action to action table. Lookahead = %d = :", lookAheadIndex); printSymbol(lookAhead);printf("\n");
                        if(compareSymbols(&lookAhead, &FunctionKeywordSymbol)){

                            printf("function keyword\n\n\n");
                            printRule(action.rule);
                        }
                        */
                        
                        table->actionTable[lookAheadIndex][state->id] = action;
                       // printf("added to action table\n");
                        
                    }

                    //if its the first rule and the lookahead is the end
                    if(compareSymbols(&StartSymbol, &item->rule.lhs) && compareSymbols(&EndSymbol, &item->lookAhead)){
                        Symbol left = item->rule.lhs;
                        Symbol lookAhead = item->lookAhead;
                        int lookAheadIndex = lookAhead.isTerminal ? lookAhead.terminal : (lookAhead.nonTerminal + NUM_TERMINALS);
                        lookAheadIndex = symbolToInt(lookAhead, parser);
                        Action action = (Action){ACTION_ACCEPT, -1};
                        table->actionTable[lookAheadIndex][state->id] = action;

                    }
                    itemNode = itemNode->next;
                }

            


            node = node->next;

}}


void addNewFunctionRule(LRParser * parser,  Rule newRule){
    int numRules = parser->numNewRules;
  //  printf("in add new funcion rule. Num new rules = %d\n", numRules);
    //if rhs exists, then replace lhs. 
    bool exists = false;
    for(int i = 0; i < numRules; i++){
       // printf("made it in for loop i = %d\n", i);
        Rule rule = parser->newRules[i];
        //printf("in for loop 2\n");

        if(rule.rhs_len == newRule.rhs_len){
            exists = true;
            for(int j = 0; j < newRule.rhs_len; j++){
                Symbol sym1 = rule.rhs[j];
                Symbol sym2 = newRule.rhs[j];
              //  printf("in inner for loop j  = %d\n", j);
              //  printf("about to compare syms.  Sym 1 = ");
              //  printSymbol(rule.rhs[j]);
             //   printf("   Sym 2 = ");
             //   printSymbol(newRule.rhs[j]);
             //   printf("\n");
                if(!compareSymbolsAndVal(&(rule.rhs[j]), &(newRule.rhs[j]))) exists = false;
            }
            }
        
        if(exists){
            parser->newRules[i] = newRule;
           printf("made it out of add new function rule (it exists)\n");

            return;
        }

    }

   // printf("made it past for loop in addNewRule\n");

    //else add append it at the end
    parser->numNewRules++;
    parser->newRules = realloc(parser->newRules, sizeof(Rule) * parser->numNewRules);
    parser->newRules[parser->numNewRules - 1] = newRule;
    printf("made it out of add new function rule (doesnt exist)\n");
}

//function that loops through rules, and replaces two consecutive syms with another sym
void replaceSymbols(Rule * rules, int numRules, Symbol sym1, Symbol sym2, Symbol replaceSym){
    for(int i = 0; i < numRules; i++){
        Rule *rule = &rules[i];
        if(rule->rhs_len < 2) continue;

        for(int j = 0; j < rule->rhs_len - 1; j++){
            Symbol firstSym = rule->rhs[j];
            Symbol secondSym = rule->rhs[j+1];

            if(compareSymbolsAndVal(&sym1, &firstSym) && compareSymbolsAndVal(&sym2, &secondSym)){
                // Replace the first symbol
                rule->rhs[j] = replaceSym;
                // Shift the rest of the symbols down
                for (int k = j + 1; k < rule->rhs_len - 1; k++) {
                    rule->rhs[k] = rule->rhs[k + 1];
                }
                // Decrease the length of the rule
                rule->rhs_len--;
            }
        }
    }
}

bool _checkIfExists(Rule * rules, int numRules, int exception, Symbol sym1, Symbol sym2){

    for(int i = 0; i < numRules && i != exception; i++){
        

        Rule rule = rules[i];
        //scan to through each symbol pair of each rule to see if theres a match
        
        if(rule.rhs_len < 2) continue;

        for(int j = 0; j < rule.rhs_len - 1; j++){

            Symbol firstSym = rule.rhs[j];
            Symbol seconSym = rule.rhs[j + 1];


            if(compareSymbolsAndVal(&firstSym, &sym1) && compareSymbolsAndVal(&seconSym, &sym2)) return true;
        }

    }

    return false;
}

//function that turns CFG into LR(1) parsable CFG
void correctRules(LRParser * parser, bool final){
    //Gonna play it safe and say any two consecutive symbols
    //in two different rules need to be translated into helper symbol
    

    printf("in correct rules\n");
    Rule * rules = parser->newRules;
    int numRules = parser->numNewRules;
    
    if(final){
        rules = parser->finalRules;
        numRules = parser->numNewRules + parser->numOriginalRules;
    }


    for(int i = 0; i < numRules; i++){
        //loop through each rule

        Rule rule = rules[i];
        if(rule.rhs_len < 2) continue;

        int numPairs = rule.rhs_len - 1;
        
        for(int j = 0; j < numPairs; j++){
            //loop through each consecutive symbol pair in rule
            Symbol firstSym = rule.rhs[j];
            Symbol secondSym = rule.rhs[j+1];


            //if the same two consecutive symbols exist in another rule, replace it

            if(_checkIfExists(rules, numRules, i, firstSym, secondSym)){

                //create helper symbol 
                Symbol helperSym = {.compareValue = false, .isHelper = true, .isTerminal = false, .nonTerminal = parser->helperCount++};
                replaceSymbols(rules, numRules, firstSym, secondSym, helperSym);

                //add helper symbol rule
                Symbol * helperRuleRHS = (Symbol *) malloc(sizeof(Symbol) * 2);
                helperRuleRHS[0] = firstSym;
                helperRuleRHS[1] = secondSym;
                Rule helperRule = {.lhs = helperSym, .rhs_len = 2, .rhs = helperRuleRHS};
                addNewFunctionRule(parser, helperRule);

                //repeat until no changes made
                return correctRules(parser, final);
            }

        }

    }
    printf("made it out of correct rules\n");
    printf("here are the new rules: \n");
    for(int i = 0; i < numRules; i++) printRule(parser->newRules[i]); 
}

void processFunctionSignature(LRParser * parser, AST_Node * signature){
    

    printf("in process function signature\n");
    printf("heres the sig before simplifying\n");
    printAST(signature);
    simplifyTree(signature);
 
    Rule rule;
    rule.lhs = FunctionCallSymbol;
    Symbol * ruleRHS = (Symbol *)malloc(sizeof(Symbol) * signature->num_children);

    rule.rhs_len = signature->num_children;


    printf("heres the sig after simplifying:\n");
    printAST(signature);

    for(int i = 0; i < signature->num_children; i++){
        AST_Node * child = signature->children[i];
        Symbol sym = child->symbol;

        //simplifyTree(child);

        //if symbol is keyword (Identifier)
        if(compareSymbols(&sym, &IdentifierSymbol)){
   
            char * word = sym.value;

            printf("IN PROCESS SIGNATURE\n");
            printf("word = %s\n", word);
            

            //add to list of function words (if not already there)
            bool exists = false;
            int index;
            for(int j = 0; j < parser->numFunctionKeywords; j++){
                index = j;
                char * funcWord = parser->functionKeywords[j];
                if(strcmp(word, funcWord) == 0)exists = true;
                
            }
            if(!exists){
                parser->functionKeywords = realloc(parser->functionKeywords, parser->numFunctionKeywords + 1);
                parser->functionKeywords[parser->numFunctionKeywords++] = word;
            }

            //create functionKeyword Symbol and add it to rule
            Symbol keywordSym = {.compareValue = true, .isHelper = false, .isTerminal = true, .terminal = Symbol_FunctionKeyWord, .value = word, .functionKeywordIndex = index};
            ruleRHS[i] = keywordSym;

        }







        //if symbol is Function Argument (Type Identifier)
        if(compareSymbols(&sym, &FunctionArgumentSymbol)){
            Symbol functionCallSym = FunctionCallArgumentSymbol;
            Symbol typeSym = child->children[0]->symbol;

            printf("the type sym = ");
            printSymbol(typeSym);
            printf("\n\n");
            
            if(compareSymbols(&typeSym, &TypeIntegerSymbol)) functionCallSym.value = "Integer";
            else if(compareSymbols(&typeSym, &TypeDecimalSymbol)) functionCallSym.value = "Decimal";
            else printf("\n\n\nuh oh this is bad\n\n\n");
            ruleRHS[i] = functionCallSym;
        }


    }




    rule.rhs = ruleRHS;



    printf("processed the new rule and the rule is:\n");
    printRule(rule);

    addNewFunctionRule(parser, rule);
    correctRules(parser, false);
}




// Shifts a symbol onto the stack and updates the current state
void shift(LRParser* parser, Symbol symbol, int nextState, int symIndex) {

    printf("shifting sym: "); printSymbol(symbol);printf("\n");
    // Push the symbol and the new state onto the stack
    AST_Node * node = createASTNode(symbol);
    node->id = parser->currentNodeId++;
    node->startIndex = symIndex;
    node->endIndex = symIndex;
    
    LRStackItem * nodeItem = (LRStackItem*)malloc(sizeof(LRStackItem));
    LRStackItem * stateItem = (LRStackItem*)malloc(sizeof(LRStackItem));

    nodeItem->type = LR_STACK_NODE;
    stateItem->type = LR_STACK_STATE;

    nodeItem->node = node;
    stateItem->state = nextState;


    push(parser->stack, nodeItem);
    push(parser->stack, stateItem);


    parser->currentState = nextState;

    

}



int reduce(LRParser * parser, Rule  rule, Symbol * inputCopy) {
   
    printf("Reducing Rule = :");
   printRule(rule);
    printf("\n");
    AST_Node * node = createASTNode(rule.lhs);
    node->id = parser->currentNodeId++;

        // Pop off symbols and states from the stack for each symbol on the right-hand side of the rule
    for (int i = 0; i < rule.rhs_len; i++) {
        pop(parser->stack);  // Pop the state

        LRStackItem * item = (LRStackItem*)pop(parser->stack); // Pop the AST node

        if(item->type != LR_STACK_NODE){
        }
        AST_Node* child = item->node;  
        addChildNode(node, child);  // Add the child node to the new node
    }


    reverseChildOrder(node);
    node->startIndex = node->children[0]->startIndex;
    node->endIndex = node->children[node->num_children - 1]->endIndex;

    
    //check if weve reached a functionDefinition, then we need to change the rules of the parser
    
    if(compareSymbols(&node->symbol, &FunctionDefinitionSymbol)){
    

        //replace keyword Symbols with Identifier Symbols in input copy
        int start = node->children[1]->startIndex;
        int end = node->children[1]->endIndex;


        printf("PROCESSING FUNCTION DEFINITION start = %d, end = %d\n", start, end);


        for(int i = start; i <= end; i++){
            
            Symbol inputSym = inputCopy[i];
            if(compareSymbols(&inputSym, &FunctionKeywordSymbol)){

                Symbol identSym = IdentifierSymbol;
                identSym.value = inputSym.value;
                inputCopy[i] = identSym;

            }
        }



        //if this is the first time encountering it, break out and reset
        //otherwise procede normally

       int cur = --(parser->currentFunctionDefinitionCount);
       int tot = parser->totalFunctionDefinitionCount;

       bool firstEncounter = cur < tot;
       if(firstEncounter){

        parser->totalFunctionDefinitionCount -= 1;
        processFunctionSignature(parser, node->children[1]);
        return -1;

       }

        
    }


        // Lookup the next state from the GOTO table

    int peekState = ((LRStackItem*)peek(parser->stack))->state;    
    
    //int nextState = parser->table->gotoTable[rule.lhs.nonTerminal + NUM_TERMINALS][peekState];
    int nextState = parser->table->gotoTable[symbolToInt(rule.lhs, parser)][peekState];



    LRStackItem * nodeStackItem = (LRStackItem*)malloc(sizeof(LRStackItem));
    nodeStackItem->type = LR_STACK_NODE;
    nodeStackItem->node = node;
    
    LRStackItem * stateStackItem = (LRStackItem*)malloc(sizeof(LRStackItem));
    stateStackItem->type = LR_STACK_STATE;
    stateStackItem->state = nextState;



    push(parser->stack, nodeStackItem);
    push(parser->stack, stateStackItem);

    parser->currentState = nextState;

    return 0;

}

void setFinalRules(LRParser* parser) {
    // Allocate memory for final rules
    parser->finalRules = (Rule*)malloc(sizeof(Rule) * (parser->numOriginalRules + parser->numNewRules));

    // Copy original rules to final rules
    for(int i = 0; i < parser->numOriginalRules; i++) {
        parser->finalRules[i] = parser->originalRules[i];
        //printRule(parser->originalRules[i]);
    }

    // Copy new rules to final rules
    for(int i = 0; i < parser->numNewRules; i++) {
        parser->finalRules[parser->numOriginalRules + i] = parser->newRules[i];

        // Print the rule
        //printRule(parser->newRules[i]);
    }

    //correctRules(parser, true);
}


AST_Node * parse(LRParser * parser, Symbol * input, int inputSize, Symbol * inputCopy){







    printf("in parse. Heres the active input:\n");
    for(int i = 0; i < inputSize; i++){
        printf("sym i = %d ", i);
        printSymbol(input[i]);
    }
    printf("thats the active input.\n\n\nHeres the input copy\n");
    for(int i = 0; i < inputSize; i++){
        printf("sym i = %d ", i);
        printSymbol(inputCopy[i]);
    }
    printf("thats the copy input\n\n");


    int i = 0;
    while (i < inputSize){




   
        Symbol inputSymbol = input[i];
        printf("parser i  = %d Symbol: ", i);
        printSymbol(inputSymbol); printf("\n");
        int symbolInt = symbolToInt(inputSymbol, parser);
        printf("symbol int = %d\n", symbolInt);
       // printf("parser current state = %d", parser->currentState);
        Action action = parser->table->actionTable[symbolInt][parser->currentState];
      //  printf("got the action\n");
        
        
        
      //  printf("parsing. Input symbol i = %d, ")
        
        
        
        switch(action.type) {
            case ACTION_SHIFT:

                
             
                shift(parser, inputSymbol, action.value, i);
                i++;
            
                break;

            case ACTION_REDUCE:
                
            
                if(reduce(parser, action.rule, inputCopy) < 0){
                    //hit a function definition, need to start over
                    //return an ast node with id = -1

                    AST_Node * node = createASTNode(EndSymbol);
                    node->id = -1;

                    return node;
               
                }
              
                break;

            case ACTION_ACCEPT:

                pop(parser->stack);
                LRStackItem * item = pop(parser->stack);



                return item->node;
                

            case ACTION_ERROR:
                printf("hit an error in the parser. CurState = %d,  Symbol = ", parser->currentState);
                
                printSymbol(inputSymbol);
                return NULL;

            default:
                printf("should not make it here ever\n");
                return NULL;
        }

    }
}



void copySymbols(Symbol * from, Symbol * to, int numSyms){
    printf("in copySymbols\n");

    for(int i = 0; i < numSyms; i++){


        to[i] = from[i];
    }

    printf("made it out of copy symbols\n");

}

AST_Node * dynamicParse(LRParser * parser, Symbol * input, int inputSize){

    bool finished = false;
    Symbol * activeInput = input;
    Symbol * inputCopy = (Symbol *)malloc(sizeof(Symbol) * inputSize);


    while(1){


        //first go through each symbol, and replace appropriate identifiers with function keywords
        //only after they appear once (not in the function definition)
        // Allocate an array of counters, one for each function keyword


        // Loop through each symbol
        for(int i = 0; i < inputSize; i++){


           /// printf("modifying copy input in for loop i = %d\n", i);

            inputCopy[i] = input[i];

            if(compareSymbols(&input[i], &IdentifierSymbol)){

                for(int j = 0; j < parser->numFunctionKeywords; j++){

                    // If the input value matches a function keyword
                    if(strcmp(parser->functionKeywords[j], input[i].value) == 0){

                      //      printf("\n\n\n\n\n\n\n FOUND A MATCH\n\n\n\n\n\n");

                            Symbol sym = FunctionKeywordSymbol;
                            sym.value = input[i].value;
                            inputCopy[i] = sym;
                        
                    }

                }

            }

           // printf("symbol %d = ", i); printSymbol(input[i]);
        }

  
    printf("modified the copy input\n");

    //initialize stack
    Stack * stack = (Stack*)malloc(sizeof(Stack));

        LRStackItem* item = (LRStackItem*)malloc(sizeof(LRStackItem));
        item->type = LR_STACK_STATE;
        item->state = 0;
        push(stack, item);
        parser->stack = stack;


        int numTerminals = NUM_TERMINALS + parser->numFunctionKeywords;
        int numNonTerminals = NUM_NON_TERMINALS + parser->helperCount + 1;
        int numRules = parser->numOriginalRules + parser->numNewRules;
        

        parser->currentState = 0;
        parser->currentFunctionDefinitionCount = 0;
        printf("in dynamic parse, about to set final rules\n");
        setFinalRules(parser);
        printf("in dynamic parse , finished = %d, finished setting final rules:\n", finished);

        for(int i = 0; i < numRules; i++){
        printRule(parser->finalRules[i]);
             }

        printf("finished printing final rules");


        LRTable * table = createLRTable(numRules * (numTerminals + numNonTerminals), numTerminals, numNonTerminals);
        parser->table = table;

        printf("finished creating LR table\n");

        Set * canonSet = generateCanonicalCollectionAndFillGOTOTable(parser);
        

        printf("canonSet finished\n");


       
        


        fillActionTable(parser, canonSet);
        printf("filled action table\n");
        

        printf("in dynamic parse, about to parse\n");
        AST_Node * result = parse(parser, activeInput, inputSize, inputCopy);
        printf("in dynamic parse, finished parsing\n");

        if(result == NULL){

            printf("hit an invalid parser, finished = %d\n\n", finished);
            if(finished) return NULL;
            finished = true;
 


          
           copySymbols(inputCopy, activeInput, inputSize);
            
        }





        else if(result->id < 0){
            //encounter function definition
            //must add newRules to table and then re parse
            printf("encountered function definition in dynamic parser\n\n\n");
            finished = false;



          copySymbols(inputCopy, activeInput, inputSize);
        }

        else{
            if(finished) return result;
            finished = true;


           copySymbols(inputCopy, activeInput, inputSize);
        }

        } 

    }





FILE *  stdoutFP;


int main(int argc, char** argv) {




       // Check the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    // Get the input and output file names
    char* input_file = argv[1];
    char* output_file = argv[2];

    // Open the input file
    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        perror("Failed to open the input file");
        return 1;
    }

    // Seek to the end of the file to find its size
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // Allocate memory to hold the content of the file plus a null terminator
    char* code = malloc(file_size + 1);
    if (code == NULL) {
        perror("Failed to allocate memory for the code");
        return 1;
    }

    // Read the file into memory
    fread(code, 1, file_size, file);
    code[file_size] = '\0';  // Null-terminate the string

    // Close the input file
    fclose(file);




    // Open the output file
    stdoutFP = fopen(output_file, "w");
    if (file == NULL) {
        perror("Failed to open the output file");
        return 1;
    }

    



    int numRules = 100;
    Rule * rules = createRules2(&numRules);

    LRTable * table = createLRTable(numRules * (NUM_TERMINALS + NUM_NON_TERMINALS), NUM_TERMINALS, NUM_NON_TERMINALS);


    printf("made it past create rules, num rules = %d\n", numRules);

    
    //Set * canonSet = generateCanonicalCollectionAndFillGOTOTable(rules, numRules, table);

   // fillActionTable(canonSet, rules, numRules, table);
    //printActionTable(table);

    printf("creating parser\n");

    LRParser * parser = createParser(table);
    parser->originalRules = rules;
    parser->numOriginalRules= numRules;
    parser->numNewRules = 0;

    printf("created parser\n");


    Symbol * syms = tokenize(code);

    printf("just tokenized\n\n\n");
    //get length of syms
    int i = 0;
    Symbol currentSym = syms[i];
    while(!compareSymbols(&currentSym, &EndSymbol)){
       
        printSymbol(currentSym);
        i++;
        currentSym = syms[i];

        printf("   ");
    }
    printf("\n");


    printf("finished lexing. Num symbols = %d\n", i);


    AST_Node *  result = dynamicParse(parser, syms, i + 1);


    printf("finished parsing the first round                   . Heres the new rules:\n");
    Rule * newRules = parser->newRules;
    for(int i = 0; i < parser->numNewRules; i++){
        printRule(newRules[i]);
    }

    printf("printed new rules\n");




    if(result == NULL){
        printf("invalid input\n");
    }
    else{
        printf("valid input\n");
        printAST(result);

    printf("about to simplify\n");
        simplifyTree(result);
        printAST(result);
        

   
    }


    fprintf(stdoutFP, ".option nopic\n");
    fprintf(stdoutFP, ".attribute arch, \"rv32i2p1_m2p0_a2p1_c2p0\"\n");
    fprintf(stdoutFP, ".attribute unaligned_access, 0\n");
    fprintf(stdoutFP, ".attribute stack_align, 16\n");






    //walk AST
    printf("\n\n about to walk ast\n\n");

    ASTWalker * walker = createASTWalker();
    walkAST(walker, result, NULL);

    printf("successfully walked AST:\n\n");
    printAST(result);

   


    



    CodeGenerator * gen = createCodeGenerator();
    gen->functionTable = walker->functionTable;
    gen->stringTable = walker->stringTable;
    


 
    initialize(gen, walker->sp);

   
   printf("about to generate code\n");

    generateCode(gen, result);

    fprintf(stdoutFP, "jalr x0, ra, 0\n");


    printVariableMap(gen);


    // Close the output file
    fclose(file);

    return 0;



}

