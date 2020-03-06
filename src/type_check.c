#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "globalEnum.h"
#include "symbol_table.h"


//so it compiles
#define INT_HOLDER NULL
#define FLOAT_HOLDER NULL;
#define RUNE_HOLDER NULL;
#define STRING_HOLDER NULL;
#define BOOL_HOLDER NULL;

TTEntry *getExpressionType(Exp *e)
{
	if ( e->contextEntry->isSymbol ) return e->contextEntry->entry.s->type;
	return e->contextEntry->entry.t;
}
int isNonCompositeType(TTEntry *t) { return t != NULL && (t->underlyingTypeType == identifierType || t->underlyingTypeType == inferType);}
int isNumericType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->underlyingType == baseInt || t->underlyingType == baseFloat64 || t->underlyingType == baseRune); }
int isIntegerType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->underlyingType == baseInt || t->underlyingType == baseRune); }
int isBool(TTEntry *t) { return t != NULL && isNonCompositeType(t) && t->underlyingType == baseBool; }
int isComparable(TTEntry *t) { return isNonCompositeType(t); }
int isOrdered(TTEntry *t) { return isNumericType(t) || (isNonCompositeType(t) && t->underlyingType == baseString); }


void numericTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a numeric type, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void integerTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a integer type, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void boolTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a bool type, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void comparableTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not comparable, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void orderedTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not ordered, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void notExpressionError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is a type, incompatiable with %s\n", lineno, t->id, operation);
	exit(1);
}
void notMatchingTypes(char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) Operation %s requires equal types for both arguments", lineno, operation);
	exit(1);
}

