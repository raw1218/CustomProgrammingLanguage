#ifndef LEXER_H
#define LEXER_H

#include "CFG.h"


typedef struct {
    Symbol symbol;
    char * string;
    void * value;
} Token; 





Symbol * tokenize(const char * input);



#endif // LEXER_H