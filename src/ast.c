#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
	RootNode* r = malloc(sizeof(RootNode));
	r -> packageName = packName;
	r -> startDecls = firstDecl;
	return r;
}

TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* v = malloc(sizeof(TopDeclarationNode*));
	v -> declType = variDeclType;
	v -> nextTopDecl = nextTopDecl;
	v -> actualRealDeclaration.varDecl = varDecl;
	return v;
}

TypeHolderNode* makeArrayHolder(Exp* arraySize, char* id){
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode*));
	t -> kind = arrayType;
	t -> identification = id;
	t -> arrayDims = arraySize;
	return t;
}

TypeHolderNode* makeStructHolder(TypeDeclNode* members) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode*));
	t -> kind = structType;
	t -> structMembers = members;
	return t;
}

TypeHolderNode* makeSliceHolder(char* id) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode*));
	t -> kind = sliceType;
	t -> identification = id;
	return t;
}

TypeHolderNode* makeIdTypeHolder(char* id) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode*));
	t -> kind = identifierType;
	t -> identification = id;
	return t;
}

IdChain* makeIdChain(char* identifier, IdChain* next) {
	IdChain* c = malloc(sizeof(IdChain));
	c -> identifier = identifier;
	c -> next = next;
	return c;
}

TypeDeclNode* makeSingleTypeDecl(IdChain* identifiers, TypeHolderNode* givenType) {
	TypeDeclNode* t = malloc(sizeof(TypeDeclNode));
	t -> actualType = givenType;
	t -> identifier = identifiers -> identifier;
	TypeDeclNode* temp = t;
	IdChain* iter = identifiers;
	while (iter -> next != NULL) {
		temp = temp -> nextDecl;
		iter = iter -> next;
		temp = malloc(sizeof(TypeDeclNode));
		temp -> identifier = iter -> identifier;
		temp -> actualType = givenType;
	}
	temp -> nextDecl = NULL;
	return t;
}

TopDeclarationNode* makeTopTypeDecl(TypeDeclNode* typeDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* t = malloc(sizeof(TopDeclarationNode*));
	t -> declType = typeDeclType;
	t -> nextTopDecl = nextTopDecl;
	t -> actualRealDeclaration.typeDecl = typeDecl;
	return t;
}

void appendTypeDecls(TypeDeclNode* baseDecl, TypeDeclNode* leafDecl) {
	TypeDeclNode* iter = baseDecl;
	while (iter -> nextDecl != NULL) {
		iter = iter -> nextDecl;
	}
	iter -> nextDecl = leafDecl;
}

void appendVarDecls(VarDeclNode* baseDecl, VarDeclNode* leafDecl) {
	VarDeclNode* iter = baseDecl;
	while (iter -> nextDecl != NULL) {
		iter = iter -> nextDecl;
	}
	iter -> nextDecl = leafDecl;
}

VarDeclNode* makeSingleVarDeclNoExps(IdChain* identifiers, TypeHolderNode* givenType) {
	printf("just before the malloc, in the makeSingleVarDeclNoExps\n");
	VarDeclNode* v = malloc(sizeof(VarDeclNode));
	printf("2\n");
	v -> typeThing = givenType;
	v -> identifier = identifiers -> identifier;
	v -> value = NULL;
	VarDeclNode* temp = v;
	IdChain* iter = identifiers;
	while (iter -> next != NULL) {
		printf("x\n");
		temp -> nextDecl = malloc(sizeof(VarDeclNode));
		temp = temp -> nextDecl;
		iter = iter -> next;
		temp -> identifier = iter -> identifier;
		temp -> value = NULL;
		temp -> typeThing = givenType;
	}
	temp -> nextDecl = NULL;
	return v;
}

VarDeclNode* makeSingleVarDeclWithExps(IdChain* identifiers, TypeHolderNode* givenType, ExpList* values) {
	ExpList* valIter = values;
	reverseArgumentList(&valIter);
	VarDeclNode* t = malloc(sizeof(VarDeclNode));
	t -> typeThing = givenType;
	t -> identifier = identifiers -> identifier;
	t -> value = valIter -> cur;
	VarDeclNode* temp = t;
	IdChain* iter = identifiers;
	while (iter -> next != NULL) {
		temp -> nextDecl = malloc(sizeof(VarDeclNode));
		temp = temp -> nextDecl;
		iter = iter -> next;
		valIter = valIter -> next;
		temp -> identifier = iter -> identifier;
		if (valIter == NULL) {
			fprintf(stderr, "Error: wrong number of expressions for assignment.\n");
			exit(1);
		}
		temp -> value = valIter -> cur;
		temp -> typeThing = givenType;
	}
	if (valIter -> next != NULL) {
		fprintf(stderr, "Error: wrong number of expressions for assignment.\n");
		exit(1);
	}
	temp -> nextDecl = NULL;
	return t;
}

FuncDeclNode* makeFuncDecl(char* funcName, TypeDeclNode* argsDecls, TypeHolderNode* returnType, void* blockStart) {
	FuncDeclNode* f = malloc(sizeof(FuncDeclNode));
	f -> identifier = funcName;
	f -> argsDecls = argsDecls;
	f -> returnType = returnType;
	f -> blockStart = blockStart;
	return f;
}

TopDeclarationNode* makeTopFuncDecl(FuncDeclNode* funcDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* t = malloc(sizeof(TopDeclarationNode*));
	t -> declType = typeDeclType;
	t -> nextTopDecl = nextTopDecl;
	t -> actualRealDeclaration.funcDecl = funcDecl;
	return t;
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
