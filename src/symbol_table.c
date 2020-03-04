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
int main(void) { return 0; }
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
