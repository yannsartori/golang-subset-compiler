#include <stdlib.h>
#include "globalEnum.h"
#include "AST.h"



Stmt* makeBlockStmt(Stmt* stmt){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindBlock;
    ptr->val.block.stmt = stmt;
    return ptr;
}


Stmt* makeExpressionStmt(Exp* expression){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindExpression;
    ptr->val.expression.expr = expression;
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

    return ptr;
}



Stmt* makePrintStmt(ExpList* list){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindPrint;
    ptr->val.print.list = list;

    return ptr;
}

Stmt* makePrintlnStmt(ExpList* list){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindPrintln;
    ptr->val.print.list = list;

    return ptr;
}



Stmt* makeReturnStmt(Exp* expr){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindReturn;
    ptr->val.returnVal.returnVal = expr;

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

    return ptr;
}

Stmt* makeElseStmt(Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindElse;
    ptr->val.elseStmt.block = block;

    return ptr;
}

Stmt* makeInfLoopStmt(Stmt* block){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindInfLoop;
    ptr->val.infLoop.block = block;

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

    return ptr;
}


Stmt* makeBreakStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindBreak;
    return ptr;

}


Stmt* makeContinueStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindContinue;
    return ptr;

}

Stmt* makeFallthroughStmt(){
    Stmt* ptr = malloc(sizeof(Stmt));
    if (ptr == NULL){
        return NULL;
    }

    ptr->kind = StmtKindFallthrough;
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
    if (expression->kind == expKindIdentifier){
        if (strcmp(expression->val.id,"_") == 0){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
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





























//Yann's functions







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
Exp *makeExpRuneLit(char* runeLit)
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