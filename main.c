#include "AST.h"
#include "Lexer.h"
#include "Parser.h"
#include <stdio.h>


int main(){


    char * source = "2 + 2";

    Token * * stream = tokenize(source);


    printf("made the tokesn, about to parse\n");
    int index = 0;
    Node * node = parseExpression(stream, &index);
    printf("about to print the tree\n");
    printAST(node, 0);
}