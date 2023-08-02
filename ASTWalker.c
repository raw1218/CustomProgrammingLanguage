#include "ASTWalker.h"
#include "stdlib.h"
#include "stdbool.h"
#include <string.h>
#include <stdio.h>
#include "Parser.h"
#include "AST.h"


//Helpers//
char * _concatenateWithSpaces(char **strings, int len) {
    // Calculate the total length needed for the final string
    int totalLength = 0;
    for(int i = 0; i < len; i++) {
        totalLength += strlen(strings[i]);
    }

    // Add space for the spaces and the null terminator
    totalLength += len; 

    // Allocate memory for the final string
    char *result = (char*)malloc(totalLength * sizeof(char));
    if(result == NULL) {
        return NULL; // Return NULL if memory allocation fails
    }

    // Copy the first string
    strcpy(result, strings[0]);

    // Concatenate the remaining strings with a space in between
    for(int i = 1; i < len; i++) {
        strcat(result, " ");
        strcat(result, strings[i]);
    }

    return result;
}


//End Helpers//



void loadIntWalker(int value, char * reg_name){
    if (value >= -2048 && value <= 2047) {
        fprintf(stdoutFP,"  addi %s, x0, %d\n", reg_name, value);
    } else {
        int upper_imm = value >> 12;
        int lower_imm = value & 0xfff;
        fprintf(stdoutFP,"  lui %s, %d\n", reg_name, upper_imm);
        fprintf(stdoutFP,"  addi %s, %s, %d\n", reg_name, reg_name, lower_imm);
    }
}


ASTWalker * createASTWalker(){
  
    ASTWalker * walker = (ASTWalker *)malloc(sizeof(ASTWalker));

    Stack * varTableStack  = createStack();
    
    Map * initialTable = createMap(compareStrings);
    push(varTableStack, initialTable);
    
    
    Map * functionTable = createMap(compareStrings);

    walker->functionTable = functionTable;
    walker->variableTableStack = varTableStack;


    Map * stringTable = createMap(compareStrings);
    walker->stringTable = stringTable;
    walker->sp = MAX_ADDRESS;
    walker->labelCount = 0;


    //intitialize strings %d and %f
    fprintf(stdoutFP, ".text\n");
    fprintf(stdoutFP, ".section .rodata\n");
    fprintf(stdoutFP, "  .align 4\n");
    fprintf(stdoutFP,".LCD:\n"); 
    fprintf(stdoutFP,"  .string \"%%d\"\n");
    fprintf(stdoutFP,"  .align 4\n");
    fprintf(stdoutFP,".LCF:\n"); 
    fprintf(stdoutFP,"  .string \"%%d.\"\n");
    fprintf(stdoutFP,"  .align 4\n");
    fprintf(stdoutFP, ".LCBT:\n");
    fprintf(stdoutFP, "  .string \"True\"\n");
    fprintf(stdoutFP,"  .align 4\n");
    fprintf(stdoutFP, ".LCBF:\n");
    fprintf(stdoutFP, "  .string \"False\"\n");
    fprintf(stdoutFP,"  .align 4\n");
    fprintf(stdoutFP, ".LCEmpty:\n");
    fprintf(stdoutFP, "  .string \"\"\n");
    fprintf(stdoutFP, ".align 1\n");


   
    return walker;
}


char * functionDefinitionToKey(AST_Node * functionNode){
    // a function that turns a function call or a function definition node into a string
    Symbol nodeType = functionNode->symbol;


    AST_Node * sig = functionNode->children[1];
    int numChildren = sig->num_children;
    char ** key = (char**)malloc(sizeof(char*) * numChildren);


    if(compareSymbols(&nodeType, &FunctionDefinitionSymbol)){
        //function definition

        for(int i = 0; i < numChildren; i++){

            AST_Node * childNode = sig->children[i];
            Symbol childSymbol = childNode->symbol;

            if(compareSymbols(&childSymbol, &IdentifierSymbol)){
                char * word = (char *)childSymbol.value;
                key[i] = word;
 
            }

            else if(compareSymbols(&childSymbol, &FunctionArgumentSymbol)){
                Symbol typeSym = childNode->children[0]->symbol;
                key[i] = wordFromTypeSymbol(typeSym);
         
            }

            else throwError(-1);
        }
    }



    char * result = _concatenateWithSpaces(key, numChildren);
    return result;

}

