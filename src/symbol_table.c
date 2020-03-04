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
int main(void) { return 0; } // to compile

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

void symbolCheckExpression(Exp *e, SymbolTable *s, TypeTable *t)
{
	if ( e->kind == expKindIdentifier )
	{
		if ( symbolLookup(e->val.id, s) == NULL )
		{
			fprintf(stderr, "Error: (%d) %s not declared as a variable", e->lineno, e->val.id);
			exit(1);
		}
		e->symbolEntry = symbolLookup(e->val.id, s);
	}
	else if ( e->kind == expKindFieldSelect || e->kind == expKindIndexing )
	{
		symbolCheckExpression(e->val.access.base, s, t);
		symbolCheckExpression(e->val.access.accessor, s, t);
	}
	else if ( e->kind == expKindFuncCall )
	{ //All that matters in this stage is that it exists in A table. Which will matter in typecheck and codegen                   
		if ( symbolLookup(e->val.funcCall.base->val.id, s) == NULL && typeLookup(e->val.funcCall.base->val.id, t) == NULL ) //we did yardwork to ensure that base is an identifier
		{
			fprintf(stderr, "Error: (%d) %s not declared as a variable, nor variable", e->lineno, e->val.funcCall.base->val.id); 
			exit(1);
		}
		e->symbolEntry = symbolLookup(e->val.funcCall.base->val.id, s);
		e->typeEntry = typeLookup(e->val.funcCall.base->val.id, t);
		ExpList *curArg = e->val.funcCall.arguments;
		while ( curArg != NULL )
		{
			symbolCheckExpression(curArg->cur, s, t);
			curArg = curArg->next;
		}
	}
	else if ( isBinary(e) )
	{
		symbolCheckExpression(e->val.binary.left, s, t);
		symbolCheckExpression(e->val.binary.right, s, t);
	}
	else if ( isUnary(e) )
	{
		symbolCheckExpression(e->val.unary, s, t);		
	}
	else if ( e->kind == expKindCapacity || e->kind == expKindLength )
	{
		symbolCheckExpression(e->val.builtInBody, s, t);
	}
	else if ( e->kind == expKindAppend )
	{
		symbolCheckExpression(e->val.append.list, s, t);
		symbolCheckExpression(e->val.append.elem, s, t);
	}
	//do nothing if lit
}
