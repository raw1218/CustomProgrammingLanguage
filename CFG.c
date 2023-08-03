#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


#include "CFG.h"
#include "AST.h"





Symbol createSymbol(bool isTerminal, int symbolIndex) {
    Symbol symbol;
    symbol.isTerminal = isTerminal;
    symbol.isHelper = false;
    symbol.compareValue = false;

    if (isTerminal) {
        symbol.terminal = (Terminal) symbolIndex;
    } else {
        symbol.nonTerminal = (NonTerminal) symbolIndex;
    }

    return symbol;
}

Rule* createRule(int rhs_len) {
    Rule* rule = (Rule*)malloc(sizeof(Rule));
    rule->rhs = (Symbol*)malloc(rhs_len * sizeof(Symbol));
    rule->rhs_len = rhs_len;
    return rule;
}



bool compareSymbolsAndVal(void *a, void * b){
    //special check for functionKeyWord Symbols

    Symbol * symbol1 = (Symbol*)a;
    Symbol * symbol2 = (Symbol *)b;
    if(compareSymbols(a,b) == false)return false;

    


    if(symbol1->isTerminal == true && symbol1->isHelper == false
        &&symbol1->terminal == Symbol_FunctionKeyWord
        &&symbol2->terminal == Symbol_FunctionKeyWord){

            char * val1 = (char*)symbol1->value;
            char * val2 = (char *)symbol2->value;
            return (strcmp(val1, val2) == 0);

        }


    return true;



    
}

bool compareSymbols(void *a, void *b) {


    //for some reason, if you //printf something, then it causes an infinite loop
    Symbol *symbol1 = (Symbol*)a;
    Symbol *symbol2 = (Symbol*)b;
    
    if (symbol1->isTerminal != symbol2->isTerminal) {
        return false;
    }

    if(symbol1->isHelper != symbol2->isHelper) return false;
    


    if(symbol1->isHelper){
        return symbol1->nonTerminal == symbol2->nonTerminal;
    }








    if (symbol1->isTerminal) {
        return symbol1->terminal == symbol2->terminal;
    } else {
        return symbol1->nonTerminal == symbol2->nonTerminal;
    }
}

bool compareRules(void* a, void* b) {
    Rule* rule1 = (Rule*)a;
    Rule* rule2 = (Rule*)b;

    if (!compareSymbolsAndVal(&(rule1->lhs), &(rule2->lhs)) || rule1->rhs_len != rule2->rhs_len) {
        return false;
    }
    for (int i = 0; i < rule1->rhs_len; i++) {
        if (!compareSymbolsAndVal(&(rule1->rhs[i]), &(rule2->rhs[i]))) {
            return false;
        }
    }
    return true;
}




int defaultInt = -99999;
float defaultFloat = -99999.999;

//////////////////////////Define the symbols ////////////////////////////
//////////////////////////////////////////////////////////////////////////

//if statement
Symbol IfSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_If};
Symbol ThenSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Then};
Symbol ElseSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Else};
Symbol IfStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_IfStatement};
Symbol IfStatementHelperSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_IfStatementHelper};
Symbol IfElseStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_IfElseStatement};
//while statement
Symbol WhileSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_While};
Symbol UntilSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Until};
Symbol WhileStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_WhileStatement};
Symbol UntilStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_UntilStatement};





/////////////////////////Arithmetic///////////////////////////

Symbol StartSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Start};
Symbol EndSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_End};

Symbol IdentifierSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Identifier};
// Define terminal symbols
Symbol PlusSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Plus };
Symbol MinusSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Minus };
Symbol StarSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Star };
Symbol SlashSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Slash };
Symbol DoubleSlashSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_DoubleSlash };
Symbol PercentSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Percent };
Symbol CaretSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Caret };
Symbol OpenParenSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_OpenParen };
Symbol CloseParenSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_CloseParen };
Symbol LeftCurlyBracketSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_LeftCurlyBracket};
Symbol RightCurlyBracketSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_RightCurlyBracket};
Symbol DoubleQuoteSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal  = Symbol_DoubleQuote};
Symbol PrintSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal  = Symbol_Print};

Symbol NumberIntegerSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Number_Integer, .value = &defaultInt};
Symbol NumberFloatSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Number_Float, .value = &defaultFloat};
Symbol StringSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_String};
Symbol TypeStringSymbol ={.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Type_String};

Symbol EqualSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Equals};
Symbol TypeIntegerSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Type_Integer};
Symbol TypeDecimalSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Type_Decimal};
Symbol PeriodSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Period};
Symbol InvalidSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Invalid};

Symbol IsSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Is};
Symbol DefineSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal =Symbol_Define};
Symbol AsSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_As};
Symbol ResultSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Result};
Symbol FunctionKeywordSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_FunctionKeyWord};

// Define non-terminal symbols
Symbol LiteralSymbol = {.compareValue = false, .isHelper = false,  .isTerminal = false, .nonTerminal = Symbol_Literal};

Symbol NonRelationalExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_NonRelationalExpression};
Symbol NumberSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Number };

