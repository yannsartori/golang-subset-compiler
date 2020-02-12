%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "to_string.h"
#include "symbol_check.h"

int yylex();
extern int yylineno;
extern int characterOnLine;
extern char curLine[500];
extern Stmt * root;

void yyerror (char const *s) {
	printErrorStmt(yylineno, characterOnLine, s, curLine);
 }
%}

%define parse.error verbose

%code requires
{
	#include "ast.h"
}

%union {
	int intval;
	int boolval;
	double floatval;
	char *stringval;
	char *identifier;
	VariableType type;
	struct Exp *exp;
	struct Stmt *stmt;
}
%token tVAR tFLOAT tINT tSTRING tBOOL tIF tELSE tWHILE tREAD tPRINT
%token tEQ tNEQ tGEQ tLEQ tLOGICAND tLOGICOR 
%token <intval>	   tINTLIT
%token <floatval>  tFLOATLIT
%token <stringval> tSTRINGLIT
%token <boolval> tBOOLVAL
%token <identifier> tIDENTIFIER
%type <exp> expression
%type <stmt> statements bracketStatements bracketStatement semiStatements semiStatement optionalElseIf 
%type <type> optionalType;
%left tLOGICOR
%left tLOGICAND
%left tEQ tNEQ
%left tGEQ tLEQ '>' '<'
%left '+' '-'
%left '*' '/'
%left '!' UNARYMINUS



%start program

%%
program: statements {root = $1;}
statements:		  semiStatements {$$ = $1; augmentStmtWithError($$, yylineno, characterOnLine, curLine); }
				| bracketStatements {$$ = $1; augmentStmtWithError($$, yylineno, characterOnLine, curLine); }
				| { $$ = NULL; }
				;	
bracketStatements:statements bracketStatement {$$ = $2; $$->next = $1; augmentStmtWithError($$, yylineno, characterOnLine, curLine);  };
bracketStatement: tIF '(' expression ')' '{' statements '}' optionalElseIf {$$ = makeStmtIf($3, $6, $8); augmentStmtWithError($$, yylineno, characterOnLine, curLine); }
				| tWHILE '(' expression ')' '{' statements '}' {$$ = makeStmtWhile($3, $6); augmentStmtWithError($$, yylineno, characterOnLine, curLine);  }
				;
semiStatements:	  statements semiStatement ';' {$$ = $2; $$->next = $1; augmentStmtWithError($$, yylineno, characterOnLine, curLine); };
semiStatement:	  tREAD '(' tIDENTIFIER ')' { 
					Exp * id = makeExpIdentifier($3, typeInference); 
					augmentExpWithError(id, yylineno, characterOnLine, curLine); 
					$$ = makeStmtRead(id); 
					augmentStmtWithError($$, yylineno, characterOnLine, curLine);} 
				| tPRINT '(' expression ')' { $$ = makeStmtPrint($3); augmentStmtWithError($$, yylineno, characterOnLine, curLine);  augmentExpWithError($3, yylineno, characterOnLine, curLine);}
				| tVAR tIDENTIFIER optionalType '=' expression {
					Exp * id = makeExpIdentifier($2, typeInference); 
					augmentExpWithError(id, yylineno, characterOnLine, curLine); 
					$$ = makeStmtAssignment(id, $5, 1); 
					augmentStmtWithError($$, yylineno, characterOnLine, curLine); }
				| tIDENTIFIER '=' expression {
					Exp * id = makeExpIdentifier($1, typeInference); 
					augmentExpWithError(id, yylineno, characterOnLine, curLine); 
					$$ = makeStmtAssignment(id, $3, 0); 
					augmentStmtWithError($$, yylineno, characterOnLine, curLine); } 
				;
optionalElseIf:	  tELSE tIF '(' expression ')' '{' statements '}' optionalElseIf { $$ = makeStmtIf($4, $7, $9); augmentStmtWithError($$, yylineno, characterOnLine, curLine);  } 
				| tELSE '{' statements '}' { $$ = $3; }
				| { $$ = NULL; }
				;
optionalType:	  ':' tFLOAT {$$ = typeFloat;} 
				| ':' tINT {$$ = typeInt;}
				| ':' tBOOL {$$ = typeBool;} 
				| ':' tSTRING {$$ = typeString;}
				| {$$ = typeInference;} 
				;
expression:		  expression '+' expression { $$ = makeExpAddition($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression '-' expression { $$ = makeExpSubtraction($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression '/' expression { $$ = makeExpDivision($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression '*' expression { $$ = makeExpMultiplication($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression '>' expression { $$ = makeExpGreater($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression '<' expression { $$ = makeExpLess($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tLEQ expression { $$ = makeExpLEQ($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tGEQ expression { $$ = makeExpGEQ($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tNEQ expression { $$ = makeExpNEQ($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tEQ expression {$$ = makeExpEQ($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tLOGICOR expression {$$ = makeExpLogicOr($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| expression tLOGICAND expression {$$ = makeExpLogicAnd($1, $3); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| '!' expression { $$ = makeExpLogicNot($2); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| '-' expression %prec UNARYMINUS { $$ = makeExpUnaryMinus($2); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| '(' expression ')' {$$ = $2; augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| tIDENTIFIER { $$ = makeExpIdentifier($1, typeInference); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| tINTLIT { $$ = makeExpIntLit($1); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| tFLOATLIT { $$ = makeExpFloatLit($1); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| tSTRINGLIT { $$ = makeExpStringLit($1); augmentExpWithError($$, yylineno, characterOnLine, curLine);  }
				| tBOOLVAL { $$ = makeExpBoolLit($1); augmentExpWithError($$, yylineno, characterOnLine, curLine); }
				;