char * functionCallToKey(ASTWalker * walker, AST_Node * functionNode){
    
    
    Symbol nodeType = functionNode->symbol;
    int numChildren = functionNode->num_children;

    char ** key = (char**)malloc(sizeof(char*) * numChildren);
    
    if(compareSymbols(&nodeType, &FunctionCallSymbol)){
        //function definition

        for(int i = 0; i < numChildren; i++){

            AST_Node * childNode = functionNode->children[i];
            Symbol childSymbol = childNode->symbol;


            if(compareSymbols(&childSymbol, &FunctionKeywordSymbol)){
                char * word = (char *)childSymbol.value;
                key[i] = word;
            }

            else if(compareSymbols(&childSymbol, &FunctionCallArgumentSymbol)){
                AST_Node * valueNode = childNode->children[0];
                walkAST(walker, valueNode, NULL);
                DataType type = valueNode->resultType;
                key[i] = wordFromDataType(type);
            }

            else throwError(-2);
        }
    }

    char * result = _concatenateWithSpaces(key, numChildren);
    return result;
}

void addNewScope(ASTWalker * walker){
    Map * newScope = createMap(compareStrings);
    push(walker->variableTableStack, newScope);

}



bool doesVariableExistInScope(Map * variableTable, Symbol identifierSym){
   
    if(variableTable == NULL)return false;

    char * word = (char * )identifierSym.value;
    return get(variableTable, word) != NULL;
}

bool doesVariableExistInCurrentScope(ASTWalker * walker, Symbol identifierSym){
    Map * currentScope = (Map*)peek(walker->variableTableStack);
    return doesVariableExistInScope(currentScope, identifierSym);
}

bool doesVariableExist(ASTWalker * walker, Symbol identifierSym){

    int numScopes = walker->variableTableStack->numItems;


    for(int i = 0; i < numScopes; i++){
        Map * scope = peekAtIndex(walker->variableTableStack, i);
        if(doesVariableExistInScope(scope, identifierSym)) return true;
    }

    return false;
}

void addVariableToCurrentScope(ASTWalker * walker, char * key, VariableTableEntry * entry){
    Map * curScope = (Map *)peek(walker->variableTableStack);
    
    if(get(curScope, key) != NULL)throwError(1);
    
    insert(curScope, key, entry);

}

VariableTableEntry * getVariableEntry(ASTWalker * walker, Symbol identifierSym){
    

    int numScopes = walker->variableTableStack->numItems;
    char * word = (char *)identifierSym.value;

    for(int i = 0; i < numScopes; i++){
        Map * scope = peekAtIndex(walker->variableTableStack, i);
        if(doesVariableExistInScope(scope, identifierSym)){
            return get(scope, word);
        }
    }

    return NULL;
}

void addFunctionToTable(ASTWalker * walker, AST_Node * declarationNode){

    char * key = functionDefinitionToKey(declarationNode);
    DataType returnType = dataTypeFromTypeSymbol(declarationNode->children[3]->symbol);


    FunctionTableEntry * entry = (FunctionTableEntry*)malloc(sizeof(FunctionTableEntry));
    entry->declarationNode = declarationNode;
    entry->returnType = returnType;

    insert(walker->functionTable, key, entry);

}















void walkFunctionDeclaration(ASTWalker * walker, AST_Node * node, char * functionName){


    char * key = functionDefinitionToKey(node);
    printf("walking function declaration, key = %s\n", key);


    //if it already exists in function table throw error
    if(get(walker->functionTable, key) != NULL) throwError(2);

    //add to table
    addFunctionToTable(walker, node);


    //start new scope
    addNewScope(walker);

    //process parameters and store in new scope
    AST_Node * sigNode = node->children[1];
    for(int i = 0; i < sigNode->num_children; i++){
        AST_Node * childNode = sigNode->children[i];
        Symbol childSym =childNode->symbol;
        if(compareSymbols(&childSym, &FunctionArgumentSymbol)){
            Symbol type = childNode->children[0]->symbol;
            Symbol iden = childNode->children[1]->symbol;
            
            DataType dataType = dataTypeFromTypeSymbol(type);
            char * word = (char * )iden.value;

            VariableTableEntry * entry = createVariableTableEntry(dataType, -1, NULL);
            addVariableToCurrentScope(walker, word, entry);
            entry->hasValue = true;
        }
    }


    //walk function body
    AST_Node * bodyNode = node->children[5];
    walkAST(walker, bodyNode, key);


    //exit scope
    pop(walker->variableTableStack);

}