Symbol ArithmeticExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_ArithmeticExpression };
Symbol AdditiveExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_AdditiveExpression };
Symbol MultiplicativeExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_MultiplicativeExpression };
Symbol ExponentiationExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_ExponentiationExpression };
Symbol UnaryNegationExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_UnaryNegationExpression };
Symbol PrimaryExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_PrimaryExpression };
Symbol InitializedVariableDeclarationSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_InitializedVariableDeclaration};
Symbol variableAssignmentSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_AssignmentExpression};

Symbol StatementSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Statement};
Symbol VariableDeclarationSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_VariableDeclaration};
Symbol TypeSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Type};
Symbol VariableDeclarationHelperSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_VariableDeclarationHelper};
Symbol VariableDeclarationHelperSymbol2 = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal =Symbol_VariableDeclarationHelper2};
Symbol StatementsSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Statements};
Symbol PunctuatedStatementSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_PunctuatedStatement};
Symbol PunctuatedStatementHelperSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_PunctuatedStatementHelper};
Symbol BlockStatementSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_BlockStatement};
Symbol ExpressionStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_ExpressionStatement};

Symbol FunctionDefinitionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_FunctionDefinition};
Symbol FunctionSignatureSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_FunctionSignature};
Symbol FunctionArgumentSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_FunctionArgument};
Symbol ReturnStatementSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_ReturnStatement};
Symbol FunctionCallSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_FunctionCall};
Symbol FunctionCallArgumentSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_FunctionCallArgument};

Symbol PrintStatementSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .terminal  = Symbol_PrintStatement};

////////////////////////////Logical/////////////////////////////////////

// Define terminal symbols
Symbol OrSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Or };
Symbol XorSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Xor };
Symbol NorSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Nor };
Symbol AndSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_And };
Symbol NandSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Nand };
Symbol NotSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_Not };
Symbol TrueSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_True };
Symbol FalseSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .terminal = Symbol_False };

// Define non-terminal symbols
Symbol LogicalExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_LogicalExpression };
Symbol OrExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_OrExpression };
Symbol AndExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_AndExpression };
Symbol NotExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_NotExpression };
Symbol PrimaryLogicalExpressionSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_PrimaryLogicalExpression };


Symbol BooleanSymbol = {.compareValue = false, .isHelper =false, .isTerminal = false, .nonTerminal = Symbol_Boolean};
Symbol TypeBooleanSymbol = {.compareValue = false, .isHelper =false, .isTerminal = true, .nonTerminal = Symbol_Type_Boolean};




/////////////////////////////////Relational//////////////////////////

Symbol LessSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Less};
Symbol LessEqualSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_LessEqual};
Symbol GreaterSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_Greater};
Symbol GreaterEqualSymbol = {.compareValue = false, .isHelper = false, .isTerminal = true, .terminal = Symbol_GreaterEqual};

Symbol RelationalExpressionSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_RelationalExpression};
Symbol RelationalOperatorSymbol = {.compareValue = false, .isHelper = false, .isTerminal = false, .nonTerminal = Symbol_RelationalOperator};



/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////                DEFINE RULES        /////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////



