#ifndef CFG_H
#define CFG_H


#include "DataStructures/Set.h"


#define NUM_TERMINALS  54
#define NUM_NON_TERMINALS  46

typedef enum {
    TYPE_INTEGER,
    TYPE_DECIMAL,
    TYPE_STRING,
    TYPE_BOOLEAN,
} DataType;


#define KEYWORD_START_INDEX  0
#define OPERATOR_START_INDEX  19
#define PUNCTUATION_START_INDEX  28

typedef enum {
 
    
    //keywords start here
    Symbol_Type_Integer, //"Integer"
    Symbol_Type_Decimal, //"Decimal"
    Symbol_Type_String, //"String"
    Symbol_Type_Boolean,  //"Boolean"
    Symbol_Is,          //"Is"
    Symbol_Define,        //"Define"
    Symbol_As,            //"As"
    Symbol_Result,         //"Result"
    Symbol_Print,           //"Print"
    Symbol_True,            //"True"
    Symbol_False,       //"False"
    Symbol_Or,              //"Or"
    Symbol_And, //"And"
    Symbol_Not, //"Not"
    Symbol_If, //"If"
    Symbol_Then, //"Then"
    Symbol_Else, //"Else"
    Symbol_While, //"While"
    Symbol_Until, //"Until"




    //operators start here
    Symbol_Plus,   // "+"
    Symbol_Minus, // "-"
    Symbol_Star, // "*"
    Symbol_Slash, // "/"
    Symbol_Equals, // "="
    Symbol_Greater, //">"
    Symbol_Less, //"<""
    Symbol_GreaterEqual, //">="
    Symbol_LessEqual, //"<="


    //punctuation starts here
    Symbol_Period, // "."
    Symbol_LeftCurlyBracket, //"{"
    Symbol_RightCurlyBracket, //"}"
    Symbol_OpenParen, //"("
    Symbol_CloseParen, //")"
    Symbol_DoubleQuote, //" " "

    Symbol_DoubleSlash,
    Symbol_Percent,
    Symbol_Caret,

    Symbol_Number_Integer,
    Symbol_Number_Float,
    Symbol_String,




 
    Symbol_Xor,
    Symbol_Nor,
    Symbol_Nand,


    Symbol_Identifier,
    Symbol_PlusEquals,
    Symbol_MinusEquals,
    Symbol_StarEquals,
    Symbol_SlashEquals,
    Symbol_DoubleSlashEquals,
    Symbol_PercentEquals,
    Symbol_CaretEquals,


    Symbol_End,
    Symbol_Invalid,

    Symbol_FunctionKeyWord,

    
} Terminal;

typedef enum {

    Symbol_Number,
    Symbol_ArithmeticExpression,
    Symbol_AdditiveExpression,
    Symbol_MultiplicativeExpression,
    Symbol_ExponentiationExpression,
    Symbol_UnaryNegationExpression,
    Symbol_PrimaryExpression,
    Symbol_NonRelationalExpression,
    Symbol_LogicalExpression,
    Symbol_OrExpression,
    Symbol_AndExpression,
    Symbol_NotExpression,
    Symbol_PrimaryLogicalExpression,
    Symbol_RelationalExpression,
    Symbol_AssignmentExpression,
    Symbol_CompoundAssignmentOperator,
    Symbol_Start,
    Symbol_InitializedVariableDeclaration,

    Symbol_Statement,
    Symbol_VariableDeclaration,
    Symbol_Type,

    Symbol_VariableDeclarationHelper,
    Symbol_VariableDeclarationHelper2,
    Symbol_Statements,
    Symbol_PunctuatedStatement,
    Symbol_PunctuatedStatementHelper,
    Symbol_BlockStatement, 

    //Functions
    Symbol_FunctionDefinition,
    Symbol_FunctionSignature,
    Symbol_FunctionArgument,
    Symbol_ReturnStatement,
    Symbol_FunctionCall,
    Symbol_FunctionCallArgument,


    Symbol_Literal, 
    Symbol_ExpressionStatement,
    Symbol_AssignmentHelper,
    Symbol_PrintStatement,
    Symbol_Boolean,

    Symbol_IfStatement,
    Symbol_IfStatementHelper,
    Symbol_IfElseStatement,
    Symbol_WhileStatement,
    Symbol_UntilStatement,

    Symbol_Expression,
    Symbol_RelationalOperator, 
    Symbol_RelationalExpressionSymbol,


    

} NonTerminal;




typedef struct Symbol {
    bool isTerminal;
    bool isHelper;
    union {
        Terminal terminal;
        NonTerminal nonTerminal;
    };

    void * value; //Optional value for literals
    int functionKeywordIndex;  //optional value for function keywords;
    bool compareValue;  //False when you want to just compare Symbol TYpe
} Symbol;

typedef struct  {
    Symbol lhs;
    Symbol* rhs;
    int rhs_len;
} Rule;