void walkStatements(ASTWalker * walker, AST_Node * node, char * functionName){
    int numChildren = node->num_children;
    for(int i = 0; i < numChildren; i++){
        AST_Node * child = node->children[i];
        walkAST(walker, child, functionName);
    }
}

void walkBinaryArithmeticExpression(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * leftChild = node->children[0];
    Symbol operationSymbol = node->children[1]->symbol;
    AST_Node * rightChild = node->children[2];

    walkAST(walker, leftChild, functionName);
    walkAST(walker, rightChild, functionName);


    DataType leftType = leftChild->resultType;
    DataType rightType = rightChild->resultType;


    DataType resultType;
    if(leftType == TYPE_INTEGER && rightType == TYPE_INTEGER) resultType = TYPE_INTEGER;
    if(leftType == TYPE_DECIMAL || rightType == TYPE_DECIMAL) resultType = TYPE_DECIMAL;



    if(leftType != TYPE_INTEGER && leftType != TYPE_DECIMAL) throwError(3);
    if(rightType != TYPE_INTEGER && rightType != TYPE_DECIMAL) throwError(4);


    //division operation always results in a decimal
    if(compareSymbols(&operationSymbol, &SlashSymbol)) resultType = TYPE_DECIMAL;


    node->resultType = resultType;
}

void walkUnaryArithmeticExpression(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * child = node->children[1];
    walkAST(walker, child, functionName);
    

    if(child->resultType != TYPE_INTEGER && child->resultType != TYPE_DECIMAL) throwError(5);

    node->resultType = child->resultType;
}


void walkUninitializedVariableDeclaration(ASTWalker * walker, AST_Node * node, char * functionName){
    
    //need to handle unrecursive chained uninit. declarations
    int numChildren = node->num_children;
    
    Symbol  typeSymbol = node->children[numChildren - 1]->symbol;
    DataType type = dataTypeFromTypeSymbol(typeSymbol);

    for(int i = 0; i < numChildren - 2; i+=2){
        Symbol identifierSym = node->children[i]->symbol;
        if(doesVariableExistInCurrentScope(walker, identifierSym)) throwError(6);
        char * word = (char *)identifierSym.value;
        VariableTableEntry * entry =  createVariableTableEntry(type, -1, node);
        entry->hasValue = false;
    
        addVariableToCurrentScope(walker, word, entry);

    }



}

void walkInitializedVariableDeclaration(ASTWalker * walker, AST_Node * node, char * functionName){
    //needs to support recursive chained init. declaration:   X Is Integer = Y Is Integer = 5.
    //needs to support nonrecursive chained init. decllaration: X Is Y Is Integer = 5. 
   
    int numChildren = node->num_children;
    Symbol typeSymbol = node->children[numChildren - 3]->symbol;
    DataType type = dataTypeFromTypeSymbol(typeSymbol);
    node->resultType = type;
    AST_Node * valueNode = node->children[numChildren - 1];

    //check if the value type matches declared type
    walkAST(walker, valueNode, functionName);
    if(valueNode->resultType != type) throwError(8);



    int numIdentifiers = 1 + ((numChildren - 5) / 2);

    for(int i = 0; i < numIdentifiers; i++){
        int identifierIndex = i * 2;
        Symbol identifierSym = node->children[identifierIndex]->symbol;
        char * word = (char *)identifierSym.value;

        //if it already exists throw error
        if(doesVariableExistInCurrentScope(walker, identifierSym)) throwError(7);

        VariableTableEntry * entry =  createVariableTableEntry(type, -1, node);
        entry->hasValue = true;
        
        addVariableToCurrentScope(walker, word, entry);
    }

}

void walkVariableUsage(ASTWalker * walker, AST_Node * node, char * functionName){
    
    Symbol identifierSym = node->symbol;
    
    if(!doesVariableExist(walker, identifierSym)) throwError(9);


    VariableTableEntry * entry = getVariableEntry(walker, identifierSym);  
    node->resultType = entry->type;
    node->declarationNode = entry->declarationNode;

    if(entry->hasValue == false) throwError(10);
    
}

void walkFunctionCall(ASTWalker * walker, AST_Node * node, char * functionName){
    char * key = functionCallToKey(walker, node);
    printf("walking function call, key = %s\n", key);


    //if it doesnt exist throw error
    FunctionTableEntry * entry = get(walker->functionTable, key);
    if( entry == NULL) throwError(11);

    node->resultType = entry->returnType;
    node->declarationNode = entry->declarationNode;


}

