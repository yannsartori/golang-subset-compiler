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
int isNonCompositeType(TTEntry *t) { return t != NULL && t->underlyingType == identifierType;}
int isNumericType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseFloat64 || t->val.nonCompositeType.type == baseRune); }
int isIntegerType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseRune); }
int isBool(TTEntry *t) { return t != NULL && isNonCompositeType(t) && t->val.nonCompositeType.type == baseBool; }
int isOrdered(TTEntry *t) { return isNumericType(t) || (isNonCompositeType(t) && t->val.nonCompositeType.type == baseString); }
int isComparable(TTEntry *t) { 
	if ( t->underlyingType == funcType || t->underlyingType == sliceType ) return 0;
	if ( t->underlyingType == arrayType ) return isComparable(t->val.arrayType.type);
	if ( t->underlyingType == structType )
	{
		for ( EntryTupleList *curField = t->val.structType.fields; curField != NULL; curField = curField->next ) 
		{
			if ( !isComparable(curField->type) ) return 0; 
		}
	}
	return 1;
}
int typeEquality(TTEntry *t1, TTEntry *t2) //only used for comparison operations
{
	if ( t1->underlyingType == arrayType && t2->underlyingType == arrayType )
	{
		return typeEquality(t1->val.arrayType.type, t2->val.arrayType.type) && t1->val.arrayType.size == t2->val.arrayType.size;
	}
	if ( t1->underlyingType == sliceType && t2->underlyingType == sliceType )
	{
		return typeEquality(t1->val.sliceType.type, t2->val.sliceType.type);
	}
	return t1 == t2;
}
char * typeToString(TTEntry *t)
{ //this might freak out pointers spook me
	char * str = (char *) calloc(100, sizeof(char));
	if ( t->underlyingType == arrayType )
	{
		sprintf(str, "[%d]%s", t->val.arrayType.size, typeToString(t->val.arrayType.type));
	}
	else if ( t->underlyingType == sliceType )
	{
		sprintf(str, "[]%s", typeToString(t->val.sliceType.type));
	}
	else
	{
		sprintf(str, "%s", t->id);
	}
	char * actualRet = (char *) calloc(strlen(str) + 1, sizeof(char));
	strcpy(actualRet, str);
	free(str);
	return actualRet;
}
void numericTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a numeric type, incompatiable with %s\n", lineno, typeToString(t), operation);
	exit(1);
}
void integerTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a integer type, incompatiable with %s\n", lineno, typeToString(t), operation);
	exit(1);
}
void boolTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a bool type, incompatiable with %s\n", lineno, typeToString(t), operation);
	exit(1);
}
void comparableTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not comparable, incompatiable with %s\n", lineno, typeToString(t), operation);
	exit(1);
}
void orderedTypeError(TTEntry *t, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not ordered, incompatiable with %s\n", lineno, typeToString(t), operation);
	exit(1);
}
void notExpressionError(TTEntry *t, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is a type, cannot take part in expressions\n", lineno, typeToString(t));
	exit(1);
}
void notMatchingTypes(TTEntry *t1, TTEntry *t2, char *operation, int lineno)
{
	fprintf(stderr, "Error: (%d) Operation %s requires equal types for both arguments (got %s and %s)", lineno, operation, typeToString(t1), typeToString(t2));
	exit(1);
}

