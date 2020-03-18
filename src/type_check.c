#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "pretty_printer.h"
#include "globalEnum.h"
#include "symbol_table.h"

extern TTEntry *builtInTypes;
int isExpListPrintable(ExpList* expList);
int isValidAssignStmt(ExpList* left, ExpList* right);
int isExpressionAddressable(Exp* exp);
int isValidOpAssignStmt(Stmt* stmt);

TTEntry *getExpressionType(Exp *e)
{
	if ( e->contextEntry->isSymbol ) return e->contextEntry->entry.s->type;
	return e->contextEntry->entry.t;
}
TTEntry *getBuiltInType(char *id)
{
	TTEntry *cur = builtInTypes;
	while ( cur != NULL )
	{
		if ( strcmp(cur->id, id) == 0 ) return cur;
	}
	return NULL; //unreachable if id typed correctly...
}
int isNonCompositeType(TTEntry *t) { return t != NULL && t->underlyingType == identifierType;}
int isNumericType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseFloat64 || t->val.nonCompositeType.type == baseRune); }
int isIntegerType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseRune); }
int isBool(TTEntry *t) { return t != NULL && isNonCompositeType(t) && t->val.nonCompositeType.type == baseBool; }
int isOrdered(TTEntry *t) { return isNumericType(t) || (isNonCompositeType(t) && t->val.nonCompositeType.type == baseString); }
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

	if (t == NULL){
		strcpy(str,"void");
		return str;
	}


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
		case expKindIntLit: return getBuiltInType("int"); //TODO this is the base base type
		case expKindFloatLit: return getBuiltInType("float64");
		case expKindRuneLit: return getBuiltInType("rune");
		case expKindRawStringLit:
		case expKindInterpretedStringLit: return getBuiltInType("string");
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
						else if ( typeLeft->comparable ) return getBuiltInType("bool");
						comparableTypeError(typeLeft, "==", e->lineno);
					case expKindNEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "!=", e->lineno);
						else if ( typeLeft->comparable ) return getBuiltInType("bool");
						comparableTypeError(typeLeft, "!=", e->lineno);
					case expKindLess:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "<", e->lineno);
						else if ( isOrdered(typeLeft) ) return getBuiltInType("bool");
						orderedTypeError(typeLeft, "<", e->lineno); 
					case expKindLEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, "<=", e->lineno);
						else if ( isOrdered(typeLeft) ) return getBuiltInType("bool");
						orderedTypeError(typeLeft, "<=", e->lineno); 
					case expKindGreater:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, ">", e->lineno);
						else if ( isOrdered(typeLeft) ) return getBuiltInType("bool");
						orderedTypeError(typeLeft, ">", e->lineno); 
					case expKindGEQ:
						if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, ">=", e->lineno);
						else if ( isOrdered(typeLeft) ) return getBuiltInType("bool");
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
					for ( STEntry *curArgSymbolEntry = baseType->val.functionType.args; curArgSymbolEntry; curArgSymbolEntry = curArgSymbolEntry->next )
					{
						if ( curArgPassed == NULL )
						{
							fprintf(stderr, "Error: (%d) %s called with too few arguments\n", e->lineno, baseType->id);
							exit(1);
						}
						TTEntry *curArgPassedType = typeCheckExpression(curArgPassed->cur);
						if ( curArgPassedType != curArgSymbolEntry->type )
						{
							fprintf(stderr, "Error: (%d) Expected parameter of type %s, received %s\n", e->lineno, typeToString(curArgSymbolEntry->type), typeToString(curArgPassedType));
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
					//cant use getEntry because that searches up the stack-- We only want this context.
					PolymorphicEntry *structField = getEntry(baseType->val.structType.fields, e->val.access.accessor->val.id);
					if ( structField == NULL || !structField->isSymbol )
					{
						fprintf(stderr, "Error: (%d) Struct %s has no field called %s", e->lineno, typeToString(baseType), e->val.access.accessor->val.id);
						exit(1);
					}
					return structField->entry.t;
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
					return getBuiltInType("int"); 
				}
				else if (e->kind == expKindCapacity )
				{
					TTEntry *bodyType = typeCheckExpression(e->val.builtInBody);
					if ( !(bodyType->underlyingType == sliceType || bodyType->underlyingType == arrayType) )
					{
						fprintf(stderr, "Error: (%d) Capacity requires an expression with underlying type slice or array, received %s\n", e->lineno, typeToString(bodyType));
						exit(1);
					}
					return getBuiltInType("int"); 
				}
			}


	}
}

