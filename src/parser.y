%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex();
extern int yylineno;

%}

%define parse.error verbose
//need to implement their builtins
%union {
	int intval;
	int boolval;
	double floatval;
	char runeval;
	char *stringval;
	char *identifier;
}
%token tLOGICOR tLOGICAND tEQ tNEQ tGEQ tLEQ tBShiftLeft tBShiftRight tAndNot tLENGTH tCAP tAPPEND 
%token <intval>	   tINTLIT
%token <floatval>  tFLOATLIT
%token <runeval> tRUNELIT
%token <stringval> tRAWSTRINGLIT tINTERPRETEDSTRINGLIT
%token <boolval> tBOOLVAL
%token <identifier> tIDENTIFIER
%left tLOGICOR
%left tLOGICAND
%left tEQ tNEQ tGEQ tLEQ '>' '<'
%left '+' '-' '|' '^'
%left '*' '/' '%' tBShiftLeft tBShiftRight '&' tAndNot
%left '!' UNARY



%start expression

%%
expression: 
			  primaryExpression /* unrolled https://golang.org/ref/spec#Expression with precdence directives*/
			| expression '*' expression /*2.9.5*/
			| expression '/' expression
			| expression '%' expression
			| expression tBShiftLeft expression
			| expression tBShiftRight expression	
			| expression '&' expression	
			| expression tAndNot expression	
			| expression '+' expression	
			| expression '-' expression	
			| expression '|' expression	
			| expression '^' expression	
			| expression tEQ expression	
			| expression tNEQ expression	
			| expression '<' expression	
			| expression tLEQ expression	
			| expression '>' expression	
			| expression tGEQ expression	
			| expression tLOGICAND expression	
			| expression tLOGICOR expression	
			| '+' expression %prec UNARY /*2.9.4*/
			| '-' expression %prec UNARY
			| '!' expression 
			| '^' expression %prec UNARY
			;
expressionList: expression | expression ',' expressionList; //maybe Neil provided this?
primaryExpression: 
			  operand 
			| conversion 
			| primaryExpression selector 
			| primaryExpression index 
			| primaryExpression arguments /*2.9.6*/ 
			| appendExpression 
			| lengthExpression 
			| capExpression; //ommitting method expression, slice indexing, type assertion
operand: 
			  literal /*2.9.3*/ 
			| tIDENTIFIER /*2.9.2*/ 
			| '(' expression ')'; /*2.9.1*/ //ommitting qualified lit, composite lit, function lit
literal:	tINTLIT | tFLOATLIT | tRUNELIT | tRAWSTRINGLIT | tINTERPRETEDSTRINGLIT /*2.9.3*/;
conversion: 
			  type '(' expression ',' ')' 
			| type '(' expression ')'; /*2.9.10*/ //see if lists are actually supported types also provided by Denali
index: '[' expression ']' /*2.9.7*/
arguments: 
			  '(' expressionList ')' 
			| '(' type ',' expressionList ')' 
			| '(' type ')' 
			| '(' ')' /*2.9.6... ask about the type ones!*/
selector: '.' tIDENTIFIER ; /*2.9.8-- Should we weed this ? "must not be the blank identifier" Also we should make sure we can do selector assignment e.g. a.b = c*/
appendExpression: tAPPEND '(' expression ',' expression ')'; /*2.9.9*/
lengthExpression: tLENGTH '(' expression ')'; /*2.9.9*/
capExpression: tCAP '(' expression ')'; /*2.9.9*/

type: 'b';//placeholder