void walkReturnStatement(ASTWalker * walker, AST_Node * node, char * functionName){
    
    if(functionName == NULL) throwError(12);
    
    FunctionTableEntry * entry = get(walker->functionTable, functionName);
    DataType expectedType = entry->returnType;


    AST_Node * returnValueNode = node->children[2];
    walkAST(walker, returnValueNode, functionName);
    
    if(expectedType != returnValueNode->resultType) throwError(13);
}

void walkLiteral(ASTWalker * walker, AST_Node * node, char * functionName){

    Symbol sym = node->symbol;
    DataType type = dataTypeFromTypeSymbol(sym);
    node->resultType = type;

}

void walkVariableAssignment(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * identifierNode = node->children[0];
    Symbol identifierSym = identifierNode->symbol;
    if(!doesVariableExist(walker, identifierSym)) throwError(14);

    AST_Node * valueNode =  node->children[2];
    walkAST(walker, valueNode, functionName);

    VariableTableEntry * entry = getVariableEntry(walker, identifierSym);
    if(entry->type != valueNode->resultType) throwError(15);
    entry->hasValue = true;

    node->resultType = entry->type;
    identifierNode->declarationNode = entry->declarationNode;
}



void walkPrintStatement(ASTWalker * walker, AST_Node * node, char * functionName){
    printf("walking print statement\n");
    AST_Node * valueNode = node->children[1];
    walkAST(walker, valueNode, functionName);

    DataType type = valueNode->resultType;

    if(!(type == TYPE_STRING || type == TYPE_INTEGER || type == TYPE_BOOLEAN || type == TYPE_DECIMAL))throwError(17);

}

void walkBinaryLogicalExpression(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * leftChild = node->children[0];
    Symbol operationSymbol = node->children[1]->symbol;
    AST_Node * rightChild = node->children[2];

    walkAST(walker, leftChild, functionName);
    walkAST(walker, rightChild, functionName);


    DataType leftType = leftChild->resultType;
    DataType rightType = rightChild->resultType;

    if(leftType != TYPE_BOOLEAN)throwError(18);

    node->resultType = TYPE_BOOLEAN;
}

void walkUnaryLogicalExpression(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * child = node->children[1];
    walkAST(walker, child, functionName);
    if(child->resultType != TYPE_BOOLEAN)throwError(19);
    node->resultType = TYPE_BOOLEAN;
}

void walkIfStatement(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * ifNode = node->children[1];
    AST_Node * thenNode = node->children[3];
    walkAST(walker, ifNode, functionName);

    if(ifNode->resultType != TYPE_BOOLEAN)throwError(20);


    addNewScope(walker);
    walkAST(walker, thenNode, functionName);
    //exit scope
    pop(walker->variableTableStack);
    
    


}

void walkIfElseStatement(ASTWalker * walker, AST_Node * node, char * functionName){
    AST_Node * ifNode = node->children[1];
    AST_Node * thenNode = node->children[3];
    AST_Node * elseNode = node->children[5];

    walkAST(walker, ifNode, functionName);

    if(ifNode->resultType != TYPE_BOOLEAN)throwError(20);


    addNewScope(walker);
    walkAST(walker, thenNode, functionName); 
    pop(walker->variableTableStack);    
    
    addNewScope(walker);   
    walkAST(walker, elseNode, functionName);
    pop(walker->variableTableStack);
}


void walkString(ASTWalker * walker, AST_Node * node, char * functionName){
    


    Symbol sym = node->symbol;
    char * word = sym.value;

    int labelNum = walker->labelCount++;
    fprintf(stdoutFP, "  .align 4\n");
    fprintf(stdoutFP, ".LC%d:\n", labelNum);
    fprintf(stdoutFP, "  .string \"%s\"\n", word);


    int * entry =(int*)malloc(sizeof(int));
    *entry = labelNum;
    
    insert(walker->stringTable, word, entry);

    node->resultType = TYPE_STRING;



}



void walkLoop(ASTWalker * walker, AST_Node * node, char * functionName){

    AST_Node * expressionNode = node->children[1];
    AST_Node * bodyNode = node->children[2];

    addNewScope(walker);

    walkAST(walker, expressionNode, functionName);

    if(expressionNode->resultType != TYPE_BOOLEAN) throwError(22);

    walkAST(walker, bodyNode, functionName);

    pop(walker->variableTableStack);


}