int statementTypeEquality(TTEntry* t1, TTEntry* t2){
	if (t1 == NULL && t2 != NULL || t2 != NULL && t1 == NULL){ // I suppose I could use ^, but what would XOR with NULL mean
		return 0;
	}else if (t1 == NULL && t2 == NULL){
		return 1;
	}else{
		return typeEquality(t1,t2);
	}
}

TTEntry* globalReturnType = NULL; //I know this is bad, but it simplifies the code

//TODO
void typeCheckStatement(Stmt* stmt){
	if (stmt == NULL){
		return;
	}

	TTEntry* type;

	switch (stmt->kind){
		case StmtKindBlock:
			typeCheckStatement(stmt->val.block.stmt);
			break;

		
		case StmtKindExpression: 
			typeCheckExpression(stmt->val.expression.expr);
			break;


		case StmtKindAssignment:
			isValidAssignStmt(stmt->val.assignment.lhs,stmt->val.assignment.rhs);
			break;

		case StmtKindPrint:
			isExpListPrintable(stmt->val.print.list);
			break;
		case StmtKindPrintln:
			isExpListPrintable(stmt->val.println.list);
			break;

		case StmtKindIf:
			typeCheckStatement(stmt->val.ifStmt.statement);
			type = typeCheckExpression(stmt->val.ifStmt.expression);
			if (!isBool(type)){
				fprintf(stderr,"Error : (line %d) conditon expected bool [received %s]\n",stmt->val.ifStmt.expression->lineno,typeToString(type));
			}
			typeCheckStatement(stmt->val.ifStmt.block);
			typeCheckStatement(stmt->val.ifStmt.elseBlock);
			
			break;  
		case StmtKindReturn:
			if (stmt->val.returnVal.returnVal == NULL){
				if (!statementTypeEquality(NULL,globalReturnType)){
					fprintf(stderr,"Error: (lune %d) return expected %s but received %s\n",stmt->lineno,typeToString(globalReturnType),typeToString(NULL));
					exit(1);
				}
			}else{
				TTEntry* type = typeCheckExpression(stmt->val.returnVal.returnVal);
				if (!statementTypeEquality(type,globalReturnType)){
					fprintf(stderr,"Error: (lune %d) return expected %s but received %s",stmt->lineno,typeToString(globalReturnType),typeToString(type));
					exit(1);
				}

			}
			
			
			break;
		case StmtKindElse:
			typeCheckStatement(stmt->val.elseStmt.block);
			break;
		case StmtKindSwitch: // A lot... sigh
			break;
		case StmtKindInfLoop:
			typeCheckStatement(stmt->val.infLoop.block);
			break;
		case StmtKindWhileLoop:
			type = typeCheckExpression(stmt->val.whileLoop.conditon);
			if (!isBool(type)){
				fprintf(stderr,"Error : (line %d) conditon expected bool [received %s]\n",stmt->lineno,typeToString(type));
				exit(1);
			}
			typeCheckStatement(stmt->val.whileLoop.block);
			break;
		case StmtKindThreePartLoop:
			typeCheckStatement(stmt->val.forLoop.init);
			if (stmt->val.forLoop.condition != NULL){
				type = typeCheckExpression(stmt->val.forLoop.condition);
				if (!isBool(type)){
					fprintf(stderr,"Error : (line %d) conditon expected bool [received %s]\n",stmt->lineno,typeToString(type));
					exit(1);
				}
			}
			typeCheckStatement(stmt->val.forLoop.inc);
			typeCheckStatement(stmt->val.forLoop.block);
			break;

		//Trivially typecheck
		case StmtKindBreak:
			break;
		case StmtKindContinue:
			break;


		//Remember that addressable is a subset of assignable
		case StmtKindInc:
			type = typeCheckExpression(stmt->val.incStmt.exp);
			if (!isNumericType(type)){
				fprintf(stderr,"Error: (line %d) expected numeric type but received %s\n",stmt->lineno,typeToString(type));
				exit(1);
			}

			if (!isExpressionAddressable(stmt->val.incStmt.exp)){
				fprintf(stderr,"Error: (line %d) ",stmt->lineno);
				prettyExp(stmt->val.incStmt.exp);
				printf(" is not addressable\n");
				exit(1);

			}
			break;
		case StmtKindDec:
			type = typeCheckExpression(stmt->val.decStmt.exp);
			if (!isNumericType(type)){
				fprintf(stderr,"Error: (line %d) expected numeric type but received %s\n",stmt->lineno,typeToString(type));
				exit(1);
			}

			if (!isExpressionAddressable(stmt->val.decStmt.exp)){
				fprintf(stderr,"Error: (line %d) ",stmt->lineno);
				prettyExp(stmt->val.decStmt.exp);
				printf(" is not addressable\n");
				exit(1);

			}
			break;
		case StmtKindOpAssignment:
			isValidOpAssignStmt(stmt);
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


void typeCheckProgram(RootNode rootNode) {
	
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

		case StmtKindInc:
			break;
		case StmtKindDec:
			break;
		case StmtKindOpAssignment:
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


	if (isDefaultCasePresent(stmt->val.switchStmt.clauseList,functionName)){
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





int isPrintable(Exp* exp){
	if (exp == NULL){
		return 1;
	}

	TTEntry* type = typeCheckExpression(exp);

	if (type == NULL){
		puts("Oh no");
		exit(1);
	}
	
	if (!isNonCompositeType(type)){
		return 0;
	}

	if( type->val.nonCompositeType.type == baseBool 
			|| type->val.nonCompositeType.type == baseInt
			|| type->val.nonCompositeType.type == baseFloat64 
			|| type->val.nonCompositeType.type == baseRune 
			|| type->val.nonCompositeType.type == baseString){

		return 1;	
	}else{
		fprintf(stderr,"Error: (line %d) print expects base types [received %s]\n",exp->lineno,typeToString(type));
		exit(1);
	}


}


int isExpListPrintable(ExpList* expList){
	if (expList == NULL){
		return 1;
	}

	if (isPrintable(expList->cur)){
		return isExpListPrintable(expList->next);
	}
}

//TODO
int isExpressionAddressable(Exp* exp){
	if (exp == NULL){
		puts("Oh no");
		exit(1);
	}

	switch(exp->kind){
		case expKindIdentifier:
			if (isBlank(exp)){
				return 1;
			}

			PolymorphicEntry* polyEntry  = exp->contextEntry;
			if (polyEntry->isSymbol){
				return 1;//VERY MUCH INCOMPLETE
			}else{
				return 0; //If its not a symbol, its a type
			}


		case expKindFieldSelect:
			return 1;
		case expKindIndexing:
			return 1;
		default:
			return 0;
	}


}

//TODO
int isExpressionAssignable(Exp* exp){
	return 0;
}

int isValidAssignPair(Exp* left,Exp* right){
	if (isBlank(left)){
		TTEntry* rightType = typeCheckExpression(right);
		if (!isExpressionAssignable(right)){
			fprintf(stderr,"Error: (line %d) ",right->lineno);
			prettyExp(right);
			printf(" cannot be assigned to ");
			prettyExp(left);
			printf("\n");

			exit(1);
		}
		//? function types

	}else{
		TTEntry* leftType = typeCheckExpression(left);
		if (!isExpressionAddressable(left)){
			fprintf(stderr,"Error: (line %d) cannot assign to ",left->lineno);
			prettyExp(left);
			printf("\n");
			exit(1);
			
		}
		TTEntry* rightType = typeCheckExpression(right);

		if (!isExpressionAssignable(right)){
			fprintf(stderr,"Error: (line %d) ",right->lineno);
			prettyExp(right);
			printf(" cannot be assigned to ");
			prettyExp(left);
			printf("\n");

			exit(1);
		}

		if (!statementTypeEquality(leftType,rightType)){
			fprintf(stderr,"Error: (line %d) %s cannot be assigned to %s\n",left->lineno,typeToString(rightType),typeToString(leftType));
			exit(1);
		}



	}

	return 1;

}

int isValidAssignStmt(ExpList* left, ExpList* right){
	if (left == NULL || right == NULL){
		return 1;
	}

	if (isValidAssignPair(left->cur,right->cur)){
		isValidAssignStmt(left->next,right->next);
	}
}


int isValidOpAssignStmt(Stmt* stmt){
	if (stmt == NULL){
		return 0;
	}

	//I essentially rewrite the statement so that I can recycle functions
	ExpList* leftList = createArgumentList(stmt->val.opAssignment.lhs);
	Exp* bin = makeExpBinary(stmt->val.opAssignment.lhs,stmt->val.opAssignment.rhs,stmt->val.opAssignment.kind);\
	bin->lineno = stmt->lineno;
	ExpList* rightList = createArgumentList(bin);
	Stmt* temp = makeAssignmentStmt(leftList,rightList);
	temp->lineno = stmt->lineno;

	typeCheckStatement(temp);

	free(temp);
	free(leftList);
	free(rightList);
	free(bin);

	return 1;

}


