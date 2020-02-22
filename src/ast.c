#include <stdlib.h>
#include "globalEnum.h"
#include <stdio.h>
#include "ast.h"

int yylineno;
int weedSwitchClause(switchCaseClause* clauseList, State loopState, State switchState, State functionState);
int weedStatement(Stmt* stmt, State loopState, State switchState, State functionState);


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
        
        case StmtKindBlock : return weedStatement(stmt->val.block.stmt,loopState,switchState,functionState);
                            break;
        case StmtKindExpression : break;
        case StmtKindAssignment :break;
    


        case StmtKindPrint :break;
        case StmtKindPrintln :break;
        case StmtKindIf :break;
        case StmtKindReturn :   if (functionState == outside){
                                    fprintf(stderr,"Error: (line %d) return statements must occur inside a function",stmt->lineno);
                                    exit(1);
                                }
                                break;
        case StmtKindElse :  return weedStatement(stmt->val.elseStmt.block,loopState,switchState,functionState);
                            break;
        case StmtKindSwitch : return weedSwitchClause(stmt->val.switchStmt.clauseList,loopState,inSwitchStatement,functionState);
                                break;

       
        case StmtKindInfLoop : return weedStatement(stmt->val.infLoop.block,inLoop,switchState,functionState);
                                break;
        case StmtKindWhileLoop : return weedStatement(stmt->val.whileLoop.block,inLoop,switchState,functionState);
                                break;


         //TODO the inc condition of the three part for loop cannot be a short declaration
        case StmtKindThreePartLoop :return weedStatement(stmt->val.whileLoop.block,inLoop,switchState,functionState);
                                    break;

        case StmtKindBreak :    
                            if (loopState != inLoop || switchState != inSwitchStatement){
                                fprintf(stderr,"Error: (line %d) break statements must occur inside a loop or a switch statement",stmt->lineno);
                                exit(1);
                            }
                            break;
        case StmtKindContinue : if (loopState != inLoop){
                                    fprintf(stderr,"Error: (line %d) continue statements must occur inside a loopt",stmt->lineno);
                                    exit(1);
                                }
                                break;


        //StmtKindDeclaration, //TODO
        //StmtKindShortDeclaration, //TODO


    }

    return 0;
}




int weedSwitchClause(switchCaseClause* clauseList, State loopState, State switchState, State functionState){
    if (clauseList == NULL){
        return 0;
    }

    int n = weedStatement(clauseList->statementList,loopState,switchState,functionState);
    if (n != 0){
        return n;
    }

    return weedSwitchClause(clauseList->next,loopState,switchState,functionState);

    

}























//Yann's functions







Exp *makeExpIdentifier(char *identifier) //How should we handle types?

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
Exp *makeExpFuncCall(Exp *base, ExpList *arguments, ExpressionKind kind)
{
	Exp * e = (Exp *) malloc (sizeof(Exp));
	e->kind = kind;
	e->val.funcCall.base = base;
	e->val.funcCall.arguments = arguments;

    e->isBracketed = 0;
	return e;
}


//Denali's functions


RootNode* makeRootNode(char* packName, TopDeclarationNode* firstDecl) {
	RootNode* r = malloc(sizeof(struct RootNode));
	r -> packageName = packName;
	r -> startDecls = firstDecl;
	return r;
}

TopDeclarationNode* makeTopVarDecl(VarDeclNode* varDecl, TopDeclarationNode* nextTopDecl, int multiDecl) {
	TopDeclarationNode* v = malloc(sizeof(TopDeclarationNode*));
	v -> declType = variDeclType;
	v -> multiDecl = multiDecl;
	v -> nextTopDecl = nextTopDecl;
	v -> actualRealDeclaration.varDecl = varDecl;
	return v;

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