void addRule(Rule * rules, int * ruleCount, Symbol * ruleRHS, Symbol ruleLHS, int rhsLen){
    Rule rule = {.lhs = ruleLHS, .rhs = ruleRHS, .rhs_len = rhsLen};
    rules[(*ruleCount)++] = rule;
}
Rule* createRules2(int* numRules) {

    
    Rule* rules = (Rule*)malloc((*numRules) * sizeof(Rule));
    int count = 0;

    // Start -> Statements
    Symbol* rule0RHS = (Symbol*)malloc(sizeof(Symbol));
    rule0RHS[0] = StatementsSymbol;
    addRule(rules, &count, rule0RHS, StartSymbol, 1);


    // Literal -> Number
    Symbol* rule0_2RHS = (Symbol*)malloc(sizeof(Symbol));
    rule0_2RHS[0] = NumberSymbol;
    addRule(rules, &count, rule0_2RHS, LiteralSymbol, 1); 

    // Literal -> String
    Symbol * rule0aRHS = malloc(sizeof(Symbol));
    rule0aRHS[0] = StringSymbol;
    addRule(rules, &count, rule0aRHS, LiteralSymbol, 1);

    // Literal -> Boolean
    Symbol * rule0bRHS = malloc(sizeof(Symbol));
    rule0bRHS[0] = BooleanSymbol;
    addRule(rules, &count, rule0bRHS, LiteralSymbol, 1);


    // ArithmeticExpression -> AdditiveExpression
    Symbol* rule1RHS = (Symbol*)malloc(sizeof(Symbol));
    rule1RHS[0] = AdditiveExpressionSymbol;
    addRule(rules, &count, rule1RHS, ArithmeticExpressionSymbol, 1);


    // AdditiveExpression -> AdditiveExpression '+' MultiplicativeExpression
    Symbol* rule2RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule2RHS[0] = AdditiveExpressionSymbol;
    rule2RHS[1] = PlusSymbol;
    rule2RHS[2] = MultiplicativeExpressionSymbol;
    addRule(rules, &count, rule2RHS, AdditiveExpressionSymbol, 3);

    // AdditiveExpression -> AdditiveExpression '-' MultiplicativeExpression
    Symbol* rule3RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule3RHS[0] = AdditiveExpressionSymbol;
    rule3RHS[1] = MinusSymbol;
    rule3RHS[2] = MultiplicativeExpressionSymbol;
    addRule(rules, &count, rule3RHS, AdditiveExpressionSymbol, 3);

    // AdditiveExpression -> MultiplicativeExpression
    Symbol* rule4RHS = (Symbol*)malloc(sizeof(Symbol));
    rule4RHS[0] = MultiplicativeExpressionSymbol;
    addRule(rules, &count, rule4RHS, AdditiveExpressionSymbol, 1);


    // MultiplicativeExpression -> MultiplicativeExpression '*' UnaryNegation
    Symbol* rule5RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule5RHS[0] = MultiplicativeExpressionSymbol;
    rule5RHS[1] = StarSymbol;
    rule5RHS[2] = UnaryNegationExpressionSymbol;
    addRule(rules, &count, rule5RHS, MultiplicativeExpressionSymbol, 3);

    // MultiplicativeExpression -> MultiplicativeExpression '/' UnaryNegation
    Symbol* rule6RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule6RHS[0] = MultiplicativeExpressionSymbol;
    rule6RHS[1] = SlashSymbol;
    rule6RHS[2] = UnaryNegationExpressionSymbol;
    addRule(rules, &count, rule6RHS, MultiplicativeExpressionSymbol, 3);


    // MultiplicativeExpression -> UnaryNegation
    Symbol* rule7RHS = (Symbol*)malloc(sizeof(Symbol));
    rule7RHS[0] = UnaryNegationExpressionSymbol;
    addRule(rules, &count, rule7RHS, MultiplicativeExpressionSymbol, 1);


    //NegationExpression -> Minus PrimaryExpression
    Symbol * rule7aRHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule7aRHS[0] = MinusSymbol;
    rule7aRHS[1] = PrimaryExpressionSymbol;
    addRule(rules, &count, rule7aRHS, UnaryNegationExpressionSymbol, 2);

    //NegationExpression -> PrimaryExpression
    Symbol * rule7bRHS = (Symbol*)malloc(sizeof(Symbol));
    rule7bRHS[0] = PrimaryExpressionSymbol;
    addRule(rules, &count, rule7bRHS, UnaryNegationExpressionSymbol, 1);




    // PrimaryExpression -> Number
    Symbol* rule8RHS = (Symbol*)malloc(sizeof(Symbol));
    rule8RHS[0] = LiteralSymbol;
    addRule(rules, &count, rule8RHS, PrimaryExpressionSymbol, 1);


    // PrimaryExpression -> Identifier
    Symbol* rule9RHS = (Symbol*)malloc(sizeof(Symbol));
    rule9RHS[0] = IdentifierSymbol;
    addRule(rules, &count, rule9RHS, PrimaryExpressionSymbol, 1);


    // PrimaryExpression -> '(' Expression ')'
    Symbol* rule10RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule10RHS[0] = OpenParenSymbol;
    rule10RHS[1] = NonRelationalExpressionSymbol;
    rule10RHS[2] = CloseParenSymbol;
    addRule(rules, &count, rule10RHS, PrimaryExpressionSymbol, 3);


    // Number -> Number_Integer
    Symbol* rule11RHS = (Symbol*)malloc(sizeof(Symbol));
    rule11RHS[0] = NumberIntegerSymbol;
    addRule(rules, &count, rule11RHS, NumberSymbol, 1);


    // Number -> Number_Float
    Symbol* rule12RHS = (Symbol*)malloc(sizeof(Symbol));
    rule12RHS[0] = NumberFloatSymbol;
    addRule(rules, &count, rule12RHS, NumberSymbol, 1);

    // VariableDeclarationHelper -> Identifier "Is"
    Symbol* rule13RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule13RHS[0] = IdentifierSymbol;
    rule13RHS[1] = IsSymbol;
    addRule(rules, &count, rule13RHS, VariableDeclarationHelperSymbol, 2);


    // VariableDeclarationHelper2 -> VariableDeclarationHelper Type
    Symbol* rule14RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule14RHS[0] = VariableDeclarationHelperSymbol;
    rule14RHS[1] = TypeSymbol;
    addRule(rules, &count, rule14RHS, VariableDeclarationHelperSymbol2, 2);


    // VariableDeclarationHelper2 -> VariableDeclarationHelper VariableDeclarationHelper2
    Symbol* rule15RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule15RHS[0] = VariableDeclarationHelperSymbol;
    rule15RHS[1] = VariableDeclarationHelperSymbol2;
    addRule(rules, &count, rule15RHS, VariableDeclarationHelperSymbol2, 2);


    // VariableDeclaration -> VariableDeclarationHelper2
    Symbol* rule16RHS = (Symbol*)malloc(sizeof(Symbol));
    rule16RHS[0] = VariableDeclarationHelperSymbol2;
    addRule(rules, &count, rule16RHS, VariableDeclarationSymbol, 1);



    // VariableDeclaration -> InitializedVariableDeclaration
    Symbol* rule17RHS = (Symbol*)malloc(sizeof(Symbol));
    rule17RHS[0] = InitializedVariableDeclarationSymbol;
    addRule(rules, &count, rule17RHS, VariableDeclarationSymbol, 1);


    // InitializedVariableDeclaration -> VariableDeclarationHelper2 "=" Expression
    Symbol* rule18RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule18RHS[0] = VariableDeclarationHelperSymbol2;
    rule18RHS[1] = EqualSymbol;
    rule18RHS[2] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule18RHS, InitializedVariableDeclarationSymbol, 3);

    // Assignment -> Identifier "=" Expression
    Symbol* rule18_2RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule18_2RHS[0] = IdentifierSymbol;
    rule18_2RHS[1] = EqualSymbol;
    rule18_2RHS[2] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule18_2RHS, variableAssignmentSymbol, 3);

    
    

    // Expression -> ArithmeticExpression
    Symbol* rule19RHS = (Symbol*)malloc(sizeof(Symbol));
    rule19RHS[0] = ArithmeticExpressionSymbol;
    addRule(rules, &count, rule19RHS, NonRelationalExpressionSymbol, 1);


    // Expression -> ExpressionStatement;
    Symbol* rule19_2RHS = (Symbol*)malloc(sizeof(Symbol));
    rule19_2RHS[0] = ExpressionStatementSymbol;
    addRule(rules, &count, rule19_2RHS, NonRelationalExpressionSymbol, 1);


    
    //ExpressionStatement -> Assignment
    Symbol * ruleaRHS = (Symbol *)malloc(sizeof(Symbol));
    ruleaRHS[0] = variableAssignmentSymbol;
    addRule(rules, &count, ruleaRHS, ExpressionStatementSymbol, 1);



    // Expression -> InitializedVariableDeclaration
    Symbol* rule20RHS = (Symbol*)malloc(sizeof(Symbol));
    rule20RHS[0] = InitializedVariableDeclarationSymbol;
    addRule(rules, &count, rule20RHS, NonRelationalExpressionSymbol, 1);

    // PrimaryExpression -> FunctionCall
    Symbol* rule20_2RHS = (Symbol*)malloc(sizeof(Symbol));
    rule20_2RHS[0] = FunctionCallSymbol;
    addRule(rules, &count, rule20_2RHS, PrimaryExpressionSymbol, 1);

    // Statements -> Statement
    Symbol* rule21RHS = (Symbol*)malloc(sizeof(Symbol));
    rule21RHS[0] = StatementSymbol;
    addRule(rules, &count, rule21RHS, StatementsSymbol, 1);



    // Statements -> Statement Statements

    Symbol* rule22RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule22RHS[0] = StatementSymbol;
    rule22RHS[1] = StatementsSymbol;
    addRule(rules, &count, rule22RHS, StatementsSymbol, 2);




    // Statement -> PunctuatedStatement 
    Symbol* rule23RHS = (Symbol*)malloc(sizeof(Symbol));
    rule23RHS[0] = PunctuatedStatementSymbol;
    addRule(rules, &count, rule23RHS, StatementSymbol, 1);
  
    // Statement -> FunctionDefinition
    Symbol* rule26RHS = (Symbol*)malloc(sizeof(Symbol));
    rule26RHS[0] = FunctionDefinitionSymbol;
    addRule(rules, &count, rule26RHS, StatementSymbol, 1);

    // BlockStatement -> "<" Statements ">"
    Symbol* rule24RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule24RHS[0] = LeftCurlyBracketSymbol;
    rule24RHS[1] = StatementsSymbol;
    rule24RHS[2] = RightCurlyBracketSymbol;
    addRule(rules, &count, rule24RHS, BlockStatementSymbol, 3);


    // PunctuatedStatement -> PunctuatedStatementHelper '.'
    Symbol* rule25_2RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule25_2RHS[0] = PunctuatedStatementHelperSymbol;
    rule25_2RHS[1] = PeriodSymbol;
    addRule(rules, &count, rule25_2RHS, PunctuatedStatementSymbol, 2);



    // PunctuatedStatementHelper -> VariableDeclaration
    Symbol* rule25RHS = (Symbol*)malloc(sizeof(Symbol));
    rule25RHS[0] = VariableDeclarationSymbol;
    addRule(rules, &count, rule25RHS, PunctuatedStatementHelperSymbol, 1);

    // PunctuatedStatementHelper -> ReturnStatement
    Symbol* rule26_2RHS = (Symbol*)malloc(sizeof(Symbol));
    rule26_2RHS[0] = ReturnStatementSymbol;
    addRule(rules, &count, rule26_2RHS, PunctuatedStatementHelperSymbol, 1);

    // PunctuatedStatementHelper -> ExpressionStatement
    Symbol* rule26_3RHS = (Symbol*)malloc(sizeof(Symbol));
    rule26_3RHS[0] = ExpressionStatementSymbol;
    addRule(rules, &count, rule26_3RHS, PunctuatedStatementHelperSymbol, 1);

    // PunctuatedStatementHelper -> PrintStatement
    Symbol* rule26_4RHS = (Symbol*)malloc(sizeof(Symbol));
    rule26_4RHS[0] = PrintStatementSymbol;
    addRule(rules, &count, rule26_4RHS, PunctuatedStatementHelperSymbol, 1);



    // Type -> "INTEGER"
    Symbol* rule27RHS = (Symbol*)malloc(sizeof(Symbol));
    rule27RHS[0] = TypeIntegerSymbol;
    addRule(rules, &count, rule27RHS, TypeSymbol, 1);


    // TYPE -> "DECIMAL"
    Symbol* rule28RHS = (Symbol*)malloc(sizeof(Symbol));
    rule28RHS[0] = TypeDecimalSymbol;
    addRule(rules, &count, rule28RHS, TypeSymbol, 1);

    //Type -> StringType
    Symbol * rule28aRHS = malloc(sizeof(Symbol));
    rule28aRHS[0] = TypeStringSymbol;
    addRule(rules, &count ,rule28aRHS, TypeSymbol, 1);

    //Type -> BooleanType
    Symbol * rule28bRHS = malloc(sizeof(Symbol));
    rule28bRHS[0] = TypeBooleanSymbol;
    addRule(rules, &count ,rule28bRHS, TypeSymbol, 1);

    //FunctionDefinition -> "Define" FunctionSignature "As" Type "=" Statement
    Symbol* rule29RHS = (Symbol*)malloc(6 * sizeof(Symbol));
    rule29RHS[0] = DefineSymbol;
    rule29RHS[1] = FunctionSignatureSymbol;
    rule29RHS[2] = AsSymbol;
    rule29RHS[3] = TypeSymbol;
    rule29RHS[4] = EqualSymbol;
    rule29RHS[5] = StatementSymbol;
    addRule(rules, &count, rule29RHS, FunctionDefinitionSymbol, 6);


    //Function Signature -> Identifier
    Symbol* rule30RHS = (Symbol*)malloc(sizeof(Symbol));
    rule30RHS[0] = IdentifierSymbol;
    addRule(rules, &count, rule30RHS, FunctionSignatureSymbol, 1);


    //FunctionSignature -> Identifier FunctionSignature
    Symbol* rule31RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule31RHS[0] = IdentifierSymbol;
    rule31RHS[1] = FunctionSignatureSymbol;
    addRule(rules, &count, rule31RHS, FunctionSignatureSymbol, 2);



    //Function Signature -> FunctionArgument
    Symbol* rule32RHS = (Symbol*)malloc(sizeof(Symbol));
    rule32RHS[0] = FunctionArgumentSymbol;
    addRule(rules, &count, rule32RHS, FunctionSignatureSymbol, 1);



    //FunctionSignature -> FunctionArgument FunctionSignature
    Symbol* rule33RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule33RHS[0] = FunctionArgumentSymbol;
    rule33RHS[1] = FunctionSignatureSymbol;
    addRule(rules, &count, rule33RHS, FunctionSignatureSymbol, 2);


    //FunctionArgument -> "<" Type Identifier ">"
    Symbol* rule34RHS = (Symbol*)malloc(4 * sizeof(Symbol));
    rule34RHS[0] = LeftCurlyBracketSymbol;
    rule34RHS[1] = TypeSymbol;
    rule34RHS[2] = IdentifierSymbol;
    rule34RHS[3] = RightCurlyBracketSymbol;
    addRule(rules, &count, rule34RHS, FunctionArgumentSymbol, 4);



    //ReturnStatement -> "Result" "Is" Expression
    Symbol* rule35RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule35RHS[0] = ResultSymbol;
    rule35RHS[1] = IsSymbol;
    rule35RHS[2] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule35RHS, ReturnStatementSymbol, 3);




    //FunctionCallArg -> Expression
    Symbol* rule36RHS = (Symbol*)malloc(sizeof(Symbol));
    rule36RHS[0] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule36RHS, FunctionCallArgumentSymbol, 1);

    //PrintStatement -> Print Expression
    Symbol* rule37RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule37RHS[0] = PrintSymbol;
    rule37RHS[1] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule37RHS, PrintStatementSymbol, 2);




    //Boolean -> True
    Symbol* rule38RHS = (Symbol*)malloc(sizeof(Symbol));
    rule38RHS[0] = TrueSymbol;
    addRule(rules, &count, rule38RHS, BooleanSymbol, 1);


    //Boolean -> False
    Symbol* rule39RHS = (Symbol*)malloc(sizeof(Symbol));
    rule39RHS[0] = FalseSymbol;
    addRule(rules, &count, rule39RHS, BooleanSymbol, 1);


    //LogicalExpression -> OrExpression
    Symbol* rule40RHS = (Symbol*)malloc(sizeof(Symbol));
    rule40RHS[0] = OrExpressionSymbol;
    addRule(rules, &count, rule40RHS, LogicalExpressionSymbol, 1);

    //OrExpression -> OrExpression "Or" AndExpression
    Symbol* rule41RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule41RHS[0] = OrExpressionSymbol;
    rule41RHS[1] = OrSymbol;
    rule41RHS[2] = AndExpressionSymbol;
    addRule(rules, &count, rule41RHS, OrExpressionSymbol, 3);

    //OrExpression -> AndExpression
    Symbol* rule42RHS = (Symbol*)malloc(sizeof(Symbol));
    rule42RHS[0] = AndExpressionSymbol;
    addRule(rules, &count, rule42RHS, OrExpressionSymbol, 1);

    //AndExpression -> AndExpression "And" NotExpression
    Symbol* rule43RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule43RHS[0] = AndExpressionSymbol;
    rule43RHS[1] = AndSymbol;
    rule43RHS[2] = NotExpressionSymbol;
    addRule(rules, &count, rule43RHS, AndExpressionSymbol, 3);

    //AndExpression -> NotExpression
    Symbol* rule44RHS = (Symbol*)malloc(sizeof(Symbol));
    rule44RHS[0] = NotExpressionSymbol;
    addRule(rules, &count, rule44RHS, AndExpressionSymbol, 1);

    //NotExpression -> "Not" PrimaryExpression
    Symbol* rule45RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule45RHS[0] = NotSymbol;
    rule45RHS[1] = PrimaryExpressionSymbol;
    addRule(rules, &count, rule45RHS, NotExpressionSymbol, 2);

    //NotExpression -> PrimaryExpression
    Symbol* rule46RHS = (Symbol*)malloc(sizeof(Symbol));
    rule46RHS[0] = PrimaryExpressionSymbol;
    addRule(rules, &count, rule46RHS, NotExpressionSymbol, 1);

    //Expression -> LogicalExpression
    Symbol* rule47RHS = (Symbol*)malloc(sizeof(Symbol));
    rule47RHS[0] = LogicalExpressionSymbol;
    addRule(rules, &count, rule47RHS, NonRelationalExpressionSymbol, 1);

    

    //IfStatementHelper -> "If" Expression
    Symbol* rule48RHS = (Symbol*)malloc(2 * sizeof(Symbol));
    rule48RHS[0] = IfSymbol;
    rule48RHS[1] = NonRelationalExpressionSymbol;
    addRule(rules, &count, rule48RHS, IfStatementHelperSymbol, 2);

    //IfStatement -> IfStatementHelper "Then" Statement
    Symbol* rule49RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule49RHS[0] = IfStatementHelperSymbol;
    rule49RHS[1] = ThenSymbol;
    rule49RHS[2] = StatementSymbol;
    addRule(rules, &count, rule49RHS, IfStatementSymbol, 3);

    //IfElseStatement -> IfStatement "Else" Statement
    Symbol* rule50RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule50RHS[0] = IfStatementSymbol;
    rule50RHS[1] = ElseSymbol;
    rule50RHS[2] = StatementSymbol;
    addRule(rules, &count, rule50RHS, IfElseStatementSymbol, 3);

    //Statement -> BlockStatement
    Symbol* rule51RHS = (Symbol*)malloc(sizeof(Symbol));
    rule51RHS[0] = BlockStatementSymbol;
    addRule(rules, &count, rule51RHS, StatementSymbol, 1);
    
    //Statement -> IfStatement
    Symbol* rule52RHS = (Symbol*)malloc(sizeof(Symbol));
    rule52RHS[0] = IfStatementSymbol;
    addRule(rules, &count, rule52RHS, StatementSymbol, 1);


    //Statement -> IfElseStatement
    Symbol* rule53RHS = (Symbol*)malloc(sizeof(Symbol));
    rule53RHS[0] = IfElseStatementSymbol;
    addRule(rules, &count, rule53RHS, StatementSymbol, 1);





    //whileStatement -> "While" Expression Statement
    Symbol* rule54RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule54RHS[0] = WhileSymbol;
    rule54RHS[1] = NonRelationalExpressionSymbol;
    rule54RHS[2] = StatementSymbol;
    addRule(rules, &count, rule54RHS, WhileStatementSymbol, 3);

   


   //statement -> whileStatement
    Symbol* rule55RHS = (Symbol*)malloc(sizeof(Symbol));
    rule55RHS[0] = WhileStatementSymbol;
    addRule(rules, &count, rule55RHS, StatementSymbol, 1);



    //UntilStatement -> "Until" Expression Statement
    Symbol* rule56RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule56RHS[0] = UntilSymbol;
    rule56RHS[1] = NonRelationalExpressionSymbol;
    rule56RHS[2] = StatementSymbol;
    addRule(rules, &count, rule56RHS, UntilStatementSymbol, 3);

   


   //statement -> UntilStatement
    Symbol* rule57RHS = (Symbol*)malloc(sizeof(Symbol));
    rule57RHS[0] = UntilStatementSymbol;
    addRule(rules, &count, rule57RHS, StatementSymbol, 1);



    //RelationalExpression -> ArithmeticExpression RelationalOperator ArithmeticExpression
    Symbol* rule58RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule58RHS[0] = ArithmeticExpressionSymbol;
    rule58RHS[1] = RelationalOperatorSymbol;
    rule58RHS[2] = ArithmeticExpressionSymbol;
    addRule(rules, &count, rule58RHS, RelationalExpressionSymbol, 3);

    //RelationalExpression -> ArithmeticExpression RelationalOperator RelationalExpression
    Symbol* rule59RHS = (Symbol*)malloc(3 * sizeof(Symbol));
    rule59RHS[0] = ArithmeticExpressionSymbol;
    rule59RHS[1] = RelationalOperatorSymbol;
    rule59RHS[2] = RelationalExpressionSymbol;
    addRule(rules, &count, rule59RHS, RelationalExpressionSymbol, 3);

    //RelationalOperator -> LessSymbol
    Symbol* rule60RHS = (Symbol*)malloc(sizeof(Symbol));
    rule60RHS[0] = LessSymbol;
    addRule(rules, &count, rule60RHS, RelationalOperatorSymbol, 1);

    //RelationalOperator -> LessEqualSymbol
    Symbol* rule61RHS = (Symbol*)malloc(sizeof(Symbol));
    rule61RHS[0] = LessEqualSymbol;
    addRule(rules, &count, rule61RHS, RelationalOperatorSymbol, 1);

    //RelationalOperator -> GreaterSymbol
    Symbol* rule62RHS = (Symbol*)malloc(sizeof(Symbol));
    rule62RHS[0] = GreaterSymbol;
    addRule(rules, &count, rule62RHS, RelationalOperatorSymbol, 1);

    //RelationalOperator -> GreaterEqualSymbol
    Symbol* rule63RHS = (Symbol*)malloc(sizeof(Symbol));
    rule63RHS[0] = GreaterEqualSymbol;
    addRule(rules, &count, rule63RHS, RelationalOperatorSymbol, 1);

    //NonRelationalExpressionSymbol -> RelationalExpressionSymbol
    Symbol* rule64RHS = (Symbol*)malloc(sizeof(Symbol));
    rule64RHS[0] = RelationalExpressionSymbol;
    addRule(rules, &count, rule64RHS, NonRelationalExpressionSymbol, 1);



    *numRules = count;
    return rules;
}