TTEntry *typeCheckExpression(Exp *e) //Note: this rejects any expressions with types as identifiers. I can't think of any instances where allowing them is desirable...
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
		case expKindIdentifier:
			if ( !e->contextEntry->isSymbol ) notExpressionError(getExpressionType(e), e->lineno);
			return getExpressionType(e);
		default:
			if ( isUnary(e) )
			{
				unaryExp = e->val.unary;
				type = typeCheckExpression(unaryExp);
				switch (e->kind)
				{
					case expKindUnaryPlus:
						if ( isNumericType(type) ) return type;
						numericTypeError(type, "+", e->lineno);
					case expKindUnaryMinus:
						if ( isNumericType(type) ) return type;
						numericTypeError(type, "-", e->lineno);
					case expKindLogicNot:
						if ( isBool(type) ) return type;
						boolTypeError(type, "!", e->lineno);
					case expKindBitNotUnary:
						if ( isIntegerType(type) ) return type;
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
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "||", e->lineno); //1.2.2 "For two defined types to be identical, they must point to the same type specification
						else if ( isBool(typeLeft) ) return typeLeft; //since we do the equality check, this is safe.
						boolTypeError(typeLeft, "||", e->lineno);
					case expKindLogicAnd:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "&&", e->lineno);
						else if ( isBool(typeLeft) ) return typeLeft;
						boolTypeError(typeLeft, "&&", e->lineno);
					case expKindEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "==", e->lineno);
						else if ( isComparable(typeLeft) ) return BOOL_HOLDER;
						comparableTypeError(typeLeft, "==", e->lineno);
					case expKindNEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "!=", e->lineno);
						else if ( isComparable(typeLeft) ) return BOOL_HOLDER;
						comparableTypeError(typeLeft, "!=", e->lineno);
					case expKindLess:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "<", e->lineno);
						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, "<", e->lineno); 
					case expKindLEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "<=", e->lineno);
						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, "<=", e->lineno); 
					case expKindGreater:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, ">", e->lineno);
						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, ">", e->lineno); 
					case expKindGEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, ">=", e->lineno);
						else if ( isOrdered(typeLeft) ) return BOOL_HOLDER;
						orderedTypeError(typeLeft, ">=", e->lineno); 
					case expKindAddition:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "+", e->lineno);
						else if ( isOrdered(typeLeft) ) return typeLeft; //equivalent to being numeric or string
						fprintf(stderr, "Error: (%d) %s is not a numeric type, nor string, incompatiable with +\n", e->lineno, typeToString(typeLeft));
						exit(1);
					case expKindSubtraction:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "-", e->lineno);
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "-", e->lineno);
					case expKindMultiplication:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "*", e->lineno);
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "*", e->lineno);
					case expKindDivision:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "/", e->lineno);
						else if ( isNumericType(typeLeft) ) return typeLeft;
						numericTypeError(typeLeft, "/", e->lineno);
					case expKindMod:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "%", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "%", e->lineno);
					case expKindBitOr:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "|", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "|", e->lineno);
					case expKindBitAnd:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "&", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "&", e->lineno);
					case expKindBitShiftLeft:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "<<", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "<<", e->lineno);
					case expKindBitShiftRight:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, ">>", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, ">>", e->lineno);
					case expKindBitAndNot:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "&^", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "&^", e->lineno);
					case expKindBitNotBinary:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "^", e->lineno);
						else if ( isIntegerType(typeLeft) ) return typeLeft;
						integerTypeError(typeLeft, "^", e->lineno);
						
				}
			}
			else
			{ //I switch to if statments here so I can declare stuff
				if ( e->kind == expKindFuncCall )
				{
					TTEntry *baseType = typeCheckExpression(e->val.funcCall.base);
					if ( strcmp(e->val.funcCall.base->val.id, "init") == 0 ) //we did yardwork-- this is safe
					{
						fprintf(stderr, "Error: (%d) init(.) may not be called\n", e->lineno);
						exit(1);
					}
					if ( baseType->underlyingType == identifierType ) /**TYPECAST**/
					{
						if ( !(e->val.funcCall.arguments != NULL && e->val.funcCall.arguments->next == NULL ) )
						{
							fprintf(stderr, "Error: (%d) Typecasts need exactly one argument\n", e->lineno);
							exit(1);
						}
						TTEntry *toCast = typeCheckExpression(e->val.funcCall.arguments->cur);
						//TODO check if we can type cast arrays and stuff. Not specified in documentation, but
						if ( toCast->val.nonCompositeType.type == baseType->val.nonCompositeType.type  || (isNumericType(toCast) && isNumericType(baseType)) || (isIntegerType(toCast) && baseType->val.nonCompositeType.type == baseString) )
						{
							return baseType;
						}
						fprintf(stderr, "Error: (%d) Typecasts need to occur with either identical underlying types, numeric types, or an integer type to a string. Received types of %s and %s\n", e->lineno, typeToString(baseType), typeToString(toCast)); 
						exit(1);
					}
					else if ( baseType->underlyingType != funcType ) 
					{
						fprintf(stderr, "Error: (%d) Can't pass arguments to something of type %s\n", e->lineno, typeToString(baseType)); 
						exit(1);
					}
					/**FUNCCALL**/
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
							fprintf(stderr, "Error: (%d) Expected parameter of type %s, received %s\n", e->lineno, typeToString(curArgType->cur), typeToString(curArgPassedType));
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
					if ( !(isNonCompositeType(indexType) && indexType->val.nonCompositeType.type == baseInt) )
					{
						fprintf(stderr, "Error: (%d) The type of the index was %s, expecting underlying type int\n", e->lineno, typeToString(indexType));
						exit(1);
					}
					if ( !(baseType->underlyingType == arrayType || baseType->underlyingType == sliceType) )
					{
						fprintf(stderr, "Error: (%d) Trying to index an un-indexable type %s\n", e->lineno, typeToString(baseType));
						exit(1);
					}
					if ( baseType->underlyingType == arrayType ) return baseType->val.arrayType.type;
					return baseType->val.sliceType.type;

				}
				else if ( e->kind == expKindFieldSelect )
				{
					TTEntry *baseType = typeCheckExpression(e->val.access.base);
					if ( !(baseType->underlyingType == structType) )
					{
						fprintf(stderr, "Error (%d) Field selection requires a base expression with underlying type struct, received %s\n", e->lineno, typeToString(baseType));
						exit(1);
					}
					for ( EntryTupleList *curField = baseType->val.structType.fields; curField != NULL; curField = curField->next ) 
					{
						if ( strcmp(e->val.access.accessor->val.id, curField->id) == 0 ) return curField->type; //safe field access-- ensured to be id in past passes
					}
					fprintf(stderr, "Error: (%d) Struct %s has no field called %s", e->lineno, typeToString(baseType), e->val.access.accessor->val.id);
					exit(1);
				}
				else if ( e->kind == expKindAppend )
				{
					TTEntry *listType = typeCheckExpression(e->val.append.list);
					TTEntry *elemType = typeCheckExpression(e->val.append.elem);
					if ( !listType->underlyingType == sliceType )
					{
						fprintf(stderr, "Error: (%d) Append requires an expression with underlying type slice, received %s\n", e->lineno, typeToString(listType));
						exit(1);
					}
					if ( elemType != listType->val.sliceType.type )
					{
						fprintf(stderr, "Error: (%d) Cannot append elements of type %s to types %s", e->lineno, typeToString(elemType), typeToString(listType));
						exit(1);
					}
					return listType;
				}
				else if ( e->kind == expKindLength )
				{
					TTEntry *bodyType = typeCheckExpression(e->val.builtInBody);
					if ( !(bodyType->underlyingType == sliceType || bodyType->underlyingType == arrayType || (bodyType->underlyingType == identifierType && bodyType->val.nonCompositeType.type == baseString)) )
					{
						fprintf(stderr, "Error: (%d) Length requires an expression with underlying type slice or array or string, received %s\n", e->lineno, typeToString(bodyType));
						exit(1);
					}
					return INT_HOLDER; 
				}
				else if (e->kind == expKindCapacity )
				{
					TTEntry *bodyType = typeCheckExpression(e->val.builtInBody);
					if ( !(bodyType->underlyingType == sliceType || bodyType->underlyingType == arrayType) )
					{
						fprintf(stderr, "Error: (%d) Capacity requires an expression with underlying type slice or array, received %s\n", e->lineno, typeToString(bodyType));
						exit(1);
					}
					return INT_HOLDER; 
				}
			}


	}
}

