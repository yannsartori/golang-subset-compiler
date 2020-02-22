%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalEnum.h"
#include "ast.h"

int yylex();
extern int yylineno;
extern RootNode * rootNode;

void yyerror(char const *s) {
	fprintf(stderr, "Error: %s on line %d\n", s, yylineno);
	exit(1);
}


void printBlankError(){
	fprintf(stderr, "Error: (line %d) print statement may not contain the blank identifier \n", yylineno);
	exit(1);
}

void returnBlankError(){
	fprintf(stderr, "Error: (line %d) cannot return the blank identifier \n", yylineno);
	exit(1);
}

void expressionStmtError() {
	fprintf(stderr, "Error: (line %d) expression statement must be a function call \n", yylineno);
	exit(1);
}


void lengthError(int left,int right) {
	fprintf(stderr, "Error: (line %d) assignment lhs(%d) != rhs(%d) \n", yylineno,left,right);
	exit(1);
}

void blankAssignmentError() {
	fprintf(stderr, "Error: (line %d) assignment value may not contain the blank identifier \n", yylineno);
	exit(1);
}


void blankTargetError() {
	fprintf(stderr, "Error: (line %d) assignment target may not contain the blank identifier \n", yylineno);
	exit(1);
}

void compoundOperatorError(Exp* left,Exp* right){
	if (isBlank(left)){
		blankTargetError();
	}else if (isBlank(right)){
		blankAssignmentError();
	}
}


void blankSwitchExprError(){
	fprintf(stderr, "Error: (line %d) switch statement condition may not contain the blank identifier \n", yylineno);
	exit(1);
}

void blankSwitchCaseClauseError(){
	fprintf(stderr, "Error: (line %d)  switch clause may not contain the blank identifier \n", yylineno);
	exit(1);
}

