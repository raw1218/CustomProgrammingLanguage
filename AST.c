#include "AST.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



// Prints a string of spaces for indentation
void printIndentation(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("     ");  // Two spaces per level of depth
    }
}

// Recursively prints an AST_Node and its children
void printASTNode(AST_Node* node, int depth) {
    // Indent and print the node's symbol
    printIndentation(depth);
    printSymbol(node->symbol);


    printf("  ID: %d  ", node->id);
    

    if(node->declarationNode != NULL){
        printf("DEC ID: %d", node->declarationNode->id);
    }
    printf("\n");
    // Recursively print the node's children
    for (int i = 0; i < node->num_children; i++) {
        printASTNode(node->children[i], depth + 1);
    }
}

// Call the recursive function with a depth of 0 to print the entire tree
void printAST(AST_Node* root) {
    printASTNode(root, 0);
}


AST_Node * createASTNode(Symbol symbol){
    AST_Node * node = (AST_Node *)malloc(sizeof(AST_Node));
    node->num_children = 0;
    node->symbol = symbol;
    node->declarationNode = NULL;

    return node;
}


void addChildNode(AST_Node * node, AST_Node * child){
    node->children[node->num_children++] = child;
}

void reverseChildOrder(AST_Node * node){
    int start = 0;
    int end = node->num_children - 1;

    while(start < end){
        AST_Node * temp = node->children[start];
        node->children[start] = node->children[end];
        node->children[end] = temp;
        start++; 
        end--;
    }
}



void simplifyNode(AST_Node * parent, AST_Node * node,int childIndex){
    Symbol sym = node->symbol;
    Symbol parentSym = parent->symbol;

    printf("simplifying node id = %d\n", node->id);

    




    for(int i = 0; i < node->num_children; i++){
        simplifyNode(node, node->children[i], i);
    }


    if(compareSymbols(&sym, &FunctionCallSymbol))return;

    if(node->num_children == 1
        && !compareSymbols(&sym, &FunctionSignatureSymbol)
        && !compareSymbols(&sym, &FunctionCallArgumentSymbol)){
        if(parent != NULL){
            parent->children[childIndex] = node->children[0];
        }
    }

    
    //remove punctuation
    if(compareSymbols(&sym, &PunctuatedStatementSymbol)){
        node->num_children--;
    }


    //simplify the helpers
    if(compareSymbols(&sym, &VariableDeclarationHelperSymbol)
        || compareSymbols(&sym, &PunctuatedStatementSymbol)
        || compareSymbols(&sym, &VariableDeclarationHelperSymbol2)
        || (compareSymbols(&sym, &FunctionSignatureSymbol) && compareSymbols(&parentSym, &FunctionSignatureSymbol))
        || (compareSymbols(&sym, &StatementsSymbol) && compareSymbols(&parentSym, &StatementsSymbol)) 
        ||sym.isHelper
        || compareSymbols(&sym, &IfStatementHelperSymbol)
        ||(compareSymbols(&sym, &IfStatementSymbol) && compareSymbols(&parentSym, &IfElseStatementSymbol) && childIndex == 0)
        ||(compareSymbols(&sym, &RelationalExpressionSymbol) && compareSymbols(&parentSym, &RelationalExpressionSymbol))
        ){
        int numChildren = node->num_children;
        int parentNumChildren = parent->num_children;

      
        // Shift the children of the parent to the right
        for(int i = parentNumChildren - 1; i > childIndex; i--){
            parent->children[i + numChildren - 1] = parent->children[i];
        }



        // Replace the child at childIndex with the children of the current node
        for(int i = 0; i < numChildren; i++){
            parent->children[childIndex + i] = node->children[i];
        }

        // Update the number of children of the parent
        parent->num_children = parentNumChildren + numChildren - 1;
    }





    //simplify statements
    


    if(compareSymbols(&sym, &BlockStatementSymbol)){
        node->children[0] = node->children[1];
        node->num_children = 1;
    }

   //remove "<" and ">" from function Arg
    if(compareSymbols(&sym, &FunctionArgumentSymbol)){

        Symbol first = node->children[0]->symbol;
        Symbol last =node->children[node->num_children - 1]->symbol;

        if(compareSymbols(&first, &LeftCurlyBracketSymbol)
            &&compareSymbols(&last, &RightCurlyBracketSymbol)
            && node->num_children == 4){
   


            node->children[0] = node->children[1];
            node->children[1] = node->children[2];
            node->num_children = 2;
    }}

    
   



 
    

    //special simplification for function signatures
    //if child of function signature is another function signature
    //then replace child with its children

    




}



void simplifyParenthesis(AST_Node * parent, AST_Node * node, int childIndex){
    Symbol sym = node->symbol;
    Symbol parentSym = node->symbol;


    int numChlidren = node->num_children;

    for(int i = 0; i < numChlidren; i++){
        simplifyParenthesis(node, node->children[i], i);
    }
;

            //remove ( )
    if(compareSymbols(&sym, &PrimaryExpressionSymbol)){
    if(compareSymbols(&node->children[0]->symbol, &OpenParenSymbol)
        && compareSymbols(&node->children[node->num_children - 1]->symbol, &CloseParenSymbol)){
            
            node->children[0] = node->children[1];
            node->num_children = 1;
            printf("removing parenthesis, node id = %d, parent id = %d\n", node->id, parent->id);
            parent->children[childIndex] = node->children[0];

        }}



    
}




void _simplifyTree(AST_Node * root){
   
   
    //special function signature exception
    if(compareSymbols(&root->symbol, &FunctionSignatureSymbol)){
        if(root->num_children == 1)return;
    }
   
    for(int i = 0; i < root->num_children; i++){
        simplifyNode(root, root->children[i],i);
    }




    if(root->num_children == 1){
        *root = *root->children[0];
    }
}

bool compareTrees(AST_Node * a, AST_Node * b){
    if(a->id != b->id)return false;
    for(int i = 0; i < a->num_children; i++){
        if(compareTrees(a->children[i], b->children[i]) == false)return false;
    }
    return true;
}
 void simplifyTree(AST_Node * root){

    //return _simplifyTree(root);
    AST_Node  prev = *root;


    _simplifyTree(root);



    while(!compareTrees(&prev, root)){


        prev = *root;
        _simplifyTree(root);
        
    }

    printf("simplified tree about to remove parenthesis\n");
    printAST(root);


    simplifyParenthesis(NULL, root, 0);
 }






bool compareStrings(void * a, void * b){

    
    // strcmp returns 0 if the strings are equal
    return strcmp(a,b) == 0;
}






VariableTableEntry * createVariableTableEntry(DataType type, int mem, AST_Node * declarationNode){
    VariableTableEntry * entry = (VariableTableEntry *)malloc(sizeof(VariableTableEntry));
    
    entry->type = type;
    entry->memoryLocation = mem;
    entry->declarationNode = declarationNode;
    return entry;
}

void throwError(int status) {
    printf("An error occurred with status: %d\n", status);
    exit(status);
}