char * wordFromTypeSymbol(Symbol sym){
    if(compareSymbols(&sym, &TypeIntegerSymbol)) return "Integer";
    else if(compareSymbols(&sym, &TypeDecimalSymbol)) return "Decimal";
    else if(compareSymbols(&sym, &TypeStringSymbol)) return "String";
    else if(compareSymbols(&sym, &TypeBooleanSymbol)) return "Boolean";
    else exit(-1);
}


char * wordFromDataType(DataType type){
    if(type == TYPE_INTEGER) return "Integer";
    else if(type == TYPE_DECIMAL) return "Decimal";
    else if (type == TYPE_STRING) return "String";
    else if (type == TYPE_BOOLEAN) return "Boolean";
    else exit(-1);
}
  

DataType dataTypeFromTypeSymbol(Symbol sym){
    if(compareSymbols(&sym, &TypeIntegerSymbol)) return TYPE_INTEGER;
    if(compareSymbols(&sym, &NumberIntegerSymbol)) return TYPE_INTEGER;


    if(compareSymbols(&sym, &TypeDecimalSymbol)) return TYPE_DECIMAL;
    if(compareSymbols(&sym, &NumberFloatSymbol)) return TYPE_DECIMAL;

    if(compareSymbols(&sym, &TypeStringSymbol)) return TYPE_STRING;
    if(compareSymbols(&sym, &StringSymbol)) return TYPE_STRING;

    if(compareSymbols(&sym, &TypeBooleanSymbol)) return TYPE_BOOLEAN;
    if(compareSymbols(&sym, &TrueSymbol)) return TYPE_BOOLEAN;
    if(compareSymbols(&sym, &FalseSymbol)) return TYPE_BOOLEAN;


    exit(1);

}




