#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "globalEnum.h"
#include "symbol_table.h"

SymbolTable *globalSymbolTable;
TypeTable *globalTypeTable;
//CURRENT PLACEHOLDERS: TODO
// STEntry * symbolLookup(char *id, SymbolTable *s)
// TTEntry * typeLookup(char *id, TypeTable *t)
STEntry * symbolLookup(char *id, SymbolTable *s);
TTEntry * typeLookup(char *id, TypeTable *t);


void symbolCheckExpressionList(ExpList* expressionList,Context* context);
void symbolCheckSwitchCaseClauseList(switchCaseClause* clauseList, Context* context);




int hashCode(char * id)
{
	unsigned int hash = 0;
	while ( *id ) hash = (hash << 1) + *(id++);
	return hash % TABLE_SIZE;
}

Context * newContext()
{
	Context *c = (Context *) malloc(sizeof(Context));
	SymbolTable *s = (SymbolTable *) malloc(sizeof(SymbolTable));
	TypeTable *t = (TypeTable *) malloc(sizeof(TypeTable));
	for ( int i = 0; i < TABLE_SIZE; i++ )
	{
		s->entries[i] = NULL;
		t->entries[i] = NULL;
	}
	c->curTypeTable = t;
	c->curSymbolTable = s;
	return c;
}

Context * scopedContext(Context *c)
{
	Context *newC = newContext();
	newC->parent = c;
	return newC;
}

int addSymbolEntry(Context *c, STEntry *s)
{
	int pos = hashCode(s->id);
	for ( STEntry *head = c->curSymbolTable->entries[pos]; head; head = head->next )
	{
		if ( strcmp(head->id, s->id) == 0 ) return 0; //TODO depending on Denali wants to handle it, we can also just either (1) put the error code here for more descriptive messaging or (2) have custom return types (1 == already symbol at scope, 2 == already type at scope, 0 fine) and raise the error in the calling method. Involves inspecting return codes
	}
	for ( TTEntry *head = c->curTypeTable->entries[pos]; head; head = head->next )
	{
		if ( strcmp(head->id, s->id) == 0 ) return 0;
	}
}

int addTypeEntry(Context *c, TTEntry *t)
{
	int pos = hashCode(t->id);
	for ( STEntry *head = c->curSymbolTable->entries[pos]; head; head = head->next )
	{
		if ( strcmp(head->id, t->id) == 0 ) return 0; //TODO depending on Denali wants to handle it, we can also just either (1) put the error code here for more descriptive messaging or (2) have custom return types (1 == already symbol at scope, 2 == already type at scope, 0 fine) and raise the error in the calling method. Involves inspecting return codes
	}
	for ( TTEntry *head = c->curTypeTable->entries[pos]; head; head = head->next )
	{
		if ( strcmp(head->id, t->id) == 0 ) return 0;
	}

}

PolymorphicEntry *getEntry(Context *c, char *id)
{
	int pos = hashCode(id);
	for ( STEntry *head = c->curSymbolTable->entries[pos]; head; head = head->next )
	{ //If the adding was done correctly, it doesn't matter in which order we traverse-- the entry will only exist in max one table per scope
		if ( strcmp(head->id, id) == 0 )
		{
			PolymorphicEntry *e = (PolymorphicEntry *) malloc(sizeof(PolymorphicEntry));
			e->isSymbol = 1;
			e->entry.s = head;
			return e;
		}
	}
	for ( TTEntry *head = c->curTypeTable->entries[pos]; head; head = head->next )
	{
		if ( strcmp(head->id, id) == 0 )
		{
			PolymorphicEntry *e = (PolymorphicEntry *) malloc(sizeof(PolymorphicEntry));
			e->isSymbol = 0;
			e->entry.t = head;
			return e;
		}
	}
	if ( c->parent == NULL ) return NULL;
	return getEntry(c->parent, id);
}

void symbolCheckExpression(Exp *e, Context* context)
{
	return;	
}




