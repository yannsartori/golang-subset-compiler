#ifndef AST
#define AST
#include "globalEnum.h"
typedef struct Exp Exp;
typedef struct ExpList ExpList; // for arguments
typedef struct rootNode rootNode;
typedef struct topDeclarationNode topDeclarationNode;
struct rootNode  {
	char* packageName;
	topDeclarationNode startDecls;
};

struct topDeclarationNode {
	char* type;
	topDeclarationNode* nextTopDecl;
	varDecl* 
};

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
		char runeLit;
		char *stringLit;
		struct { Exp *left; Exp *right; } binary;
		Exp * unary;
		struct { Exp *list; Exp *elem; }  append;
		struct { Exp *base; ExpList *arguments; } funcCall;
		struct { Exp *base; Exp *accessor; } access;
		Exp *builtInBody;
	} val;
};
ExpList *addArgument(ExpList * args, Exp * argument);
ExpList *createArgumentList(Exp *argument);
Exp *makeExpIdentifier(char *identifier); //How should we handle types?
Exp *makeExpIntLit(int intLit);
Exp *makeExpRuneLit(char runeLit);
Exp *makeExpFloatLit(double floatLit);
Exp *makeExpStringLit(ExpressionKind kind, char *stringLit);
Exp *makeExpBinary(Exp *left, Exp *right, ExpressionKind kind);
Exp *makeExpUnary(Exp *unary, ExpressionKind kind );
Exp *makeExpAppend(Exp *list, Exp *elem);
Exp *makeExpAccess(Exp *base, Exp * accessor, ExpressionKind kind);
Exp *makeExpFuncCall(Exp *base, ExpList *arguments, ExpressionKind kind);
Exp *makeExpBuiltInBody(Exp *builtInBody, ExpressionKind kind);
#endif
