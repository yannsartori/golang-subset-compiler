#include <stdlib.h>
#include "globalEnum.h"
#include "ast.h"

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
Exp *makeExpFuncCall(Exp *base, ExpList *arguments, ExpressionKind kind)
{
	Exp * e = (Exp *) malloc (sizeof(Exp));
	e->kind = kind;
	e->val.funcCall.base = base;
	e->val.funcCall.arguments = arguments;
	return e;
}