Symbol createSymbol(bool isTerminal, int symbolIndex);
void printSymbol(Symbol symbol);


bool compareSymbols(void *a, void *b);
bool compareRules(void* a, void* b);
bool compareSymbolsAndVal(void *a, void * b);



//////////////////////////Declare the symbols ////////////////////////////
//////////////////////////////////////////////////////////////////////////


//if statemet
extern Symbol IfSymbol;
extern Symbol ThenSymbol;
extern Symbol ElseSymbol;
extern Symbol IfStatementSymbol; 
extern Symbol IfStatementHelperSymbol; 
extern Symbol IfElseStatementSymbol;
//while statement
extern Symbol WhileSymbol;
extern Symbol UntilSymbol;
extern Symbol WhileStatementSymbol;
extern Symbol UntilStatementSymbol;



/////////////////////////Arithmetic///////////////////////////

extern Symbol StartSymbol;
extern Symbol EndSymbol;
extern Symbol IdentifierSymbol;
extern Symbol LiteralSymbol;
extern Symbol PrintSymbol;
extern Symbol PrintStatementSymbol;


// Declare terminal symbols
extern Symbol PlusSymbol;
extern Symbol MinusSymbol;
extern Symbol StarSymbol;
extern Symbol SlashSymbol;
extern Symbol DoubleSlashSymbol;
extern Symbol PercentSymbol;
extern Symbol CaretSymbol;
extern Symbol NumberIntegerSymbol;
extern Symbol NumberFloatSymbol;
extern Symbol StringSymbol;



extern Symbol OpenParenSymbol;
extern Symbol CloseParenSymbol;
extern Symbol LeftCurlyBracketSymbol;
extern Symbol RightCurlyBracketSymbol;
extern Symbol DoubleQuoteSymbol;

extern Symbol EqualSymbol;
extern Symbol TypeIntegerSymbol;
extern Symbol TypeDecimalSymbol;
extern Symbol TypeStringSymbol;
extern Symbol PeriodSymbol;
extern Symbol InvalidSymbol;

extern Symbol IsSymbol;
extern Symbol DefineSymbol;
extern Symbol AsSymbol;
extern Symbol ResultSymbol;
extern Symbol FunctionKeywordSymbol;

// Declare non-terminal symbols
extern Symbol NonRelationalExpressionSymbol;
extern Symbol NumberSymbol;
extern Symbol ArithmeticExpressionSymbol;
extern Symbol AdditiveExpressionSymbol;
extern Symbol MultiplicativeExpressionSymbol;
extern Symbol ExponentiationExpressionSymbol;
extern Symbol UnaryNegationExpressionSymbol;
extern Symbol PrimaryExpressionSymbol;
extern Symbol InitializedVariableDeclarationSymbol;
extern Symbol variableAssignmentSymbol;


extern Symbol StatementSymbol;
extern Symbol VariableDeclarationSymbol;
extern Symbol TypeSymbol;
extern Symbol VariableDeclarationHelperSymbol;
extern Symbol VariableDeclarationHelperSymbol2;
extern Symbol StatementsSymbol;
extern Symbol PunctuatedStatementSymbol;
extern Symbol PunctuatedStatementHelperSymbol;
extern Symbol BlockStatementSymbol;
extern Symbol ExpressionStatementSymbol;
extern Symbol AssignmentHelperSymbol;

extern Symbol FunctionDefinitionSymbol;
extern Symbol FunctionSignatureSymbol;
extern Symbol FunctionArgumentSymbol;
extern Symbol ReturnStatementSymbol;
extern Symbol FunctionCallSymbol;
extern Symbol FunctionCallArgumentSymbol;
////////////////////////////Logical/////////////////////////////////////

// Declare terminal symbols

extern Symbol BooleanSymbol;
extern Symbol TrueSymbol;
extern Symbol FalseSymbol;
extern Symbol TypeBooleanSymbol;

extern Symbol OrSymbol;
extern Symbol XorSymbol;
extern Symbol NorSymbol;
extern Symbol AndSymbol;
extern Symbol NandSymbol;
extern Symbol NotSymbol;
extern Symbol TrueSymbol;
extern Symbol FalseSymbol;

// Declare non-terminal symbols
extern Symbol LogicalExpressionSymbol;
extern Symbol OrExpressionSymbol;
extern Symbol AndExpressionSymbol;
extern Symbol NotExpressionSymbol;
extern Symbol PrimaryLogicalExpressionSymbol;

/////////////////////Relational///////////////////////////////////

extern Symbol LessSymbol;
extern Symbol LessEqualSymbol;
extern Symbol GreaterSymbol;
extern Symbol GreaterEqualSymbol;
extern Symbol RelationalExpressionSymbol;
extern Symbol RelationalOperatorSymbol;



Rule* createRules2(int* numRules);

char * wordFromTypeSymbol(Symbol sym);
char * wordFromDataType(DataType type);
DataType dataTypeFromTypeSymbol(Symbol sym);


#endif //CFG_H