//TODO
void typeCheckStatement(Stmt* stmt){
	if (stmt == NULL){
		return;
	}

	switch (stmt->kind){
		case StmtKindBlock:
			typeCheckStatement(stmt->val.block.stmt);
			break;

		
		case StmtKindExpression: 
			typeCheckExpression(stmt->val.expression.expr);
			break;

		//TODO, lots todo
		//lvalue assignability checks
		case StmtKindAssignment:
			break;


		//TODO
		case StmtKindPrint:
			break;
		case StmtKindPrintln:
			break;


		case StmtKindIf:
			break;  
		case StmtKindReturn:
			break;
		case StmtKindElse:
			break;
		case StmtKindSwitch:
			break;
		case StmtKindInfLoop:
			break;
		case StmtKindWhileLoop:
			break;
		case StmtKindThreePartLoop:
			break;

		case StmtKindBreak:
			break;
		case StmtKindContinue:
			break;
		case StmtKindFallthrough:
			break;



		//For denali to implement
		case StmtKindTypeDeclaration:
			break;
		case StmtKindVarDeclaration:
			break;
		case StmtKindShortDeclaration:
			break;
	}

	typeCheckStatement(stmt->next);


}


int statementIsProperlyTerminated(Stmt* stmt, char* funcName);


int isLocalBreakPresent(Stmt* stmt){
	if (stmt == NULL){
		return 0;
	}

	int wasFound = 0;

	
	switch (stmt->kind){
		case StmtKindBlock : 
			wasFound =  isLocalBreakPresent(stmt->val.block.stmt);
			break;

		case StmtKindExpression :
			break;
		case StmtKindAssignment:
			break;
	

		case StmtKindPrint:
			break;
		case StmtKindPrintln:
			break;
		case StmtKindIf:
			wasFound = isLocalBreakPresent(stmt->val.ifStmt.block) || isLocalBreakPresent(stmt->val.ifStmt.elseBlock);
			break;

		case StmtKindReturn:
			break;
		case StmtKindElse :
			wasFound = isLocalBreakPresent(stmt->val.elseStmt.block);
			break;

		case StmtKindSwitch :
			break;
		case StmtKindInfLoop :
			break;
		case StmtKindWhileLoop:
			break;
		case StmtKindThreePartLoop:
			break;

		case StmtKindBreak:
			return 1;
			break;
		case StmtKindContinue:
			break;

		case StmtKindTypeDeclaration:
			break;
		case StmtKindVarDeclaration:
			break;
		case StmtKindShortDeclaration:
			break;
	}

	if (wasFound){
		return 1;
	}

	return isLocalBreakPresent(stmt->next);
}

