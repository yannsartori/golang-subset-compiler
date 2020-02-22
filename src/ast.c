#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "globalEnum.h"
#include "ast.h"

int yylineno;
int weedSwitchClause(switchCaseClause* clauseList, State loopState, State switchState, State functionState);
int weedStatement(Stmt* stmt, State loopState, State switchState, State functionState);
int defaultClauseCount(switchCaseClause* clauseList);

Stmt* makeBlockStmt(Stmt* stmt){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindBlock;
    ptr->val.block.stmt = stmt;

    ptr->lineno = yylineno;

    return ptr;
}


Stmt* makeExpressionStmt(Exp* expression){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindExpression;
    ptr->val.expression.expr = expression;

    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makeAssignmentStmt(ExpList* lhs, ExpList* rhs){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindAssignment;
    ptr->val.assignment.lhs = lhs;
     ptr->val.assignment.rhs = rhs;

     ptr->lineno = yylineno;

    return ptr;
}



Stmt* makePrintStmt(ExpList* list){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindPrint;
    ptr->val.print.list = list;

    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makePrintlnStmt(ExpList* list){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindPrintln;
    ptr->val.print.list = list;

    ptr->lineno = yylineno;

    return ptr;
}



Stmt* makeReturnStmt(Exp* expr){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindReturn;
    ptr->val.returnVal.returnVal = expr;

    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makeIfStmt(Stmt* statement, Exp* expression, Stmt* block,Stmt* elseBlock){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

 

    ptr->kind = StmtKindIf;
    ptr->val.ifStmt.statement = statement;
    ptr->val.ifStmt.expression= expression;
    ptr->val.ifStmt.block = block;
    ptr->val.ifStmt.elseBlock = elseBlock;


    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makeElseStmt(Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindElse;
    ptr->val.elseStmt.block = block;

    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makeInfLoopStmt(Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindInfLoop;
    ptr->val.infLoop.block = block;

    ptr->lineno = yylineno;

    return ptr;
}



Stmt* makeWhileLoopStmt(Exp* condition, Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindWhileLoop;
    ptr->val.whileLoop.block = block;
    ptr->val.whileLoop.conditon = condition;

    ptr->lineno = yylineno;

    return ptr;
}



Stmt* makeThreePartLoopStmt(Stmt* init, Exp* condition, Stmt* inc, Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindThreePartLoop;
    ptr->val.forLoop.init = init;
    ptr->val.forLoop.condition = condition;
    ptr->val.forLoop.inc = inc;
    ptr->val.forLoop.block = block;


    ptr->lineno = yylineno;

    return ptr;
}

Stmt* makeSwitchStmt(Stmt* statement, Exp* expression, switchCaseClause* clauseList){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindSwitch;
    
    ptr->val.switchStmt.statement = statement;
    ptr->val.switchStmt.expression = expression;
    ptr->val.switchStmt.clauseList = clauseList;


    ptr->lineno = yylineno;

    return ptr;
}


Stmt* makeBreakStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindBreak;

    ptr->lineno = yylineno;

    return ptr;

}


Stmt* makeContinueStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindContinue;

    ptr->lineno = yylineno;

    return ptr;

}

Stmt* makeFallthroughStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindFallthrough;

    ptr->lineno = yylineno;
    return ptr;

}



switchCaseClause* makeSwitchCaseClause(ExpList* expressionList, Stmt* statementList){
    switchCaseClause* ptr = malloc(sizeof(switchCaseClause));
    
    if (ptr == NULL){
        return NULL;
    }

    ptr->expressionList = expressionList;
    ptr->statementList = statementList;

    return ptr;
}






ExpList* reverseListPrivate(ExpList* correct,ExpList* reversed){
    if (reversed == NULL){
        return correct;
    }else{
        ExpList* newHead = reversed;
        ExpList* newTail = reversed->next;
        newHead->next = correct;
        return reverseListPrivate(newHead,newTail);

    }


}


ExpList* reverseList(ExpList* reversed){
    return reverseListPrivate(NULL,reversed);
}






Stmt* reverseStmtListPrivate(Stmt* correct,Stmt* reversed){
    if (reversed == NULL){
        return correct;
    }else{
        Stmt* newHead = reversed;
        Stmt* newTail = reversed->next;
        newHead->next = correct;
        return reverseStmtListPrivate(newHead,newTail);

    }


}


Stmt* reverseStmtList(Stmt* reversed){
    return reverseStmtListPrivate(NULL,reversed);
}

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


int expListLength(ExpList* list){
    if (list == NULL){
        return 0;
    }

    return 1 + expListLength(list->next);


}


int isFuncCall(Exp* expression){
    if (expression == NULL){
        return 0;
    }

    if (expression->kind == expKindFuncCall){
        return 1;
    }

    return 0;
}

Stmt* cons(Stmt* head,Stmt* tail){
    if (head == NULL){
        return tail;
    }else if(tail == NULL){
        return head;
    }else{
        head->next = tail;
        return head;
    }
}






int weedStatement(Stmt* stmt, State loopState, State switchState, State functionState){
    if (stmt == NULL){
        return 0;
    }


    switch (stmt->kind)
    
    {
        
        case StmtKindBlock : weedStatement(stmt->val.block.stmt,loopState,switchState,functionState);
                            break;
        case StmtKindExpression : break;
        case StmtKindAssignment :break;
    


        case StmtKindPrint :break;
        case StmtKindPrintln :break;
        case StmtKindIf :   weedStatement(stmt->val.ifStmt.block,loopState,switchState,functionState);
                            break;
        case StmtKindReturn :   if (functionState == outside){
                                    fprintf(stderr,"Error: (line %d) return statements must occur inside a function\n",stmt->lineno);
                                    exit(1);
                                }
                                break;
        case StmtKindElse :  weedStatement(stmt->val.elseStmt.block,loopState,switchState,functionState);
                            break;
        case StmtKindSwitch :   if(defaultClauseCount(stmt->val.switchStmt.clauseList) > 1){
                                    fprintf(stderr,"Error: (line %d) switch statement contains multiple default clauses\n",stmt->lineno);
                                    exit(1);
                                }
                                weedSwitchClause(stmt->val.switchStmt.clauseList,loopState,inSwitchStatement,functionState);
                                break;

       
        case StmtKindInfLoop : weedStatement(stmt->val.infLoop.block,inLoop,switchState,functionState);
                                break;
        case StmtKindWhileLoop : weedStatement(stmt->val.whileLoop.block,inLoop,switchState,functionState);
                                break;


         //TODO the inc condition of the three part for loop cannot be a short declaration
        case StmtKindThreePartLoop :weedStatement(stmt->val.whileLoop.block,inLoop,switchState,functionState);
                                    break;

        case StmtKindBreak :  
                            if (loopState != inLoop && switchState != inSwitchStatement){
                                fprintf(stderr,"Error: (line %d) break statements must occur inside a loop or a switch statement\n",stmt->lineno);
                                exit(1);
                            }
                            break;
        case StmtKindContinue : 
                                if (loopState != inLoop){
                                    fprintf(stderr,"Error: (line %d) continue statements must occur inside a loop\n",stmt->lineno);
                                    exit(1);
                                }
                                
                                break;


        //StmtKindDeclaration, //TODO
        //StmtKindShortDeclaration, //TODO


    }




    return weedStatement(stmt->next,loopState,switchState,functionState);
}


int weed(Stmt* stmt){
    return weedStatement(stmt,outside,outside,outside);
}

int weedSwitchClause(switchCaseClause* clauseList, State loopState, State switchState, State functionState){

    if (clauseList == NULL){
        return 0;
    }

    int n = weedStatement(clauseList->statementList,loopState,switchState,functionState);
    

    return weedSwitchClause(clauseList->next,loopState,switchState,functionState);

    

}

int defaultClauseCount(switchCaseClause* clauseList){
    if (clauseList == NULL){
        return 0;
    }

    if (clauseList->expressionList == NULL){ // Is defualt
        return 1 + defaultClauseCount(clauseList->next);
    }

    return defaultClauseCount(clauseList->next);
}





Exp *makeExpIdentifier(char *identifier) 
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindIdentifier;
	e->val.id = identifier;
	e->isBracketed = 0;
	return e;
}
Exp *makeExpIntLit(int intLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindIntLit;
	e->val.intLit = intLit;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpFloatLit(double floatLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindFloatLit;
	e->val.floatLit = floatLit;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpStringLit(ExpressionKind kind, char *stringLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.stringLit = stringLit;
  e->isBracketed = 0;
	return e;
}

Exp *makeExpRuneLit(char *runeLit)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindRuneLit;
	e->val.runeLit = runeLit;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpBinary(Exp *left,  Exp *right, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.binary.left = left;
	e->val.binary.right = right;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpUnary(Exp *unary, ExpressionKind kind )
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.unary = unary;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpAppend(Exp *list, Exp *elem)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = expKindAppend;
	e->val.append.list = list;
	e->val.append.elem = elem;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpBuiltInBody(Exp *builtInBody, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.builtInBody = builtInBody;
  e->isBracketed = 0;
	return e;
}
Exp *makeExpAccess(Exp *base, Exp * accessor, ExpressionKind kind)
{
	Exp *e = (Exp *) malloc(sizeof(Exp));
	e->kind = kind;
	e->val.access.base = base;
	e->val.access.accessor = accessor;
    e->isBracketed = 0;
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
Exp *makeExpFuncCall(Exp *base, ExpList *arguments)
{
	Exp * e = (Exp *) malloc (sizeof(Exp));
	e->kind = expKindFuncCall;
	e->val.funcCall.base = base;
	e->val.funcCall.arguments = reverseList(arguments);	
    e->isBracketed = 0;
	return e;
}


//Denali's functions


RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl) {
	if (strcmp(packName, "_") == 0) {
		fprintf(stderr, "Package name cannot be the blank identifier.\n");
		exit(1);
	}
	RootNode* r = malloc(sizeof(RootNode));
	r -> packageName = packName;
	r -> startDecls = firstDecl;
	return r;
}

TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* v = malloc(sizeof(TopDeclarationNode));
	v -> declType = variDeclType;
	v -> nextTopDecl = nextTopDecl;
	v -> actualRealDeclaration.varDecl = varDecl;
	return v;
}

TypeHolderNode* makeArrayHolder(Exp* arraySize, TypeHolderNode* id){
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode));
	t -> kind = arrayType;
	t -> underlyingType = id;
	t -> arrayDims = arraySize;
	return t;
}

TypeHolderNode* makeStructHolder(TypeDeclNode* members) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode));
	t -> kind = structType;
	t -> structMembers = members;
	return t;
}

