#ifndef AST
#define AST


#include "globalEnum.h"
#include <string.h>

//typedefs in globalEnum

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
Stmt* makeOpAssignmentStmt(Exp* lhs, Exp* rhs,ExpressionKind kind);
Stmt* makeIncStmt(Exp* exp);
Stmt* makeDecStmt(Exp* exp);
switchCaseClause* makeSwitchCaseClause(ExpList* expressionList, Stmt* statementList);
ExpList* reverseList(ExpList* reversed);
Exp *makeExpIdentifier(char *identifier); 
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
Exp *makeExpFuncCall(Exp *base, ExpList *arguments);

Stmt* reverseStmtList(Stmt* reversed);
ExpList* reverseList(ExpList* reversed);

int weedTopDeclarationNode(TopDeclarationNode* node, State loopState, State switchState, State functionState);

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

		struct {Exp* lhs; Exp* rhs; ExpressionKind kind;} opAssignment;

		struct {Exp* exp;} incStmt;

		struct {Exp* exp;} decStmt;

		struct {char* label;} gotoStmt;//Not actually a feature of golite, this helps me during codegen

        //Break and continue are encoded in kind

		VarDeclNode* varDeclaration;
		TypeDeclNode* typeDeclaration;

        // TODO Declaration, short declaration

    }val;


};


struct switchCaseClause {
    ExpList* expressionList; // NULL corresponds to the default case
    Stmt* statementList;
	int lineno;
    struct switchCaseClause* next;

} ;



// Yann's definitions 
int isBinary(Exp *e);
int isUnary(Exp *e);

 // for arguments

struct ExpList { //Will be reversed because of left recursion!
	Exp * cur;
	ExpList *next;
};

struct Exp {
	ExpressionKind kind;
	int isBracketed;
	int lineno;
	PolymorphicEntry *contextEntry;
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







struct RootNode  {
	char* packageName;
	TopDeclarationNode* startDecls;
};

struct TopDeclarationNode {
	TopDeclarationNode* nextTopDecl;
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
	VarDeclNode* nextDecl;
	VarDeclNode* multiDecl;
	TypeHolderNode* typeThing;
	STEntry* whoAmI;
	int iDoDeclare;		/*Used to track whether or not a short var declaration (i.e.  one that uses the ":=" operator) actually declares or just assigns. The symbol table printer  reads this and decides whether or not to print the symbol */
	int lineno;
	TTEntry* functionArgTypeEntry;//Only used to give function args a TTEntry
};

struct TypeDeclNode {
	char* identifier;
	TypeDeclNode* nextDecl;
	TypeHolderNode* actualType;
	int lineno;
	TTEntry* typeEntry;
	TTEntry* stmtTypeEntry;
};

struct FuncDeclNode {
	char* identifier;
	VarDeclNode* argsDecls;
	TypeHolderNode* returnType;
	Stmt* blockStart;
	int lineno;
	STEntry* symbolEntry;
};

struct TypeHolderNode {
	TypeType kind;
	char* identification;
	TypeHolderNode* underlyingType;
	int arrayDims;
	VarDeclNode* structMembers;
};

struct IdChain {
	char* identifier;
	IdChain* next;
};

RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl);
TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl);
TypeHolderNode* makeArrayHolder(int arraySize, TypeHolderNode* id);
TypeHolderNode* makeStructHolder(VarDeclNode* members);
TypeHolderNode* makeSliceHolder(TypeHolderNode* id);
TypeHolderNode* makeIdTypeHolder(char* id);
IdChain* makeIdChain(char* identifier, IdChain* next);
TypeDeclNode* makeSingleTypeDecl(char* identifier, TypeHolderNode* givenType);
TopDeclarationNode* makeTopTypeDecl(TypeDeclNode* typeDecl, TopDeclarationNode* nextTopDecl);
void appendTypeDecls(TypeDeclNode* baseDecl, TypeDeclNode* leafDecl);
void appendVarDecls(VarDeclNode* baseDecl, VarDeclNode* leafDecl);
VarDeclNode* makeSingleVarDeclNoExps(IdChain* identifiers, TypeHolderNode* givenType);
VarDeclNode* makeSingleVarDeclWithExps(IdChain* identifiers, TypeHolderNode* givenType, ExpList* values, int lineno);
FuncDeclNode* makeFuncDecl(char* funcName, VarDeclNode* argsDecls, TypeHolderNode* returnType, int lineno);
TopDeclarationNode* makeTopFuncDecl(FuncDeclNode* funcDecl, TopDeclarationNode* nextTopDecl);
Stmt* makeVarDeclStatement(VarDeclNode* declaration, int isShort, int lineNomber);
Stmt* makeTypeDeclStatement(TypeDeclNode* declaration, int lineNomber);
IdChain* extractIdList(ExpList* expressions, int lineno);
TypeHolderNode* makeInferredTypeHolder();

#endif
