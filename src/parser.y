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

void builtInBlankError(char * func)
{
	fprintf(stderr, "Error: (line %d) Cannot use the blank identifier in the %s function.\n", yylineno, func);
	exit(1);
}
void argumentBlankError()
{
	fprintf(stderr, "Error: (line %d) Cannot cast the blank identifier or use it as an argument to a function call.\n", yylineno);
	exit(1);
}
void fieldSelectBlankError()
{
	fprintf(stderr, "Error: (line %d) Cannot use the blank identifier as a field selector.\n", yylineno);
	exit(1);
}
void indexingBlankError()
{
	fprintf(stderr, "Error: (line %d) Cannot use the blank identifier as an index\n", yylineno);
	exit(1);
}

%}

%define parse.error verbose

%code requires
{
	#include "ast.h"
}
%union {
	int intval;
	double floatval;
	char *runeval;
	char *stringval;
	char *identifier;
	struct Exp *exp;
	struct ExpList *explist;
	RootNode* rootNode;
	TopDeclarationNode* topDeclNode;
	VarDeclNode* varDeclNode;
	TypeDeclNode* typeDeclNode;
	FuncDeclNode* funcDeclNode;
	TypeHolderNode* declType;
	IdChain* tempIdChain;
}
%token tLOGICOR tLOGICAND tEQ tNEQ tGEQ tLEQ tBShiftLeft tBShiftRight tAndNot tLENGTH tCAP tAPPEND tBreak tDefault tFunc tInterface tSelect tCase tDefer tGo tMap tStruct tChan tElse tGoto tPackage tSwitch tConst tFallthrough tIf tRange tType tContinue tFor tImport tReturn tVar tPrint tPrintln tPlusEq tAndEq tMinusEquals tOrEquals tTimesEquals tHatEquals tLessMinus tDivideEquals tLShiftEquals tIncrement tDefined tModEquals tRShiftEquals tDecrement tElipses tAndHatEquals 
%token <intval> tINTLIT
%token <floatval>  tFLOATLIT
%token <runeval> tRUNELIT
%token <stringval> tRAWSTRINGLIT tINTERPRETEDSTRINGLIT
%token <boolval> tBOOLVAL
%token <identifier> tIDENTIFIER
%type <exp> expression operand literal index selector appendExpression lengthExpression capExpression primaryExpression 
%type <explist> expressionList arguments
%type <rootNode> root
%type <topDeclNode> topDeclarationList
%type <varDeclNode> variableDecl singleVarDecl innerVarDecls
%type <typeDeclNode> typeDecl singleTypeDecl innerTypeDecls funcArgDecls
%type <funcDeclNode> funcDecl
%type <declType> declType sliceDeclType arrayDeclType structDeclType
%type <tempIdChain> identifierList
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
			| variableDecl topDeclarationList	{$$ = makeTopVarDecl($1, $2);}
			| typeDecl topDeclarationList	{$$ = makeTopTypeDecl($1, $2);}
			| funcDecl topDeclarationList	{$$ = makeTopFuncDecl($1, $2);}
;

variableDecl		: tVar singleVarDecl ';'		{$$ = $2;}
			| tVar '(' innerVarDecls ')' ';'	{$$ = $3;}
			| tVar '(' ')' ';'			{$$ = NULL;}
;

innerVarDecls		: singleVarDecl				{$$ = $1;}
			| singleVarDecl ';'				{$$ = $1;}
			| singleVarDecl ';' innerVarDecls		{$$ = $1, appendVarDecls($$, $3);}
;

singleVarDecl		: identifierList declType '=' expressionList
							{$$ = makeSingleVarDeclWithExps($1, $2, $4);}
			| identifierList '=' expressionList
							{$$ = makeSingleVarDeclWithExps($1, NULL, $3);}
			| identifierList declType	
							{$$ = makeSingleVarDeclNoExps($1, $2);}
;

typeDecl		: tType singleTypeDecl ';'			{$$ = $2;}
			| tType '(' innerTypeDecls ')' ';'		{$$ = $3;}
			| tType '(' ')' ';'				{$$ = NULL;}
;

innerTypeDecls	: singleTypeDecl				{$$ = $1;}
			| singleTypeDecl ';'				{$$ = $1;}
			| singleTypeDecl ';' innerTypeDecls	{appendTypeDecls($1, $3); $$ = $1;}
;

singleTypeDecl	: identifierList declType			{$$ = makeSingleTypeDecl($1, $2);}
;

funcDecl		: tFunc tIDENTIFIER '(' funcArgDecls ')' declType block ';'
						{$$ = makeFuncDecl($2, $4, $6, NULL);}
			| tFunc tIDENTIFIER '(' ')' declType block ';'
						{$$ = makeFuncDecl($2, NULL, $5, NULL);}
			| tFunc tIDENTIFIER '(' funcArgDecls ')' block ';'
						{$$ = makeFuncDecl($2, $4, NULL, NULL);}
			| tFunc tIDENTIFIER '(' ')' block ';'
						{$$ = makeFuncDecl($2, NULL, NULL, NULL);}
;

funcArgDecls		: singleTypeDecl ',' funcArgDecls		{appendTypeDecls($1, $3); $$ = $1;}
			| singleTypeDecl				{$$ = $1;}
;

declType		: tIDENTIFIER					{$$ = makeIdTypeHolder($1);}
			| sliceDeclType				{$$ = $1;}
			| arrayDeclType				{$$ = $1;}
			| structDeclType				{$$ = $1; }
;

sliceDeclType		: '[' ']' tIDENTIFIER			{$$ = makeSliceHolder($3);}
;
arrayDeclType		: index tIDENTIFIER				{$$ = makeArrayHolder($1, $2);}
;
structDeclType	: tStruct '{' innerTypeDecls '}'		{$$ = makeStructHolder($3);}
			| tStruct '{' '}'				{$$ = makeStructHolder(NULL);}
