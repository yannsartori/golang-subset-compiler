%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalEnum.h"
#include "ast.h"

int yylex();
extern int yylineno;

void yyerror(char const *s) {
	fprintf(stderr, "Error: %s on line %d\n", s, yylineno);
	exit(1);
}

%}

%define parse.error verbose

%code requires
{
	#include "ast.h"
}
//need to implement their builtins
//denali provided
%union {
	int intval;
	double floatval;
	char runeval;
	char *stringval;
	char *identifier;
	struct Exp *exp;
	struct ExpList *explist;
	RootNode* rootNode;
	TopDeclarationNode* topDeclNode;
}
%token tLOGICOR tLOGICAND tEQ tNEQ tGEQ tLEQ tBShiftLeft tBShiftRight tAndNot tLENGTH tCAP tAPPEND tBreak tDefault tFunc tInterface tSelect tCase tDefer tGo tMap tStruct tChan tElse tGoto tPackage tSwitch tConst tFallthrough tIf tRange tType tContinue tFor tImport tReturn tVar tPrint tPrintln tPlusEq tAndEq tMinusEquals tOrEquals tTimesEquals tHatEquals tLessMinus tDivideEquals tLShiftEquals tIncrement tDefined tModEquals tRShiftEquals tDecrement tElipses tAndHatEquals 
%token <intval>	   tINTLIT
%token <floatval>  tFLOATLIT
%token <runeval> tRUNELIT
%token <stringval> tRAWSTRINGLIT tINTERPRETEDSTRINGLIT
%token <boolval> tBOOLVAL
%token <identifier> tIDENTIFIER
%type <exp> expression operand literal conversion index selector appendExpression lengthExpression capExpression simpleType primaryExpression 
%type <explist> expressionList arguments
%type <rootNode> root
%type <topDeclNode> variableDecl typeDecl funcDecl topDeclarationList
%left tLOGICOR
%left tLOGICAND
%left tEQ tNEQ tGEQ tLEQ '>' '<'
%left '+' '-' '|' '^'
%left '*' '/' '%' tBShiftLeft tBShiftRight '&' tAndNot
%left '!' UNARY



%start root

%%

root			: tPackage tIDENTIFIER ';' topDeclarationList {$$ = makeRootNode($2, $4);}
;

topDeclarationList	: 					{$$ = NULL;}
			| variableDecl topDeclarationList	
			| typeDecl topDeclarationList	
			| funcDecl topDeclarationList	
;

variableDecl		: tVar singleVarDecl ';'		
			| tVar '(' innerVarDecls ')' ';'	
			| tVar '(' ')' ';'			
;

innerVarDecls		: singleVarDecl
			| singleVarDecl ';'
			| singleVarDecl ';' innerVarDecls
;

singleVarDecl		: identifierList declType '=' expressionList
			| identifierList '=' expressionList
			| identifierList declType
;

typeDecl		: tType singleTypeDecl ';'
			| tType '(' innerTypeDecls ')' ';'
			| tType '(' ')' ';'
;

innerTypeDecls	: singleTypeDecl
			| singleTypeDecl ';'
			| singleTypeDecl ';' innerTypeDecls
;

singleTypeDecl	: identifierList declType
;

funcDecl		: tFunc tIDENTIFIER '(' funcArgDecls ')' declType block
			| tFunc tIDENTIFIER '(' ')' declType block
			| tFunc tIDENTIFIER '(' funcArgDecls ')' block
			| tFunc tIDENTIFIER '(' ')' block
;

funcArgDecls		: identifierList declType funcArgDecls
			| identifierList declType
;

declType		: tIDENTIFIER
			| sliceDeclType
			| arrayDeclType
			| structDeclType
;

sliceDeclType		: '[' ']' tIDENTIFIER;
arrayDeclType		: index tIDENTIFIER;
structDeclType	: tStruct '{' innerTypeDecls '}' ';'
			| tStruct '{' '}' ';'
;

identifierList	: tIDENTIFIER
			| tIDENTIFIER ',' identifierList
;