TypeHolderNode* makeSliceHolder(TypeHolderNode* id) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode));
	t -> kind = sliceType;
	t -> underlyingType = id;
	return t;
}

TypeHolderNode* makeIdTypeHolder(char* id) {
	TypeHolderNode* t = malloc(sizeof(TypeHolderNode));
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
		temp -> nextDecl = malloc(sizeof(TypeDeclNode));
		temp = temp -> nextDecl;
		iter = iter -> next;
		temp -> identifier = iter -> identifier;
		temp -> actualType = givenType;
	}
	temp -> nextDecl = NULL;
	return t;
}

TopDeclarationNode* makeTopTypeDecl(TypeDeclNode* typeDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* t = malloc(sizeof(TopDeclarationNode));
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
	VarDeclNode* v = malloc(sizeof(VarDeclNode));
	v -> typeThing = givenType;
	v -> identifier = identifiers -> identifier;
	v -> value = NULL;
	VarDeclNode* temp = v;
	IdChain* iter = identifiers;
	while (iter -> next != NULL) {
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

VarDeclNode* makeSingleVarDeclWithExps(IdChain* identifiers, TypeHolderNode* givenType, ExpList* values, int lineno) {
	ExpList* valIter = reverseList(values);
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
			fprintf(stderr, "Error: wrong number of expressions for assignment on line %d.\n", lineno);
			exit(1);
		}
		temp -> value = valIter -> cur;
		temp -> typeThing = givenType;
	}
	if (valIter -> next != NULL) {
		fprintf(stderr, "Error: wrong number of expressions for assignment on line %d.\n", lineno);
		exit(1);
	}
	temp -> nextDecl = NULL;
	return t;
}

