


#include "Lexer2.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



//all the following lists must match order in CFG.h

char *keywords[] = {

    "Integer", "Decimal", "String", "Boolean",  "Is", "Define", "As", "Result",
    "Print", "True", "False", "Or", "And", "Not", "If", "Then", "Else", "While", "Until",
};

int numKeywords = 19;

char  * operators[] = {
    "+", "-", "*", "/", "=", ">", "<", ">=", "<=",
};

int numOperators= 9;

char *  punctuation[] = {
    ".", "{", "}", "(" ,")", "\"",
};

int numPunctuation = 6;









Symbol read_alphabetical(const char* input, int* position) {
   
    printf("in read alphabetical\n");
    int start = *position;
    while (isalnum(input[*position]) || input[*position] == '_') {
        (*position)++;

    }

    int length = *position - start;
    char* value = malloc(length + 1);
    strncpy(value, input + start, length);
    value[length] = '\0';




    // Check if the word matches any control keywords.
    for (int i = 0; i < numKeywords; i++) {
        if (strcmp(value, keywords[i]) == 0) {

            Symbol sym = {.isTerminal = true, .terminal = i + KEYWORD_START_INDEX};
            return sym;
        }
    } 



    //otherwise its an identifier  
    Symbol sym = {.isTerminal = true, .terminal = Symbol_Identifier};
    printf("value for alphabetical is %s\n", value);
    sym.value = value;
    return sym;

}


Symbol read_nonAlphaNumeric(const char* input, int* position) {
    printf(" in read nonalphanumeric\n");

    if (input[*position] == '\0') {
        printf("End of input in read_nonAlphaNumeric.\n");
        // Return an end-of-input symbol or handle as appropriate.
    }

    char value[3] = {0}; // Assuming that no operator or punctuation is more than 2 characters long
    value[0] = input[*position];

    // Check for 2-character operators or punctuation
    if (input[*position + 1] != '\0') { // Make sure not to read past the end
        value[1] = input[*position + 1];
        for (int i = 0; i < numOperators; i++) {
            if (strcmp(value, operators[i]) == 0) {
                (*position) += 2;
                Symbol sym = {.isTerminal = true, .terminal = i + OPERATOR_START_INDEX};
                return sym;
            }
        }
        for (int i = 0; i < numPunctuation; i++) {
            if (strcmp(value, punctuation[i]) == 0) {
                (*position) += 2;
                Symbol sym = {.isTerminal = true, .terminal = i + PUNCTUATION_START_INDEX};
                return sym;
            }
        }
    }

    // Check for 1-character operators or punctuation
    value[1] = '\0';
    for (int i = 0; i < numOperators; i++) {
        if (strcmp(value, operators[i]) == 0) {
            (*position)++;
            Symbol sym = {.isTerminal = true, .terminal = i + OPERATOR_START_INDEX};
            return sym;
        }
    }
    for (int i = 0; i < numPunctuation; i++) {
        if (strcmp(value, punctuation[i]) == 0) {
            (*position)++;
            Symbol sym = {.isTerminal = true, .terminal = i + PUNCTUATION_START_INDEX};
            return sym;
        }
    }

    printf("unexpected symbol in read_nonAlphaNumeric. value = %s\n", value);
    // Return an error symbol or handle the error as appropriate.
}

/*
Symbol read_nonAlphaNumeric(const char* input, int* position) {
    printf(" in read nonalphanumeric\n");
    int start = *position;
    while (!isalnum(input[*position]) && input[*position] != '\0' && input[*position] != '_' && input[*position] != ' ') {
        (*position)++;
    }

    int length = *position - start ;
    char* value = malloc(length + 1);
    strncpy(value, input + start, length);
    value[length] = '\0';


    printf("reading nonalpha: %s   , len = %d\n", value, length);

    // Check if the word matches any operators.
    for (int i = 0; i < numOperators; i++) {
        if (strcmp(value, operators[i]) == 0) {
            free(value);
            Symbol sym = {.isTerminal = true, .terminal = i + OPERATOR_START_INDEX};
            return sym;
        }
    }

    // Check if the word matches any punctuation.
    for (int i = 0; i < numPunctuation; i++) {
        if (strcmp(value, punctuation[i]) == 0) {
            free(value);
            Symbol sym = {.isTerminal = true, .terminal = i + PUNCTUATION_START_INDEX};
            return sym;
        }
    }

    free(value);
    printf("unexpected symbol in read_nonAlphaNumeric. value = %s\n", value);
    // Return an error symbol or handle the error as appropriate.
}
*/

