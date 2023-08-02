#ifndef AST_H
#define AST_H

#include "CFG.h"
#include  "DataStructures/Map.h"

#define MAX_CHILDREN 20
//TODO: redo nodes so that theres no max children and children are dynamically located



typedef enum {
    NODE_TYPE_INTEGER,
    NODE_TYPE_DECIMAL,
} AST_Node_Type;


typedef struct AST_Node {

    Symbol symbol;
    struct AST_Node * children[MAX_CHILDREN];
    int num_children;

    int id; //Used by the Assembly Code Generation to keep track of subvalues

    DataType resultType;


    int startIndex;   //start and end index in symbol input
    int endIndex;


    struct AST_Node *  declarationNode; //identifiers and function calls will be linked to their declaration nodes after walking AST
    char * label;  //used for strings


} AST_Node;


AST_Node * createASTNode(Symbol symbol);

void addChildNode(AST_Node * node, AST_Node * child);

void simplifyTree(AST_Node * root);

void reverseChildOrder(AST_Node * node);


///////////////////////////////////////////////////////
// structs and funcs used for Walking and Generation phases

typedef struct{
    Map * variables;
    int frameSize;
    int frameStart;

} VariableTable;

bool compareStrings(void * a, void * b);


char * functionToKey(AST_Node * functionNode);

//variable table
typedef struct {
    DataType type;
    int memoryLocation;
    AST_Node * declarationNode;
    bool hasValue;
} VariableTableEntry;

VariableTableEntry * createVariableTableEntry(DataType type, int mem, AST_Node * declarationNode);


//function table
typedef struct {
    AST_Node * declarationNode;
    DataType returnType;
    char * label;
} FunctionTableEntry;


void throwError(int status);


void printAST(AST_Node * node);


#endif //AST_H