/////////////////////////////////////////////////////////
///////////// Print  FUnctions //////////////////////////
/////////////////////////////////////////////////////////



void printSymbol(Symbol symbol) {
    const char* terminalNames[] = {

        "Type_Integer",
        "Type_Decimal",
        "Type_String",
        "Type_Boolean",
        "Is",
        "Define",
        "As",
        "Result",
        "Print",
        "True",
        "False",
        "Or", 
        "And", 
        "Not",
        "If", 
        "Then",
        "Else",
        "While",
        "Until",





        "Plus",
        "Minus",
        "Star",
        "Slash",
        "Equals",
        "Greater",
        "Less",
        "GreaterEqual",
        "LessEqual",

        "Period",
        "Left Pointy",
        "Right Pointy",
        "OpenParen",
        "CloseParen",
        "Double Quote",


        "DoubleSlash",
        "Percent",
        "Caret",

        "Number_Integer",
        "Number_Float",
        "String",


       
        
        "Xor",
        "Nor",
       
        "Nand",
       


        "Identifier",
        "PlusEquals",
        "MinusEquals",
        "StarEquals",
        "SlashEquals",
        "DoubleSlashEquals",
        "PercentEquals",
        "CaretEquals",
        "End",
        "Invalid",
        "Function Keyword",



    };

    const char* nonTerminalNames[] = {

        "Number",
        "ArithmeticExpression",
        "AdditiveExpression",
        "MultiplicativeExpression",
        "ExponentiationExpression",
        "UnaryNegationExpression",
        "PrimaryExpression",
        "Expression",
        "LogicalExpression",
        "OrExpression",
        "AndExpression",
        "NotExpression",
        "PrimaryLogicalExpression",
        "RelationalExpression",
        "AssignmentExpression",
        "CompoundAssignmentOperator",
        "Start",
        "InitializedVariableDeclaration",
        "Statement",
        "VariableDeclaration",
        "Type",
        "VariableDeclarationHelper",
        "VariableDeclarationHelper2",
        "Statements",
        "PunctuatedStatement",
        "PunctuatedStatementHelper",
        "BlockStatement",

        "FunctionDefinition",
        "FunctionSignature",
        "FunctionArgument",
        "ReturnStatement",
        "FunctionCall",
        "FunctionCallArg", 
        "Literal",

        "ExpressionStatement",
        "AssignmentHelper",
        "Print Statement",
        "Boolean",

        "If Statement",
        "If Statement Helper",
        "If Else Statement",
        "While Statment",
        "Until Statement",

        "Expression",
        "Relational Operator",
        "Relational Expression",






    };









    if(symbol.isHelper){

        //printf("helper symbol int = %d  ", symbol.nonTerminal);
        return;
    }




    if (symbol.isTerminal) {
        //printf("Terminal: %s  ", terminalNames[symbol.terminal]);
    } else {
        //printf("Non-terminal: %s  ", nonTerminalNames[symbol.nonTerminal]);
    }


    if(compareSymbols(&symbol, &StringSymbol)){
        char * val = symbol.value;
        //printf("Val = %s  ", val);
    }



    if(compareSymbols(&symbol, &FunctionCallArgumentSymbol)){
        char * val = (char*)symbol.value;
        //printf("Val = %s  ", val);
    }


    if(compareSymbols(&symbol, &FunctionKeywordSymbol)){
        char * val = (char*)symbol.value;
        //printf("Val = %s   ", val);
    }


    if(compareSymbols(&symbol, &NumberIntegerSymbol)){
        int val = *(int*)symbol.value;
        //printf(" Val = %d  ", val);
    }

    if(compareSymbols(&symbol, &NumberFloatSymbol)){
        double val = *(double*)symbol.value;
        //printf("  Val = %lf  ", val);
    }

    if(compareSymbols(&symbol, &IdentifierSymbol)){
        char * val = (char *)symbol.value;
        //printf("  Val = %s  ", val);
    }
}