Symbol read_number(const char * input, int * position){
    printf("in read number\n");
    Symbol sym;

    if (isdigit(input[*position])) {
        int start = *position;
        while (isdigit(input[*position])) {
            (*position)++;
        }

        if (input[*position] == '.' && isdigit(input[*position + 1])) {
            (*position)++;
            while (isdigit(input[*position])) {
                (*position)++;
            }

            int length = *position - start;
            char* value = malloc(length + 1);
            strncpy(value, input + start, length);
            value[length] = '\0';

            

            sym = (Symbol){.isTerminal = true, .terminal = Symbol_Number_Float};
            double numVal = atof(value);
            
            double * ptr = (double *)malloc(sizeof(float));
            *ptr = numVal;
            sym.value = ptr;

        } else {


            int length = *position - start;
            char* value = malloc(length + 1);
            strncpy(value, input + start, length);
            value[length] = '\0';

            sym = (Symbol){.isTerminal = true, .terminal = Symbol_Number_Integer};
            
            int numVal = atoi(value);
            int * ptr = (int*)malloc(sizeof(int));
            *ptr = numVal;
            sym.value = ptr;

        }
    }

    return sym;
}




Symbol read_string(const char * input, int * position) {
    printf("in read string\n");
    Symbol sym;

    if (input[*position] == '"') {
        int start = *position;
        (*position)++;  // Skip the opening quote
        while (input[*position] != '\0' && input[*position] != '"') {
            (*position)++;
        }

        // Check for the missing closing quote
        if (input[*position] == '\0') {
            printf("error missing closing quotes.  \n");
            return InvalidSymbol;
        }

        int length = *position - start - 1;  // Subtract one to not include the opening quote
        char* value = malloc(length + 1);
        strncpy(value, input + start + 1, length);  // Start copying after the opening quote
        value[length] = '\0';

        sym = StringSymbol;
        sym.value = value;

        (*position)++;  // Skip the closing quote
    }

    return sym;
}



Symbol * tokenize(const char * input){

    int position = 0;
    int tokensCount = 0;   
    Symbol * tokens = (Symbol *)malloc(sizeof(Symbol));

    while (input[position] != '\0') {

        char c = input[position];

        // Skip white spaces
        if (isspace(c)) {
            position++;
            continue;
        }

        tokens = (Symbol *)realloc(tokens, (tokensCount + 1) * sizeof(Symbol));
        if(tokens == NULL)printf("why tf is tokens null rn\n");
        Symbol sym;


        //read string
        if(c == '"'){
            sym = read_string(input, &position);
        }

        else if(!isalnum(c)){
            printf("about to read nonalpha pos = %d\n", position);
            sym = read_nonAlphaNumeric(input, &position);

        }

        else if(isdigit(c)){
            sym = read_number(input, &position);

        }

        else if(isalpha(c)){
            sym = read_alphabetical(input, &position);
            printf("read alphabetical and the symbol is: ");
            printSymbol(sym);
            printf("\n");
        }


        else{
            //character is invalid
            printf("invalid character: %c\n", c);
            sym = InvalidSymbol;
            position++;
        }

        tokens[tokensCount] = sym;
        tokensCount++;

    
    }

    // Add end token

    tokens = realloc(tokens, (tokensCount + 1) * sizeof(Symbol));
    tokens[tokensCount] = EndSymbol;
    return tokens;

}



