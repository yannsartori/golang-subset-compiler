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

Stmt* cons(Stmt* head,Stmt* tail);
Stmt* makeBlockStmt(Stmt* stmt);
Stmt* makeExpressionStmt(Exp* expression);
Stmt* makeAssignmentStmt(ExpList* lhs, ExpList* rhs);
Stmt* makePrintStmt(ExpList* list);
Stmt* makePrintlnStmt(ExpList* list);
Stmt* makeReturnStmt(Exp* expr);
Stmt* makeIfStmt(Stmt* statement, Exp* expression, Stmt* block,Stmt* elseBlock);
Stmt* makeElseStmt(Stmt* block);
Stmt* makeWhileLoopStmt(Exp* condition, Stmt* block);
Stmt* makeInfLoopStmt(Stmt* block);
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

    int lineno;

    union {
        struct {Stmt* stmt;} block;
        struct {Exp* expr;} expression;
        struct {ExpList* lhs; ExpList* rhs;} assignment;


        struct {ExpList* list;} print;
        struct {ExpList* list;} println;

        struct {Exp* returnVal;} returnVal;
        
        struct {Stmt* statement; Exp* expression; Stmt* block;Stmt* elseBlock;} ifStmt; // Statement is added to the scope of if statement and else, and expression may be null to represent variants of if
        struct {Stmt* block;} elseStmt;


        struct {Stmt* block;} infLoop;
        struct {Exp* conditon; Stmt* block;} whileLoop;
        struct {Stmt* init; Exp* condition; Stmt* inc; Stmt* block;} forLoop;

 

        struct {Stmt* statement; Exp* expression; switchCaseClause* clauseList;} switchStmt; //Different variants are encodes using NULL

        //Break and continue are encoded in kind



        // TODO Declaration, short declaration

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
    int isBracketed;
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








typedef struct RootNode RootNode;
typedef struct TopDeclarationNode TopDeclarationNode;
typedef struct VarDeclNode VarDeclNode;
typedef struct TypeDeclNode TypeDeclNode;
typedef struct FuncDeclNode FuncDeclNode;



RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl);

struct RootNode  {
	char* packageName;
	TopDeclarationNode* startDecls;
};

struct TopDeclarationNode {
	TopDeclarationNode* nextTopDecl;
	int multiDecl;
	TopDeclarationType declType;
	union {
		VarDeclNode* varDecl;
		TypeDeclNode* typeDecl;
		FuncDeclNode* funcDecl;
	} actualRealDeclaration;
};



struct VarDeclNode {
	char* identifier;
	Exp* value;
	int arrayLength;
	VarDeclNode* nextDecl;
};

struct TypeDeclNode {
	char* identifier;
	TypeDeclNode* nextDecl;
};

struct FuncDeclNode {
	char* identifier;
	TypeDeclNode* argsDecls;
};


#endif