int isDefaultCasePresent(switchCaseClause* clauseList){
	if (clauseList == NULL){
		return 0;
	}

	if (clauseList->expressionList == NULL){
		return 1;
	}

	return isDefaultCasePresent(clauseList->next);
}

int clauseListBreakCheck(switchCaseClause* clauseList,char* functionName){
	if (clauseList == NULL){
		return 0;
	}

	if (isLocalBreakPresent(clauseList->statementList)){
		fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [switch clause cannot break]\n",clauseList->lineno,functionName);
		exit(1);
	}


	return statementIsProperlyTerminated(clauseList->statementList,functionName) && clauseListBreakCheck(clauseList->next,functionName);
}


int weedSwitchStatementClauseList(Stmt* stmt, char* functionName){


	if (isDefaultCasePresent(stmt->val.switchStmt.clauseList)){
		return clauseListBreakCheck(stmt->val.switchStmt.clauseList,functionName);

	}else{
		fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [no default case]\n",stmt->val.switchStmt.clauseList->lineno,functionName);
		exit(1);
	}



}


//Needs to be used as a heler function for a function weeder
// Remember NULL statement has a specific error 
int statementIsProperlyTerminated(Stmt* stmt, char* funcName){
	if (stmt == NULL){
		return 0;
	}


	if (stmt->next != NULL){
		return statementIsProperlyTerminated(stmt->next,funcName);
	}

	switch(stmt->kind)
	{
		case StmtKindReturn :
			return 1;
		case StmtKindBlock :
			return statementIsProperlyTerminated(stmt->val.block.stmt,funcName);
		case StmtKindIf :
			return statementIsProperlyTerminated(stmt->val.ifStmt.elseBlock,funcName) && statementIsProperlyTerminated(stmt->val.ifStmt.block,funcName) ;
		case StmtKindElse :
			return statementIsProperlyTerminated(stmt->val.elseStmt.block,funcName);
		case StmtKindInfLoop : 
			if ( isLocalBreakPresent(stmt->val.infLoop.block) ){
				fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [loop cannot break]\n",stmt->lineno,funcName);
				exit(1);
			}else{
				return 1;
			}
		case StmtKindWhileLoop : 
			if (stmt->val.whileLoop.conditon == NULL){
				fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [loop condition not empty]\n",stmt->lineno,funcName);
				exit(1);
			}
			if ( isLocalBreakPresent(stmt->val.whileLoop.block) ){
				fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [loop cannot break]\n",stmt->lineno,funcName);
				exit(1);
			}else{
				return 1;
			}
		case StmtKindThreePartLoop : 
			if (stmt->val.forLoop.condition == NULL){
				fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [loop condition not empty]\n",stmt->lineno,funcName);
				exit(1);
			}
			if ( isLocalBreakPresent(stmt->val.forLoop.block) ){
				fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [loop cannot break]\n",stmt->lineno,funcName);
				exit(1);
			}else{
				return 1;
			}




		case StmtKindSwitch : 
				return weedSwitchStatementClauseList(stmt,funcName);
				break; //TODO





		default:
			return 0;
		
	}

	
}




int forAll (ExpList* list,int(*predicate)(Exp*) ){
	if (list == NULL){
		return 1;
	}

	if (predicate(list->cur)){
		return forAll(list->next,predicate);
	}else{
		return 0;
	}
}