void walkRelationalExpression(ASTWalker * walker, AST_Node * node, char * functionName){
    int numChildren = node->num_children;

    int numTerms = ((numChildren - 3) / 2) + 2;


    for(int i = 0; i < numTerms - 1; i++){

        int firstIndex = i * 2;
        int secondIndex = firstIndex + 2;


        AST_Node * firstNode = node->children[firstIndex];
        AST_Node * secondNode = node->children[secondIndex];

        walkAST(walker, firstNode, functionName);
        walkAST(walker, secondNode, functionName);

        if(firstNode->resultType != TYPE_INTEGER && firstNode->resultType != TYPE_DECIMAL)throwError(23);
        if(secondNode->resultType != TYPE_INTEGER && secondNode->resultType != TYPE_DECIMAL)throwError(24);


    }

    node->resultType = TYPE_BOOLEAN;


}















void walkAST(ASTWalker * walker, AST_Node * node, char * functionName){
   
    bool insideFunction = functionName != NULL;
   
    int numChildren = node->num_children;
    Symbol sym = node->symbol;

    //statements
    if(compareSymbols(&sym, &StatementsSymbol)){
        walkStatements(walker, node, functionName);
    }


    //function definition
    if(compareSymbols(&sym, &FunctionDefinitionSymbol)){
        walkFunctionDeclaration(walker, node, functionName);
    }


    //Binary Arithmetic Expression
    if(compareSymbols(&sym, &AdditiveExpressionSymbol)
        || compareSymbols(&sym, &MultiplicativeExpressionSymbol)){

        walkBinaryArithmeticExpression(walker, node, functionName);
    }


    //unary arithmetic expression
    if(compareSymbols(&sym, &UnaryNegationExpressionSymbol)){
        walkUnaryArithmeticExpression(walker, node, functionName);
    }

    //Uninitialized variable declaration
    if(compareSymbols(&sym, &VariableDeclarationSymbol)){
        walkUninitializedVariableDeclaration(walker, node, functionName);
    }


    //Initialized variable declaration
    if(compareSymbols(&sym, &InitializedVariableDeclarationSymbol)){
        walkInitializedVariableDeclaration(walker, node, functionName);
    }

    //Identifier (Variable Usage)
    if(compareSymbols(&sym, &IdentifierSymbol)){
        walkVariableUsage(walker, node, functionName);
    }

    //Function Call
    if(compareSymbols(&sym, &FunctionCallSymbol)){
        walkFunctionCall(walker, node, functionName);
    }

    //Return Statement
    if(compareSymbols(&sym, &ReturnStatementSymbol)){
        walkReturnStatement(walker, node, functionName);
    }

    //Literals
    if(compareSymbols(&sym, &NumberIntegerSymbol)
        || compareSymbols(&sym, &NumberFloatSymbol)
        || compareSymbols(&sym, &TrueSymbol)
        || compareSymbols(&sym, &FalseSymbol)){

        walkLiteral(walker, node, functionName);
    }

    //Variable Assignment
    if(compareSymbols(&sym, &variableAssignmentSymbol)){
        walkVariableAssignment(walker, node, functionName);
    }


    //string
    if(compareSymbols(&sym, &StringSymbol)){
        printf("about to walk string\n");
        walkString(walker, node, functionName);
    }



    //Binary Logical Expression
    if(compareSymbols(&sym, &AndExpressionSymbol)
        || compareSymbols(&sym, &OrExpressionSymbol)){

        walkBinaryLogicalExpression(walker, node, functionName);
    }


    //unary arithmetic expression
    if(compareSymbols(&sym, &NotExpressionSymbol)){
        walkUnaryLogicalExpression(walker, node, functionName);
    }


    if(compareSymbols(&sym, &IfStatementSymbol))walkIfStatement(walker, node, functionName);

    if(compareSymbols(&sym, &IfElseStatementSymbol))walkIfElseStatement(walker, node, functionName);

    if(compareSymbols(&sym, &PrintStatementSymbol)) walkPrintStatement(walker, node, functionName);



    //loops
    if(compareSymbols(&sym, &WhileStatementSymbol)
        || compareSymbols(&sym, &UntilStatementSymbol)){

        walkLoop(walker, node, functionName);
    }


    if(compareSymbols(&sym, &RelationalExpressionSymbol))walkRelationalExpression(walker, node, functionName);


}