Stmt* compoundOperator(Exp* left,Exp* right,ExpressionKind kind){
	compoundOperatorError(left,right);
	return makeAssignmentStmt(createArgumentList(left) ,createArgumentList(makeExpBinary(left,right,kind) ) );

	
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

void shortDeclarationPostError(Stmt* stmt){

	if(stmt == NULL){
		return;
	}
	if (stmt->kind == StmtKindShortDeclaration){
		fprintf(stderr, "Error: (line %d) for loop post statement may not be a short declaration\n", yylineno);
		exit(1);
	}
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
	struct Stmt* stmt;
	struct switchCaseClause* clause;
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

%type <exp>  expression operand literal index selector appendExpression lengthExpression capExpression primaryExpression 
%type <explist> expressionList arguments expressionSwitchCase maybeEmptyExpressionList
%type <rootNode> root
%type <topDeclNode> topDeclarationList
%type <varDeclNode> variableDecl singleVarDecl innerVarDecls
%type <typeDeclNode> typeDecl singleTypeDecl innerTypeDecls funcArgDecls
%type <funcDeclNode> funcDecl
%type <declType> declType sliceDeclType arrayDeclType structDeclType
%type <tempIdChain> identifierList
%type <clause> expressionCaseClauseList expressionCaseClause
%type <stmt> block statementList statement simpleStatement assignmentStatement ifStatement loop switch

%left tLOGICOR
%left tLOGICAND
%left tEQ tNEQ tGEQ tLEQ '>' '<'
%left '+' '-' '|' '^'
%left '*' '/' '%' tBShiftLeft tBShiftRight '&' tAndNot
%left '!' UNARY



%start root

%%


root			: tPackage tIDENTIFIER ';' topDeclarationList {rootNode = makeRootNode($2, $4);}
;

topDeclarationList	: %empty				{$$ = NULL;}
			| variableDecl topDeclarationList	{$$ = makeTopVarDecl($1, $2);}
			| typeDecl topDeclarationList	{$$ = makeTopTypeDecl($1, $2); }
			| funcDecl topDeclarationList	{$$ = makeTopFuncDecl($1, $2); }
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
							{$$ = makeSingleVarDeclWithExps($1, $2, $4, yylineno);}
			| identifierList '=' expressionList
							{$$ = makeSingleVarDeclWithExps($1, NULL, $3, yylineno);}
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
						{$$ = makeFuncDecl($2, $4, $6, $7);}
			| tFunc tIDENTIFIER '(' ')' declType block ';'
						{$$ = makeFuncDecl($2, NULL, $5, $6);}
			| tFunc tIDENTIFIER '(' funcArgDecls ')' block ';'
						{$$ = makeFuncDecl($2, $4, NULL, $6);}
			| tFunc tIDENTIFIER '(' ')' block ';'
						{$$ = makeFuncDecl($2, NULL, NULL, $5);}
;

funcArgDecls		: singleTypeDecl ',' funcArgDecls		{appendTypeDecls($1, $3); $$ = $1;}
			| singleTypeDecl				{$$ = $1;}
;

declType		: tIDENTIFIER					{$$ = makeIdTypeHolder($1);}
			| sliceDeclType				{$$ = $1;}
			| arrayDeclType				{$$ = $1;}
			| structDeclType				{$$ = $1; }
;

sliceDeclType		: '[' ']' declType				{$$ = makeSliceHolder($3);}
;
arrayDeclType		: '[' tINTLIT ']' declType				{$$ = makeArrayHolder($2, $4);}
;
structDeclType	: tStruct '{' innerTypeDecls '}'		{$$ = makeStructHolder($3);}
			| tStruct '{' '}'				{$$ = makeStructHolder(NULL);}
;

identifierList	: tIDENTIFIER					{$$ = makeIdChain($1, NULL);}
			| tIDENTIFIER ',' identifierList		{$$ = makeIdChain($1, $3);}
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
maybeEmptyExpressionList : 
			  %empty {$$ = NULL;}
			| expressionList {$$ = $1;};
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
index:		  '[' expression ']'							{ if ( isBlank($2) ) indexingBlankError(); $$ = $2; }/*2.9.7*/
arguments: 
			  '(' maybeEmptyExpressionList ')'				{ if ( containsBlank($2) ) argumentBlankError(); $$ = $2;}
			;
selector:		  '.' tIDENTIFIER							{ if ( strcmp($2, "_") == 0 ) fieldSelectBlankError(); $$ = makeExpIdentifier($2); }; /*2.9.8*/
appendExpression: tAPPEND '(' expression ',' expression ')' { if ( isBlank($3) || isBlank($5) ) builtInBlankError("append"); $$ = makeExpAppend($3, $5); }; /*2.9.9*/
lengthExpression: tLENGTH '(' expression ')'				{ if ( isBlank($3) ) builtInBlankError("length"); $$ = makeExpBuiltInBody($3, expKindLength); }; /*2.9.9*/
capExpression:	  tCAP '(' expression ')'					{ if ( isBlank($3) ) builtInBlankError("capacity"); $$ = makeExpBuiltInBody($3, expKindCapacity); }; /*2.9.9*/











statementList :	 statementList statement {$$ = cons($2,$1);}
				 | %empty {$$ = NULL;}
				
block : '{' statementList '}'  {$$ = makeBlockStmt(reverseStmtList($2));}// 2.8.2 




statement: 
		
	
			tPrint '(' maybeEmptyExpressionList ')' ';' {if (containsBlank($3)) { printBlankError(); } else {$$ = makePrintStmt(reverseList($3));}  
												
												} // 2.8.8 Blank identifier in expressionList



			| tPrintln '(' maybeEmptyExpressionList ')' ';' {if (containsBlank($3)) { printBlankError(); } else {$$ = makePrintlnStmt(reverseList($3));}  
												
												}// 2.8.8 Blank identifier in expressionList
			| tReturn ';' {$$ = makeReturnStmt(NULL);}// 2.8.9 
			| tReturn expression ';' {if (isBlank($2)) {returnBlankError();}  else {$$ = makeReturnStmt($2);}} // 2.8.9 Blank Identifier

			| tBreak ';' {$$ = makeBreakStmt();} //2.8.13 Caught at weeding 
			| tFallthrough ':' {$$ = makeFallthroughStmt();} // Weeding (present only in switch statement last line, in all but last switch case)
			| tContinue ';' {$$ = makeContinueStmt();} //2.8.13 Caught at weeder
			


			| simpleStatement ';' {$$ = $1;}
			| block {$$ = $1;}
			| switch //2.8.11 {$$ = $1;}
			| ifStatement //2.8.10 {$$ = $1;}
			| loop //2.8.12 {$$ = $1;}
			| typeDecl				{$$ = makeTypeDeclStatement($1, yylineno);}
			| variableDecl			{$$ = makeVarDeclStatement($1, 0, yylineno);}


;

simpleStatement: 
			%empty {$$ = NULL ;}/*2.8.1*/  
			| expression  {if (isFuncCall($1)) {$$ = makeExpressionStmt($1); } else {expressionStmtError();}     } /* 2.8.3 sketchy, (needs to be a function call)*/ 


			| tIDENTIFIER tIncrement {$$ = compoundOperator(makeExpIdentifier($1),makeExpIntLit(1),expKindAddition);} // 2.8.7 , Blank identifier, not _
			| tIDENTIFIER tDecrement {$$ = compoundOperator(makeExpIdentifier($1),makeExpIntLit(1),expKindSubtraction);}// 2.8.7 , Blank identifier, not _ 



			| assignmentStatement {$$ = $1;}//2.8.4


			| expressionList tDefined expressionList   
				{$$ = 
					makeVarDeclStatement(
						makeSingleVarDeclWithExps(
							extractIdList($1, yylineno), 
							NULL, 
							$3,
							yylineno
						), 
						1, 
						yylineno
					);
				} /*2.8.6 Hacky Fix, LHS needs to be an identifier list
													(Short declaration) Parser needs to check that length(LHS) == length(RHS), 
													weeder probably needs to check that we can assign into LHS
													 */

;

assignmentStatement : 	
							expressionList '=' expressionList {int left = expListLength($1); int right = expListLength($3);
																if (left == right){
																	if (containsBlank($3)){
																		blankAssignmentError();
																	}
																	$$ = makeAssignmentStmt(reverseList($1),reverseList($3));


																 }
																else {lengthError(left,right);}    }
							/*2.8.4 Parser needs to check that length(LHS) == length(RHS), 
													weeder probably needs to check that we can assign into LHS 
													Blank identifier RHS*/
													
							//AVOID BLANK IDENTIFIER LHS AND RHS
					
							|expression tPlusEq expression {$$ = compoundOperator($1,$3,expKindAddition);}
							|expression tAndEq expression {$$ = compoundOperator($1,$3,expKindLogicAnd);}
							|expression tMinusEquals expression {$$ = compoundOperator($1,$3,expKindSubtraction);}
							|expression tOrEquals expression {$$ = compoundOperator($1,$3,expKindLogicOr);}
							|expression tTimesEquals expression {$$ = compoundOperator($1,$3,expKindMultiplication);}
							|expression tHatEquals expression {$$ = compoundOperator($1,$3,expKindBitNotBinary);}
							|expression tLShiftEquals expression {$$ = compoundOperator($1,$3,expKindBitShiftLeft);}
							|expression tRShiftEquals expression {$$ = compoundOperator($1,$3,expKindBitShiftRight);}
							|expression tAndHatEquals expression {$$ = compoundOperator($1,$3,expKindBitAndNot);}
							|expression tModEquals expression {$$ = compoundOperator($1,$3,expKindMod);}
							|expression tDivideEquals expression {$$ = compoundOperator($1,$3,expKindDivision);}
						
							
;

// 2.8.10
ifStatement : 
			tIf expression block {$$ = makeIfStmt(NULL,$2,$3,NULL);}
			| tIf expression block tElse ifStatement {$$ = makeIfStmt(NULL,$2,$3,makeElseStmt(makeBlockStmt($5))); }
			| tIf expression block tElse block {$$ = makeIfStmt(NULL,$2,$3,makeElseStmt($5)); }
			| tIf simpleStatement  ';' expression block {$$ = makeIfStmt($2,$4,$5,NULL); }
			| tIf simpleStatement ';'  expression block tElse ifStatement {$$ = makeIfStmt($2,$4,$5,makeElseStmt(makeBlockStmt($7))); } //Blockify
			| tIf simpleStatement  ';' expression block tElse block {$$ = makeIfStmt($2,$4,$5,makeElseStmt($7));}





//2.8.12
loop : 
		tFor block {$$ = makeInfLoopStmt($2);}
		| tFor expression block {$$ = makeWhileLoopStmt($2,$3);}
		| tFor simpleStatement ';' expression ';' simpleStatement block { shortDeclarationPostError($6) ;  $$ = makeThreePartLoopStmt($2,$4,$6,$7);}






//2.8.11 Check usage for AST, check usage of the blank identifier in the switch case clause
switch:
		tSwitch simpleStatement ';' expression '{' expressionCaseClauseList '}' { if (isBlank($4)) { blankSwitchExprError(); }   ;  $$ = makeSwitchStmt($2,$4,$6); }
		| tSwitch expression '{' expressionCaseClauseList '}' {if (isBlank($2)) { blankSwitchExprError(); }   ;$$ = makeSwitchStmt(NULL,$2,$4); }
		| tSwitch simpleStatement ';' '{' expressionCaseClauseList '}' {$$ = makeSwitchStmt($2,NULL,$5); }
		| tSwitch '{' expressionCaseClauseList '}'  {$$ = makeSwitchStmt(NULL,NULL,$3); }




expressionCaseClauseList : %empty {$$ = NULL;}
						| expressionCaseClause expressionCaseClauseList {$$ = $1; $1->next  = $2;}
						
expressionCaseClause : expressionSwitchCase ':' statementList {$$ = makeSwitchCaseClause($1, reverseStmtList($3));}

expressionSwitchCase : tCase expressionList {if (containsBlank($2)) {blankSwitchCaseClauseError();} ;$$ = $2;}
					| tDefault {$$ = NULL;}