/*

void printSymbolSet(Set * set){

    //printf("about to print symbol set\n");
    SetNode * node = set->head;
    while(node != NULL){

        Symbol * sym = (Symbol *)node->data;
        //printf(" ");
        printSymbol(*sym);
    }

    //printf("done printing symbol set\n");
}

void _printRule(Rule rule, int dot_position) {
    //printf("LHS: ");
    printSymbol(rule.lhs);
    //printf("\n");

    //printf("RHS: ");
    for (int i = 0; i < rule.rhs_len; i++) {
        if (i == dot_position) {
            //printf("     .      ");
        }
        printSymbol(rule.rhs[i]);
    }

    // Print the dot at the end if the dot position is at the end of RHS
    if (dot_position == rule.rhs_len) {
        //printf("     .      ");
    }

    //printf("\n\n");
}

void printRule(Rule rule){
    _printRule(rule, -1);
}



void printGrammarItemSet(Set * set){
    //printf("printing grammar Item set\n");
    SetNode * node = set->head;
    while(node != NULL){

        GrammarItem * item = (GrammarItem*)node->data;
        printGrammarItem(*item);
        node = node->next;
    }
}


void printStateSet(Set* stateSet) {
    SetNode* currentNode = stateSet->head;

    //printf("State Set: { ");
    while (currentNode != NULL) {
        State * state = (State*)currentNode->data;
        printState(*state);
        currentNode = currentNode->next;
    }
    //printf("}\n");
}



void printActionTable(LRTable* table) {
    //printf("ACTION Table:\n");

    // Print column headers (terminals)
    //printf("%-4s", "");
    for (int i = 0; i < table->numStates; i++) {
        //printf("%-4d", i);
    }
    //printf("\n");

    // Print horizontal line for table
    for (int i = 0; i <= table->numStates; i++) {
        //printf("----");
    }
    //printf("\n");

    // Print row headers (states) and corresponding entries in the ACTION table
    for (int i = 0; i < (table->numTerminals + table->numNonTerminals); i++) {
        //printf("%-4d|", i);
        for (int j = 0; j < (table->numStates); j++) {
            Action action = table->actionTable[i][j];
            switch (action.type) {
                case ACTION_SHIFT:
                    //printf("S%-3d", action.value);
                    break;
                case ACTION_REDUCE:
                    //printf("R%-3d", action.value);
                    break;
                case ACTION_ACCEPT:
                    //printf("%-4s", "A");
                    break;
                default:
                    //printf("%-4s", "E");
            }
        }
        //printf("\n");
    }
}


*/




/*
int main() {




    int numRules = 8;
    Rule * rules = createRules2(&numRules);
   // Rule * rules = createRules(&numRules);

    int total = 1;
    for(int i = 0; i < numRules; i++){
        int n = rules[i].rhs_len;
        total *= n;
    }

    LRTable * table = createLRTable(30, numTerminals, numNonTerminals);

    printGOTOTable(table);
    //printStateSet(canonSet);


 
    

       Set * canonSet = generateCanonicalCollectionAndFillGOTOTable(rules, numRules, table);

        fillActionTable(canonSet, rules, numRules, table);



        //printf("\n\n\n about to print the gototable:\n");
        printGOTOTable(table);


        //printf("\n\n\n about to print action table\n\n");
        printActionTable(table);

     

    return 0;

}

*/
