#ifndef AST
#define AST
#include "globalEnum.h"
typedef struct Exp Exp;
typedef struct ExpList ExpList; // for arguments

typedef struct RootNode RootNode;
typedef struct TopDeclarationNode TopDeclarationNode;
typedef struct VarDeclNode VarDeclNode;
typedef struct TypeDeclNode TypeDeclNode;
typedef struct FuncDeclNode FuncDeclNode;
typedef struct TypeHolderNode TypeHolderNode;
typedef struct IdChain IdChain;

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
	TypeHolderNode* typeThing;
};

struct TypeDeclNode {
	char* identifier;
	TypeDeclNode* nextDecl;
	TypeHolderNode* actualType;
};

struct FuncDeclNode {
	char* identifier;
	TypeDeclNode* argsDecls;
	TypeHolderNode* returnType;
	void* blockStart;
};

struct TypeHolderNode {
	TypeType kind;
	char* identification;
	Exp* arrayDims;
	TypeDeclNode* structMembers;
};

struct IdChain {
	char* identifier;
	IdChain* next;
};

RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl);
TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl);
TypeHolderNode* makeArrayHolder(Exp* arraySize, char* id);
TypeHolderNode* makeStructHolder(TypeDeclNode* members);
TypeHolderNode* makeSliceHolder(char* id);
TypeHolderNode* makeIdTypeHolder(char* id);
IdChain* makeIdChain(char* identifier, IdChain* next);
TypeDeclNode* makeSingleTypeDecl(IdChain* identifiers, TypeHolderNode* givenType);
TopDeclarationNode* makeTopTypeDecl(TypeDeclNode* typeDecl, TopDeclarationNode* nextTopDecl);
void appendTypeDecls(TypeDeclNode* baseDecl, TypeDeclNode* leafDecl);
void appendVarDecls(VarDeclNode* baseDecl, VarDeclNode* leafDecl);
VarDeclNode* makeSingleVarDeclNoExps(IdChain* identifiers, TypeHolderNode* givenType);
VarDeclNode* makeSingleVarDeclWithExps(IdChain* identifiers, TypeHolderNode* givenType, ExpList* values);
FuncDeclNode* makeFuncDecl(char* funcName, TypeDeclNode* argsDecls, TypeHolderNode* returnType, void* blockStart);
TopDeclarationNode* makeTopFuncDecl(FuncDeclNode* funcDecl, TopDeclarationNode* nextTopDecl);

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
		char *runeLit;
		char *stringLit;
		struct { Exp *left; Exp *right; } binary;
		Exp * unary;
		struct { Exp *list; Exp *elem; }  append;
		struct { Exp *base; ExpList *arguments; } funcCall;
		struct { Exp *base; Exp *accessor; } access;
		Exp *builtInBody;
	} val;
};
//Below are Neil provided functions
int isBlank(Exp* expression);
int containsBlank(ExpList* list);

//Below are Yann provided functions
ExpList *addArgument(ExpList * args, Exp * argument);
ExpList *createArgumentList(Exp *argument);
Exp *makeExpIdentifier(char *identifier); //How should we handle types?
Exp *makeExpIntLit(int intLit);
Exp *makeExpRuneLit(char *runeLit);
Exp *makeExpFloatLit(double floatLit);
Exp *makeExpStringLit(ExpressionKind kind, char *stringLit);
Exp *makeExpBinary(Exp *left, Exp *right, ExpressionKind kind);
Exp *makeExpUnary(Exp *unary, ExpressionKind kind );
Exp *makeExpAppend(Exp *list, Exp *elem);
Exp *makeExpAccess(Exp *base, Exp * accessor, ExpressionKind kind);
Exp *makeExpFuncCall(Exp *base, ExpList *arguments);
Exp *makeExpBuiltInBody(Exp *builtInBody, ExpressionKind kind);
void reverseArgumentList(ExpList **list);
#endif
