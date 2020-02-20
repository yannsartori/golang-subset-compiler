#include <stdlib.h>
#include <string.h>
#include "globalEnum.h"
#include "ast.h"
//Neil provided
int isBlank(Exp* expression){
    if (expression == NULL){
        return 0;
    }
    return (expression->kind == expKindIdentifier && (strcmp(expression->val.id,"_") == 0));
}
int containsBlank(ExpList* list){
    if (list == NULL){
        return 0;
    }else{
        if (isBlank(list->cur)){
            return 1;
        }else{
            return containsBlank(list->next);
        }
    }
}

RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl) {
	rootNode* r = malloc(sizeof(rootNode));
	r -> packageName = packname;
	r -> startDecls = firstDecl;
	return r;
}

TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl, int multiDecl) {
	TopDeclarationNode* v = malloc(sizeof(TopDeclarationNode*));
	v -> TopDeclarationType = variDeclType;
	v -> multiDecl = multiDecl;
	v -> nextTopDecl = nextTopDecl;
	v -> actualRealDeclaration.varDecl = varDecl;
	return v;
}

Exp *makeExpIdentifier(char *identifier) //How should we handle types?
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindIdentifier;
	e->val.id = identifier;
	return e;
}
Exp *makeExpIntLit(int intLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindIntLit;
	e->val.intLit = intLit;
	return e;
}
Exp *makeExpFloatLit(double floatLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindFloatLit;
	e->val.floatLit = floatLit;
	return e;
}
Exp *makeExpStringLit(ExpressionKind kind, char *stringLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.stringLit = stringLit;
	return e;
}
Exp *makeExpRuneLit(char *runeLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindRuneLit;
	e->val.runeLit = runeLit;
	return e;
}
Exp *makeExpBinary(Exp *left,  Exp *right, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.binary.left = left;
	e->val.binary.right = right;
	return e;
}
Exp *makeExpUnary(Exp *unary, ExpressionKind kind )
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.unary = unary;
	return e;
}
Exp *makeExpAppend(Exp *list, Exp *elem)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindAppend;
	e->val.append.list = list;
	e->val.append.elem = elem;
	return e;
}
Exp *makeExpBuiltInBody(Exp *builtInBody, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.builtInBody = builtInBody;
	return e;
}
Exp *makeExpAccess(Exp *base, Exp * accessor, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.access.base = base;
	e->val.access.accessor = accessor;
	return e;
}
ExpList *addArgument(ExpList * args, Exp * argument)
{
	ExpList *newNode = createArgumentList(argument);
	newNode->next = args;
	return newNode;
}
ExpList *createArgumentList(Exp *argument)
{
	ExpList *l = (ExpList *) malloc(sizeof(ExpList));
	l->cur = argument;
	return l;
}
void reverseArgumentList(ExpList **list)
{
	ExpList *prev = NULL; 
    ExpList *current = *list; 
    ExpList *next = NULL; 
    while ( current != NULL ) 
	{ 
        next = current->next; 
        current->next = prev; 
        prev = current; 
        current = next; 
    } 
    *list = prev; 
}
Exp *makeExpFuncCall(Exp *base, ExpList *arguments)
{
	Exp * e = (Exp *) malloc (sizeof(Exp));
	e->kind = expKindFuncCall;
	e->val.funcCall.base = base;
	reverseArgumentList(&arguments);
	e->val.funcCall.arguments = arguments;
	return e;
}
