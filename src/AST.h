#ifndef AST
#define AST

#include "globalEnum.h"
#include <string.h>


typedef struct Stmt Stmt;
typedef struct switchCaseClause switchCaseClause;

typedef struct Exp Exp;
typedef struct ExpList ExpList;



Stmt* makeBreakStmt();
Stmt* makeContinueStmt();
Stmt* makeFallthroughStmt();


Stmt* makeBlockStmt(Stmt* stmt);
Stmt* makeExpressionStmt(Exp* expression);
Stmt* makeAssignmentStmt(ExpList* lhs, ExpList* rhs);
Stmt* makePrintStmt(ExpList* list);
Stmt* makePrintlnStmt(ExpList* list);
Stmt* makeReturnStmt(Exp* expr);
Stmt* makeIfStmt(Stmt* statement, Exp* expression, Stmt* block);
Stmt* makeWhileLoopStmt(Exp* condition, Stmt* block);
Stmt* makeThreePartLoopStmt(Stmt* init, Exp* condition, Stmt* inc, Stmt* block);
Stmt* makeSwitchStmt(Stmt* statement, Exp* expression, switchCaseClause* clauseList);
switchCaseClause* makeSwitchCaseClause(ExpList* expressionList, Stmt* statementList);
ExpList* reverseList(ExpList* reversed);
Exp *makeExpIdentifier(char *identifier); //How should we handle types?
Exp *makeExpIntLit(int intLit);
Exp *makeExpFloatLit(double floatLit);
Exp *makeExpStringLit(ExpressionKind kind, char *stringLit);
Exp *makeExpRuneLit(char* runeLit);
Exp *makeExpBinary(Exp *left,  Exp *right, ExpressionKind kind);
Exp *makeExpUnary(Exp *unary, ExpressionKind kind );
Exp *makeExpAppend(Exp *list, Exp *elem);
Exp *makeExpBuiltInBody(Exp *builtInBody, ExpressionKind kind);
Exp *makeExpAccess(Exp *base, Exp * accessor, ExpressionKind kind);
ExpList *addArgument(ExpList * args, Exp * argument);
ExpList *createArgumentList(Exp *argument);
Exp *makeExpFuncCall(Exp *base, ExpList *arguments, ExpressionKind kind);

Stmt* reverseStmtList(Stmt* reversed);
ExpList* reverseList(ExpList* reversed);

int isFuncCall(Exp* expression);


int isBlank(Exp* expression);
int containsBlank(ExpList* list);
int expListLength(ExpList* list);

struct Stmt{
    StatementKind kind;
    struct Stmt* next;

    union {
        struct {Stmt* stmt;} block;
        struct {Exp* expr;} expression;
        struct {ExpList* lhs; ExpList* rhs;} assignment;
        // TODO Declaration, short declaration


        struct {ExpList* list;} print;
        struct {ExpList* list;} println;

        struct {Exp* returnVal;} returnVal;
        
        struct {Stmt* statement; Exp* expression; Stmt* block;} ifStmt; // Statement is added to the scope of if statement and else, and expression may be null to represent variants of if
        struct {Stmt* block;} elseStmt;


        struct {Stmt* block;} infLoop;
        struct {Exp* conditon; Stmt* block;} whileLoop;
        struct {Stmt* init; Exp* condition; Stmt* inc; Stmt* block;} forLoop;

 

        struct {Stmt* statement; Exp* expression; switchCaseClause* clauseList;} switchStmt; //Different variants are encodes using NULL


        //Break and continue are encoded in kind

    }val;


};


struct switchCaseClause {
    ExpList* expressionList; // NULL corresponds to the default case
    Stmt* statementList;
    struct switchCaseClause* next;

} ;









// Yann's definitions 


 // for arguments
struct ExpList { //Will be reversed because of left recursion!
	Exp * cur;
	ExpList *next;
};
struct Exp {
	ExpressionKind kind;
	union {
		char *id;
		int intLit;
		double floatLit;
		char* runeLit;
		char *stringLit;
		struct { Exp *left; Exp *right; } binary;
		Exp * unary;
		struct { Exp *list; Exp *elem; }  append;
		struct { Exp *base; ExpList *arguments; } funcCall;
		struct { Exp *base; Exp *accessor; } access;
		Exp *builtInBody;
	} val;
};







#endif