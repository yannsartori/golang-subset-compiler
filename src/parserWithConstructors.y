%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalEnum.h"
#include "AST.h"


Stmt* root;

int yylex();
extern int yylineno;

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

Stmt* compoundOperator(Exp* left,Exp* right,ExpressionKind kind){
	compoundOperatorError(left,right);
	return makeAssignmentStmt(createArgumentList(left) ,createArgumentList(makeExpBinary(left,right,kind) ) );

	
}


%}

%define parse.error verbose

%code requires
{
	#include "AST.h"
}
//need to implement their builtins
//denali provided
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
}
%token tLOGICOR tLOGICAND tEQ tNEQ tGEQ tLEQ tBShiftLeft tBShiftRight tAndNot tLENGTH tCAP tAPPEND tBreak tDefault tFunc tInterface tSelect tCase tDefer tGo tMap tStruct tChan tElse tGoto tPackage tSwitch tConst tFallthrough tIf tRange tType tContinue tFor tImport tReturn tVar tPrint tPrintln tPlusEq tAndEq tMinusEquals tOrEquals tTimesEquals tHatEquals tLessMinus tDivideEquals tLShiftEquals tIncrement tDefined tModEquals tRShiftEquals tDecrement tElipses tAndHatEquals 
%token <intval>	   tINTLIT
%token <floatval>  tFLOATLIT
%token <runeval> tRUNELIT
%token <stringval> tRAWSTRINGLIT tINTERPRETEDSTRINGLIT
%token <boolval> tBOOLVAL
%token <identifier> tIDENTIFIER
%type <exp>  expression operand literal conversion index selector appendExpression lengthExpression capExpression type primaryExpression 
%type <explist> expressionList arguments expressionSwitchCase
%left tLOGICOR
%left tLOGICAND
%left tEQ tNEQ tGEQ tLEQ '>' '<'
%left '+' '-' '|' '^'
%left '*' '/' '%' tBShiftLeft tBShiftRight '&' tAndNot
%left '!' UNARY



%type <clause> expressionCaseClauseList expressionCaseClause
%type <stmt> block statementList statement simpleStatement assignmentStatement ifStatement loop switch

%start Program

%%

Program : statementList {root = reverseStmtList($1);}

 
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
			type '(' expression ')' { $$ = makeExpFuncCall($1, createArgumentList($3), expKindTypeCast); } /*also fix type pls */
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
type: 'b' { $$ = makeExpIdentifier("b"); }; //placeholder









statementList :	 statementList statement {$$ = $1;$$->next = $2;}
				 | %empty {$$ = NULL;}
				
block : '{' statementList '}'  {$$ = makeBlockStmt(reverseStmtList($2));}// 2.8.2 




statement: 
		
	
			tPrint '(' expressionList ')' ';' {if (containsBlank($3)) { printBlankError(); } else {$$ = makePrintStmt(reverseList($3));}  
												
												} // 2.8.8 Blank identifier in expressionList



			| tPrintln '(' expressionList ')' ';' {if (containsBlank($3)) { printBlankError(); } else {$$ = makePrintlnStmt(reverseList($3));}  
												
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


;

simpleStatement: 
			%empty {$$ = NULL ;}/*2.8.1*/  
			| expression  {if (isFuncCall($1)) {$$ = makeExpressionStmt($1); } else {expressionStmtError();}     } /* 2.8.3 sketchy, (needs to be a function call)*/ 


			| tIDENTIFIER tIncrement  // 2.8.7 , Blank identifier, not _
			| tIDENTIFIER tDecrement // 2.8.7 , Blank identifier, not _ 



			| assignmentStatement {$$ = $1;}//2.8.4


			| expressionList tDefined expressionList    /*2.8.6 Hacky Fix, LHS needs to be an identifier list
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
			tIf expression block
			| tIf expression block tElse ifStatement
			| tIf expression block tElse block
			| tIf simpleStatement  ';' expression block
			| tIf simpleStatement ';'  expression block tElse ifStatement
			| tIf simpleStatement  ';' expression block tElse block






//2.8.12
loop : 
		tFor block 
		| tFor expression block
		| tFor simpleStatement ';' expression ';' simpleStatement block






//2.8.11 Check usage for AST
switch:
		tSwitch simpleStatement ';' expression '{' expressionCaseClauseList '}'
		| tSwitch expression '{' expressionCaseClauseList '}'
		| tSwitch simpleStatement ';' '{' expressionCaseClauseList '}'
		| tSwitch '{' expressionCaseClauseList '}'




expressionCaseClauseList : %empty {$$ = NULL;}
						| expressionCaseClause expressionCaseClauseList {$$ = $1; $1->next  = $2;}
expressionCaseClause : expressionSwitchCase ":" statementList {$$ = makeSwitchCaseClause($1,$3);}

expressionSwitchCase : tCase expressionList {$$ = $2;}
					| tDefault {$$ = NULL;}