;

identifierList	: tIDENTIFIER					{$$ = makeIdChain($1, NULL);}
			| tIDENTIFIER ',' identifierList		{$$ = makeIdChain($1, $3);}
;

block			: '{' '}'
;

expression:
			  primaryExpression								{ $$ = $1; }
			| expression '*' expression						{ $$ = makeExpBinary($1, $3, expKindMultiplication); } /*2.9.5*/
			| expression '/' expression						{ $$ = makeExpBinary($1, $3, expKindDivision); }
			| expression '%' expression						{ $$ = makeExpBinary($1, $3, expKindMod); }
			| expression tBShiftLeft expression				{ $$ = makeExpBinary($1, $3, expKindBitShiftLeft); }
			| expression tBShiftRight expression			{ $$ = makeExpBinary($1, $3, expKindBitShiftRight); }	
			| expression '&' expression						{ $$ = makeExpBinary($1, $3, expKindBitAnd); }	
			| expression tAndNot expression					{ $$ = makeExpBinary($1, $3, expKindBitAndNot); }	
			| expression '+' expression						{ $$ = makeExpBinary($1, $3, expKindAddition); }	
			| expression '-' expression						{ $$ = makeExpBinary($1, $3, expKindSubtraction); }	
			| expression '|' expression						{ $$ = makeExpBinary($1, $3, expKindBitOr); }	
			| expression '^' expression						{ $$ = makeExpBinary($1, $3, expKindBitNotBinary); }	
			| expression tEQ expression						{ $$ = makeExpBinary($1, $3, expKindEQ); }	
			| expression tNEQ expression					{ $$ = makeExpBinary($1, $3, expKindNEQ); }	
			| expression '<' expression						{ $$ = makeExpBinary($1, $3, expKindLess); }	
			| expression tLEQ expression					{ $$ = makeExpBinary($1, $3, expKindLEQ); }	
			| expression '>' expression						{ $$ = makeExpBinary($1, $3, expKindGreater); }	
			| expression tGEQ expression					{ $$ = makeExpBinary($1, $3, expKindGEQ); }	
			| expression tLOGICAND expression				{ $$ = makeExpBinary($1, $3, expKindLogicAnd); }	
			| expression tLOGICOR expression				{ $$ = makeExpBinary($1, $3, expKindLogicOr); }	
			| '+' expression %prec UNARY					{ $$ = makeExpUnary($2, expKindUnaryPlus); } /*2.9.4*/
			| '-' expression %prec UNARY					{ $$ = makeExpUnary($2, expKindUnaryMinus); }
			| '!' expression								{ $$ = makeExpUnary($2, expKindLogicNot); } 
			| '^' expression %prec UNARY					{ $$ = makeExpUnary($2, expKindBitNotUnary); }
			;

expressionList: 
			  expression									{ $$ = createArgumentList($1); }
			| expressionList ',' expression					{ $$ = addArgument($1, $3); } /*gets reversed in add argument */
			; 
primaryExpression: 
			  operand										{ $$ = $1; }
			| primaryExpression selector					{ $$ = makeExpAccess($1, $2, expKindFieldSelect); } 
			| primaryExpression index						{ $$ = makeExpAccess($1, $2, expKindIndexing); }  
			| primaryExpression arguments					{ $$ = makeExpFuncCall($1, $2); }/*2.9.6, 2.9.10*/ 
			| appendExpression								{ $$ = $1; } 
			| lengthExpression								{ $$ = $1; }
			| capExpression									{ $$ = $1; }
			; 
operand: 
			  literal										{ $$ = $1; } /*2.9.3*/
			| tIDENTIFIER									{ $$ = makeExpIdentifier($1); } /*2.9.2*/ 
			| '(' expression ')'							{ $$ = $2; }
			; /*2.9.1*/ 
			
literal:	
			  tINTLIT										{ $$ = makeExpIntLit($1); } 
			| tFLOATLIT										{ $$ = makeExpFloatLit($1); }  
			| tRUNELIT										{ $$ = makeExpRuneLit($1); }  
			| tRAWSTRINGLIT									{ $$ = makeExpStringLit(expKindRawStringLit, $1); }  
			| tINTERPRETEDSTRINGLIT							{ $$ = makeExpStringLit(expKindInterpretedStringLit, $1); }  /*2.9.3*/
			;

			
index:		  '[' expression ']'			{ if ( isBlank($2) ) indexingBlankError(); $$ = $2; }/*2.9.7*/

arguments: 
			  '(' expressionList ')'						{ if ( containsBlank($2) ) argumentBlankError(); $$ = $2;}
			| '(' ')'										{ $$ = createArgumentList(NULL); }/*2.9.6*/
			;

			
selector:		  '.' tIDENTIFIER							{ if ( strcmp($2, "_") == 0 ) fieldSelectBlankError(); $$ = makeExpIdentifier($2); }; /*2.9.8*/
appendExpression: tAPPEND '(' expression ',' expression ')' { if ( isBlank($3) || isBlank($5) ) builtInBlankError("append"); $$ = makeExpAppend($3, $5); }; /*2.9.9*/
lengthExpression: tLENGTH '(' expression ')'				{ if ( isBlank($3) ) builtInBlankError("length"); $$ = makeExpBuiltInBody($3, expKindLength); }; /*2.9.9*/
capExpression:	  tCAP '(' expression ')'					{ if ( isBlank($3) ) builtInBlankError("capacity"); $$ = makeExpBuiltInBody($3, expKindCapacity); }; /*2.9.9*/

