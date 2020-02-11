%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"
    int yylex();
    void yyerror(const char *string);
    extern STMT* root;
    extern int yylineno;
%}



%define parse.error verbose

%union{
    int intval;
    float floatval;
    char *identifier;
    char *quoted;
    EXP* exp;
    STMT* stmt;
    TypeKind kType;
}


%type <exp> exp
%type <kType> type
%type <stmt> stmt master elif
%token <intval> tInt
%token <floatval> tFloat
%token <identifier> tId
%token <quoted> tQuote
%token UMINUS tString tBool tVar tRead tPrint tIf tWhile tElse tNeq tGeq tLeq tTrue tFalse


%left '|'
%left '&'
%left '~' tNeq
%left '<' '>' tGeq tLeq
%left '+' '-'
%left '/' '*'
%left UMINUS

%start master

%%

master  : 							{$$ = NULL;}
        | stmt master			{
									$$ = $1;
									$$ -> next_statement = $2;
									root = $$;
								}
;


type    : tInt						{$$ = intType;}
        | tFloat					{$$ = floatType;}
        | tString					{$$ = stringType;}
        | tBool						{$$ = boolType;}
;

stmt    : tId '=' exp ';'					{$$ = makeAssignment(makeExpNodeId($1, yylineno), $3, yylineno);}
		| tVar tId '=' exp ';'				{$$ = makeUntypedDecl($2, $4, yylineno);}
		| tVar tId ':' type '=' exp ';'		{$$ = makeTypedDecl($2, $4, $6, yylineno);}
		| tRead '(' tId ')' ';'				{$$ = makeReadStmt(makeExpNodeId($3, yylineno), yylineno);}
		| tPrint '(' exp ')' ';'			{$$ = makePrintStmt($3, yylineno);}
		| tIf '(' exp ')' '{' master '}' elif
											{$$ = makeIfStmt($3, $6, $8, yylineno);}
		| tWhile '(' exp ')' '{' master '}'
											{$$ = makeWhileStmt($3, $6, yylineno);}
;

elif    :						{$$ = NULL;}
        | tElse tIf '(' exp ')' '{' master '}' elif
								{$$ = makeIfStmt($4, $7, $9, yylineno);}
        | tElse '{' master '}'
								{$$ = $3;}
;

exp     : tInt								{$$ = makeExpNodeInt($1, yylineno);}
        | tFloat							{$$ = makeExpNodeFloat($1, yylineno);}
        | tId								{$$ = makeExpNodeId($1, yylineno);}
        | tQuote							{$$ = makeExpNodeString($1, yylineno);}
        | tTrue								{$$ = makeExpNodeBool(true, yylineno);}
        | tFalse							{$$ = makeExpNodeBool(false, yylineno);}
        | exp '+' exp						{$$ = makeBinaryExpNode(expKindPlus, $1, $3, yylineno);}
        | exp '-' exp						{$$ = makeBinaryExpNode(expKindMinus, $1, $3, yylineno);}
        | exp '*' exp						{$$ = makeBinaryExpNode(expKindTimes, $1, $3, yylineno);}
        | exp '/' exp						{$$ = makeBinaryExpNode(expKindDivide, $1, $3, yylineno);}
        | '(' exp ')'						{$$ = $2;}
        | '-' exp         %prec UMINUS		{$$ = makeUnaryExpNode(expKindNeg, $2, yylineno);}
        | '!' exp         %prec UMINUS		{$$ = makeUnaryExpNode(expKindNot, $2, yylineno);}
        | exp '|' exp						{$$ = makeBinaryExpNode(expKindOr, $1, $3, yylineno);}
        | exp '&' exp						{$$ = makeBinaryExpNode(expKindAnd, $1, $3, yylineno);}
        | exp '~' exp						{$$ = makeBinaryExpNode(expKindEquiv, $1, $3, yylineno);}
        | exp '<' exp						{$$ = makeBinaryExpNode(expKindLess, $1, $3, yylineno);}
        | exp '>' exp						{$$ = makeBinaryExpNode(expKindGreater, $1, $3, yylineno);}
        | exp tLeq exp						{$$ = makeBinaryExpNode(expKindLeq, $1, $3, yylineno);}
        | exp tGeq exp						{$$ = makeBinaryExpNode(expKindGeq, $1, $3, yylineno);}
        | exp tNeq exp						{$$ = makeBinaryExpNode(expKindNeq, $1, $3, yylineno);}
;

%%


void yyerror(const char *string) {
    fprintf(stderr, "Error: %s on line %d\n", string, yylineno);
    exit(1);
}