void symbolCheckStatement(Stmt* stmt, Context* context){
	if (stmt == NULL){
		return;
	}

	Context* newContext;

	if (context == NULL){
		puts("Context shouldn't be NULL in symbolCheckSatement");
		exit(1);
	}

	switch (stmt->kind){


		//TODO
		case StmtKindSwitch :
							newContext = scopedContext(context);
							symbolCheckStatement(stmt->val.switchStmt.statement,newContext);
							symbolCheckExpression(stmt->val.switchStmt.expression,newContext);


							//I'm in sense encasing the clasue list in a block from the perspective of scope
							symbolCheckSwitchCaseClauseList(stmt->val.switchStmt.clauseList,scopedContext(newContext));
							break;



		case StmtKindBlock :
						newContext = scopedContext(context);
						symbolCheckStatement(stmt->val.block.stmt,newContext);
						break;




    	case StmtKindExpression: //TODO (symbolCheckExpression signature, implementation need to change)
						symbolCheckExpression(stmt->val.expression.expr,context);
						break;


		//lvalue assignability checks done at typechecking
		case StmtKindAssignment: 
								symbolCheckExpressionList(stmt->val.assignment.lhs,context);
								symbolCheckExpressionList(stmt->val.assignment.rhs,context);
								break;
	
		

		case StmtKindPrint :
							symbolCheckExpressionList(stmt->val.print.list,context);
							break;
		case StmtKindPrintln : 
							symbolCheckExpressionList(stmt->val.print.list,context);
							break;
		case StmtKindIf :
						newContext = scopedContext(context);

						symbolCheckStatement(stmt->val.ifStmt.statement,newContext);
						if(stmt->val.ifStmt.expression != NULL){
							symbolCheckExpression(stmt->val.ifStmt.expression,newContext);
						}
						symbolCheckStatement(stmt->val.ifStmt.block,newContext);

						symbolCheckStatement(stmt->val.ifStmt.elseBlock,context);

						break;
		case StmtKindReturn :
							if (stmt->val.returnVal.returnVal != NULL){
								symbolCheckExpression(stmt->val.returnVal.returnVal,context);
							}
							break;
		case StmtKindElse :
							symbolCheckStatement(stmt->val.elseStmt.block,context);
							break;
		
		case StmtKindInfLoop : symbolCheckStatement(stmt->val.infLoop.block,context);
								break;
		case StmtKindWhileLoop :
								symbolCheckExpression(stmt->val.whileLoop.conditon,context);
								symbolCheckStatement(stmt->val.whileLoop.block,context);
								break;
		case StmtKindThreePartLoop :
									newContext = scopedContext(context);
									symbolCheckStatement(stmt->val.forLoop.init,newContext);
									if (stmt->val.forLoop.condition != NULL){
										symbolCheckExpression(stmt->val.forLoop.condition,newContext);
									}
									symbolCheckStatement(stmt->val.forLoop.inc,newContext);
									symbolCheckStatement(stmt->val.forLoop.block,newContext);
									
									break;


		//Trivially symbolcheck
		case StmtKindBreak :break;
		case StmtKindContinue :break;
		








		//For Denali to implement
		case StmtKindTypeDeclaration :break;
		case StmtKindVarDeclaration :break;
		case StmtKindShortDeclaration : break; //Short declaration needs to contain a new variable
	}

	symbolCheckStatement(stmt->next,context);
}

void symbolCheckExpressionList(ExpList* expressionList,Context* context){
	if (expressionList == NULL){
		return;
	}

	symbolCheckExpression(expressionList->cur,context);
	symbolCheckExpressionList(expressionList->next,context);

	
}


void symbolCheckSwitchCaseClauseList(switchCaseClause* clauseList, Context* context){
	if (clauseList == NULL){
		return;
	}

	symbolCheckExpressionList(clauseList->expressionList,context);
	symbolCheckStatement(clauseList->statementList,context);

	symbolCheckSwitchCaseClauseList(clauseList->next,context);
}