FuncDeclNode* makeFuncDecl(char* funcName, TypeDeclNode* argsDecls, TypeHolderNode* returnType, Stmt* blockStart) {
	FuncDeclNode* f = malloc(sizeof(FuncDeclNode));
	f -> identifier = funcName;
	f -> argsDecls = argsDecls;
	f -> returnType = returnType;
	f -> blockStart = blockStart;
	return f;
}

TopDeclarationNode* makeTopFuncDecl(FuncDeclNode* funcDecl, TopDeclarationNode* nextTopDecl) {
	TopDeclarationNode* t = malloc(sizeof(TopDeclarationNode));
	t -> declType = funcDeclType;
	t -> nextTopDecl = nextTopDecl;
	t -> actualRealDeclaration.funcDecl = funcDecl;
	return t;
}

Stmt* makeVarDeclStatement(VarDeclNode* declaration, int isShort, int lineNomber){
	Stmt* s = malloc(sizeof(Stmt));
	s -> val.varDeclaration = declaration;
	if (isShort) {
		s -> kind = StmtKindShortVarDecl;
	} else {
		s -> kind = StmtKindVarDeclaration;
	}
	s -> lineno = lineNomber;
	return s;
}

Stmt* makeTypeDeclStatement(TypeDeclNode* declaration, int lineNomber){
	Stmt* s = malloc(sizeof(Stmt));
	s -> val.typeDeclaration = declaration;
	s -> kind = StmtKindTypeDeclaration;
	s -> lineno = lineNomber;
	return s;
}

IdChain* extractIdList(ExpList* expressions, int lineno) {
	if (expressions == NULL) return NULL;
	IdChain* base = malloc(sizeof(IdChain));
	if (expressions -> cur -> kind != expKindIdentifier) {
		fprintf(stderr, "Error: expected identifier, found something else on line %d\n", lineno);
		exit(1);
	}
	base -> identifier = expressions -> cur -> val.id;
	ExpList* expIter = expressions;
	IdChain* idIter = base;
	while (expIter -> next != NULL) {
		expIter = expIter -> next;
		idIter -> next = malloc(sizeof(IdChain));
		idIter = idIter -> next;
		if (expIter -> cur -> kind != expKindIdentifier) {
			fprintf(stderr, "Error: expected identifier, found something else on line %d\n", lineno);
			exit(1);
		}
		idIter -> identifier = expIter -> cur -> val.id;
	}
	idIter -> next = NULL;
	return base;
}