expression: /* unrolled https://golang.org/ref/spec#Expression with precdence directives*/
			  primaryExpression { $$ = $1; }
			| expression '*' expression { $$ = makeExpBinary($1, $3, expKindMultiplication); } /*2.9.5*/
			| expression '/' expression { $$ = makeExpBinary($1, $3, expKindDivision); }
			| expression '%' expression { $$ = makeExpBinary($1, $3, expKindMod); }
			| expression tBShiftLeft expression { $$ = makeExpBinary($1, $3, expKindBitShiftLeft); }
			| expression tBShiftRight expression { $$ = makeExpBinary($1, $3, expKindBitShiftRight); }	
			| expression '&' expression { $$ = makeExpBinary($1, $3, expKindBitAnd); }	
			| expression tAndNot expression { $$ = makeExpBinary($1, $3, expKindBitAndNot); }	
			| expression '+' expression { $$ = makeExpBinary($1, $3, expKindAddition); }	
			| expression '-' expression { $$ = makeExpBinary($1, $3, expKindSubtraction); }	
			| expression '|' expression { $$ = makeExpBinary($1, $3, expKindBitOr); }	
			| expression '^' expression { $$ = makeExpBinary($1, $3, expKindBitNotBinary); }	
			| expression tEQ expression { $$ = makeExpBinary($1, $3, expKindEQ); }	
			| expression tNEQ expression { $$ = makeExpBinary($1, $3, expKindNEQ); }	
			| expression '<' expression { $$ = makeExpBinary($1, $3, expKindLess); }	
			| expression tLEQ expression { $$ = makeExpBinary($1, $3, expKindLEQ); }	
			| expression '>' expression { $$ = makeExpBinary($1, $3, expKindGreater); }	
			| expression tGEQ expression { $$ = makeExpBinary($1, $3, expKindGEQ); }	
			| expression tLOGICAND expression { $$ = makeExpBinary($1, $3, expKindLogicAnd); }	
			| expression tLOGICOR expression { $$ = makeExpBinary($1, $3, expKindLogicOr); }	
			| '+' expression %prec UNARY { $$ = makeExpUnary($2, expKindUnaryPlus); } /*2.9.4*/
			| '-' expression %prec UNARY { $$ = makeExpUnary($2, expKindUnaryMinus); }
			| '!' expression { $$ = makeExpUnary($2, expKindLogicNot); } 
			| '^' expression %prec UNARY { $$ = makeExpUnary($2, expKindBitNotUnary); }
			;
expressionList: 
			  expression { $$ = createArgumentList($1); }
			| expressionList ',' expression { $$ = addArgument($1, $3); } /*Reversed!!!*/
			; //maybe Neil provided this?
primaryExpression: 
			  operand { $$ = $1; }
			| conversion { $$ = $1;}
			| primaryExpression selector { $$ = makeExpAccess($1, $2, expKindFieldSelect); } 
			| primaryExpression index { $$ = makeExpAccess($1, $2, expKindIndexing); }  
			| primaryExpression arguments { $$ = makeExpFuncCall($1, $2, expKindFuncCall); }/*2.9.6*/ 
			| appendExpression { $$ = $1; } 
			| lengthExpression { $$ = $1; }
			| capExpression { $$ = $1; }
			; //ommitting method expression, slice indexing, type assertion
operand: 
			  literal { $$ = $1; } /*2.9.3*/
			| tIDENTIFIER { $$ = makeExpIdentifier($1); } /*2.9.2*/ 
			| '(' expression ')' { $$ = $2; }
			; /*2.9.1*/ 
			//ommitting qualified lit, composite lit, function lit
literal:	
			  tINTLIT { $$ = makeExpIntLit($1); } 
			| tFLOATLIT { $$ = makeExpFloatLit($1); }  
			| tRUNELIT { $$ = makeExpRuneLit($1); }  
			| tRAWSTRINGLIT { $$ = makeExpStringLit(expKindRawStringLit, $1); }  
			| tINTERPRETEDSTRINGLIT { $$ = makeExpStringLit(expKindInterpretedStringLit, $1); }  /*2.9.3*/
			;
conversion: /* I ommitted the trailing ',' */
			simpleType '(' expression ')' { $$ = makeExpFuncCall($1, createArgumentList($3), expKindTypeCast); } /*also fix type pls */
			; /*2.9.10*/ //types also provided by Denali
index: '[' expression ']' { $$ = $2; }/*2.9.7*/
arguments: 
			  '(' expressionList ')' { $$ = $2;}
			| '(' ')' { $$ = createArgumentList(NULL); }/*2.9.6*/
			;
selector: '.' tIDENTIFIER { $$ = makeExpIdentifier($2); }; /*2.9.8-- Should we weed this ? "must not be the blank identifier" Also we should make sure we can do selector assignment e.g. a.b = c*/
appendExpression: tAPPEND '(' expression ',' expression ')' { $$ = makeExpAppend($3, $5); }; /*2.9.9*/
lengthExpression: tLENGTH '(' expression ')' { $$ = makeExpBuiltInBody($3, expKindLength); }; /*2.9.9*/
capExpression: tCAP '(' expression ')' { $$ = makeExpBuiltInBody($3, expKindCapacity); }; /*2.9.9*/
simpleType: 		'b' { $$ = makeExpIdentifier("b"); }; //placeholder
block: 		'{' '}' //placeholder

