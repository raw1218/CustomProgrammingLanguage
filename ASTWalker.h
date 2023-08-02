#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "DataStructures/Stack.h"
#include "DataStructures/Map.h"
#include "AST.h"



#define MAX_ADDRESS 1000


typedef struct{

    Stack * variableTableStack; //A stack of variable tables
    Map * functionTable;  //A function table (key is string version of function sig)



    Map * stringTable;
    int sp;
    int labelCount; //for labeling strings

} ASTWalker;

ASTWalker * createASTWalker();




void walkAST(ASTWalker * walker, AST_Node * tree, char * functionName);


char * functionDefinitionToKey(AST_Node * functionNode);

char * functionCallToKey(ASTWalker * walker, AST_Node * functionNode);











#endif //ASTWALKER_H