TTEntry *typeCheckExpression(Exp *e)
{
	TTEntry *typeLeft;
	TTEntry *typeRight;
	TTEntry *type;
	Exp *unaryExp;
	Exp *leftExp;
	Exp *rightExp;
	switch (e->kind)
	{ //TODO make sure that bool lits are properly dealt with...
		case expKindIntLit: return INT_HOLDER; //TODO this is the base base type
		case expKindFloatLit: return FLOAT_HOLDER;
		case expKindRuneLit: return RUNE_HOLDER;
		case expKindRawStringLit:
		case expKindInterpretedStringLit: return STRING_HOLDER
		case expKindIdentifier: return getExpressionType(e); //this might cause issues
		default:
			if ( isUnary(e) )
			{
				unaryExp = e->val.unary;
				type = typeCheckExpression(unaryExp);
				switch (e->kind)
				{
					//there's duplication because I think it's better if the error message says which operation failed
					case expKindUnaryPlus:
						if ( !unaryExp->contextEntry->isSymbol ) notExpressionError(type, "+", e->lineno);  
						else if ( isNumericType(type) ) return type;
						numericTypeError(type, "+", e->lineno);
					case expKindUnaryMinus:
						if ( !unaryExp->contextEntry->isSymbol ) notExpressionError(type, "-", e->lineno);  
						else if ( isNumericType(type) ) return type;
						numericTypeError(type, "-", e->lineno);
					case expKindLogicNot:
						if ( !unaryExp->contextEntry->isSymbol ) notExpressionError(type, "!", e->lineno);  
						else if ( isBool(type) ) return type;
						boolTypeError(type, "!", e->lineno);
					case expKindBitNotUnary:
						if ( !unaryExp->contextEntry->isSymbol ) notExpressionError(type, "^", e->lineno);  
						else if ( isIntegerType(type) ) return type;
						integerTypeError(type, "^", e->lineno);
				}
			}
			else if ( isBinary(e) )
			{
				leftExp = e->val.binary.left;
				rightExp = e->val.binary.right;
				typeLeft = typeCheckExpression(leftExp);
				typeRight = typeCheckExpression(rightExp);
				switch (e->kind)
				{
					case expKindLogicOr:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "||", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("||", e->lineno); //1.2.2 "For two defined types to be identical, they must point to the same type specification
						
						else if ( isBool(typeLeft) ) return typeLeft; //since we do the equality check, this is safe. Also this is the behaviour the ref compiler (returning the overarching type, not base type) follows
						boolTypeError(typeLeft, "||", e->lineno);
					case expKindLogicAnd:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "&&", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("&&", e->lineno);
						
						else if ( isBool(typeLeft) ) return typeLeft;
						boolTypeError(typeLeft, "&&", e->lineno);
					case expKindEQ:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "==", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("==", e->lineno);

						else if ( isComparable(typeLeft) ) return BOOL_HOLDER;
						comparableTypeError(typeLeft, "==", e->lineno);
					case expKindNEQ:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "!=", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("!=", e->lineno);

						else if ( isComparable(typeLeft) ) return BOOL_HOLDER;
						comparableTypeError(typeLeft, "!=", e->lineno);
					case expKindLess:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "<", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("<", e->lineno);

						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, "<", e->lineno); 
					case expKindLEQ:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "<=", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("<=", e->lineno);

						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, "<=", e->lineno); 
					case expKindGreater:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, ">", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes(">", e->lineno);

						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, ">", e->lineno); 
					case expKindGEQ:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, ">=", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes(">=", e->lineno);

						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, ">=", e->lineno); 
					case expKindAddition:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "+", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("+", e->lineno);
						
						else if ( isOrdered(typeLeft) ) return typeLeft; //equivalent to being numeric or string
						fprintf(stderr, "Error: (%d) %s is not a numeric type, nor string, incompatiable with +\n", e->lineno, typeLeft->id);
						exit(1);
					case expKindSubtraction:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "-", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("-", e->lineno);
						
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "-", e->lineno);
					case expKindMultiplication:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "*", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("*", e->lineno);
						
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "*", e->lineno);
					case expKindDivision:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "/", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("/", e->lineno);
						
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "/", e->lineno);
					case expKindMod:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "%", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("%", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "%", e->lineno);
					case expKindBitOr:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "|", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("|", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "|", e->lineno);
					case expKindBitAnd:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "&", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("&", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "&", e->lineno);
					case expKindBitShiftLeft:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "<<", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("<<", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "<<", e->lineno);
					case expKindBitShiftRight:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, ">>", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes(">>", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, ">>", e->lineno);
					case expKindBitAndNot:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "&^", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("&^", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "&^", e->lineno);
					case expKindBitNotBinary:
						if ( !leftExp->contextEntry->isSymbol || !rightExp->contextEntry->isSymbol ) notExpressionError(typeLeft, "^", e->lineno);
						else if ( typeLeft != typeRight ) notMatchingTypes("^", e->lineno);

						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "^", e->lineno);
						
				}
			}
			else
			{ //I switch to if statments here so I can declare stuff
				if ( e->kind == expKindFuncCall )
				{
					TTEntry *baseType = typeCheckExpression(e->val.funcCall.base);
					if ( strcmp(baseType->id, "init") == 0 )
					{
						fprintf(stderr, "Error: (%d) init(.) may not be called\n", e->lineno);
						exit(1);
					}
					if ( baseType->underlyingTypeType == identifierType || baseType->underlyingTypeType == inferType )
					{
						if ( !(e->val.funcCall.arguments != NULL && e->val.funcCall.arguments->next == NULL ) )
						{
							fprintf(stderr, "Error: (%d) Typecasts need exactly one argument\n", e->lineno);
							exit(1);
						}
						TTEntry *toCast = typeCheckExpression(e->val.funcCall.arguments->cur);
						//TODO check if we can type cast arrays and stuff. Not specified in documentation, but
						if ( (toCast->underlyingType == baseType->underlyingType) || (isNumericType(toCast) && isNumericType(baseType)) || (isIntegerType(toCast) && baseType->underlyingType == baseString) )
						{
							return baseType;
						}
						fprintf(stderr, "Error: (%d) Typecasts need to occur with either identical underlying types, numeric types, or an integer type to a string. Received types of %s and %s\n", e->lineno, baseType->id, toCast->id); //maybe make more specific?
						exit(1);
					}
					else if ( baseType->underlyingTypeType != funcType ) 
					{
						fprintf(stderr, "Error: (%d) Can't pass arguments to something of type %s\n", e->lineno, baseType->id); 
						exit(1);
					}
					ExpList * curArgPassed = e->val.funcCall.arguments;
					for ( TTEntryList *curArgType = baseType->val.functionType.args; curArgType; curArgType = curArgType->next )
					{
						if ( curArgPassed == NULL )
						{
							fprintf(stderr, "Error: (%d) %s called with too few arguments\n", e->lineno, baseType->id);
							exit(1);
						}
						TTEntry *curArgPassedType = typeCheckExpression(curArgPassed->cur);
						if ( curArgPassedType != curArgType->cur )
						{
							fprintf(stderr, "Error: (%d) Expected parameter of type %s, received %s\n", e->lineno, curArgType->cur->id, curArgPassedType->id);
							exit(1);
						}
						curArgPassed = curArgPassed->next;
					}
					if ( curArgPassed != NULL )
					{
						fprintf(stderr, "Error: (%d) %s called with too many arguments\n", e->lineno, baseType->id);
						exit(1);
					}
					return baseType->val.functionType.ret;
				}
				else if ( e->kind == expKindIndexing )
				{
					TTEntry * indexType = typeCheckExpression(e->val.access.accessor);
					TTEntry * baseType = typeCheckExpression(e->val.access.base);
					if ( !(isNonCompositeType(indexType) && indexType->underlyingType == baseInt) )
					{
						fprintf(stderr, "Error: (%d) The type of the index was %s, expecting underlying type int\n", e->lineno, indexType->id);
						exit(1);
					}
					if ( !(baseType->underlyingTypeType == arrayType || baseType->underlyingTypeType == sliceType) )
					{
						fprintf(stderr, "Error: (%d) Trying to index an un-indexable type %s\n", e->lineno, baseType->id);
						exit(1);
					}
					return baseType->val.normalType.type;

				}
				else if ( e->kind == expKindFieldSelect )
				{
					TTEntry *baseType = typeCheckExpression(e->val.access.base);
					if ( !(baseType->underlyingTypeType == structType) )
					{
						fprintf(stderr, "Error (%d) Field selection requires a base expression with underlying type struct, received %s\n", e->lineno, baseType->id);
						exit(1);
					}
					for ( EntryTupleList *curField = baseType->val.structType.fields; curField != NULL; curField = curField->next ) 
					{
						if ( strcmp(e->val.access.accessor->val.id, curField->id) == 0 ) return curField->type; //safe field access-- ensured to be id in past passes
					}
					fprintf(stderr, "Error: (%d) Struct %s has no field called %s", e->lineno, baseType->id, e->val.access.accessor->val.id);
					exit(1);
				}
				else if ( e->kind == expKindAppend )
				{
					TTEntry *listType = typeCheckExpression(e->val.append.list);
					TTEntry *elemType = typeCheckExpression(e->val.append.elem);
					if ( !listType->underlyingTypeType == sliceType )
					{
						fprintf(stderr, "Error: (%d) Append requires an expression with underlying type slice, received %s\n", e->lineno, listType->id);
						exit(1);
					}
					if ( elemType != listType->val.normalType.type )
					{
						fprintf(stderr, "Error: (%d) Cannot append elements of type %s to types %s", e->lineno, elemType->id, listType->id);
						exit(1);
					}
					return listType;
				}
				else if ( e->kind == expKindLength )
				{
					TTEntry *bodyType = typeCheckExpression(e->val.builtInBody);
					if ( !(bodyType->underlyingTypeType == sliceType || bodyType->underlyingTypeType == arrayType || ((bodyType->underlyingTypeType == identifierType || bodyType->underlyingTypeType == inferType) && bodyType->underlyingType == baseString)) )
					{
						fprintf(stderr, "Error: (%d) Length requires an expression with underlying type slice or array or string, received %s\n", e->lineno, bodyType->id);
						exit(1);
					}
					return INT_HOLDER; 
				}
				else if (e->kind == expKindCapacity )
				{
					TTEntry *bodyType = typeCheckExpression(e->val.builtInBody);
					if ( !(bodyType->underlyingTypeType == sliceType || bodyType->underlyingTypeType == arrayType) )
					{
						fprintf(stderr, "Error: (%d) Capacity requires an expression with underlying type slice or array, received %s\n", e->lineno, bodyType->id);
						exit(1);
					}
					return INT_HOLDER; 
				}
			}


	}
}
