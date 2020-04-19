#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "pretty_printer.h"
#include "globalEnum.h"
#include "symbol_table.h"




extern Trie* trie;

extern TTEntry *builtInTypes;
int isExpListPrintable(ExpList* expList);
int isValidAssignStmt(ExpList* left, ExpList* right);
int isExpressionAddressable(Exp* exp);
int isValidOpAssignStmt(Stmt* stmt);
void typecheckSwitchStatements(Stmt* stmt);
void functionWeeder(FuncDeclNode* function);
void typeCheckVarDecl(VarDeclNode* decl);

char * expKindToString(ExpressionKind e)
{
	char *ret = (char *) malloc(sizeof(char));
	switch (e)
	{
		case expKindAddition:
			strcpy(ret, "+");
			return ret;
		case expKindSubtraction:
			strcpy(ret, "-");
			return ret;
		case expKindMultiplication:
			strcpy(ret, "*");
			return ret;
		case expKindDivision:
			strcpy(ret, "/");
			return ret;
		case expKindMod:
			strcpy(ret, "%");
			return ret;
		case expKindBitAnd:
			strcpy(ret, "&");
			return ret;
		case expKindBitOr:
			strcpy(ret, "|");
			return ret;
		case expKindBitNotUnary:
			strcpy(ret, "^");
			return ret;
		case expKindBitNotBinary:
			strcpy(ret, "^");
			return ret;
		case expKindBitShiftRight:
			strcpy(ret, ">>");
			return ret;
		case expKindBitAndNot:
			strcpy(ret, "&^");
			return ret;
		case expKindLogicAnd:
			strcpy(ret, "&&");
			return ret;
		case expKindLogicOr:
			strcpy(ret, "||");
			return ret;
		case expKindEQ:
			strcpy(ret, "==");
			return ret;
		case expKindGreater:
			strcpy(ret, ">");
			return ret;
		case expKindLess:
			strcpy(ret, "<");
			return ret;
		case expKindLogicNot:
			strcpy(ret, "!");
			return ret;
		case expKindNEQ:
			strcpy(ret, "!=");
			return ret;
		case expKindGEQ:
			strcpy(ret, ">=");
			return ret;
		case expKindUnaryPlus:
			strcpy(ret, "+");
			return ret;
		case expKindUnaryMinus:
			strcpy(ret, "-");
			return ret;
		case expKindFuncCall:
			strcpy(ret, "calls");
			return ret;
		case expKindIndexing:
			strcpy(ret, "indexing");
			return ret;
		case expKindFieldSelect:
			strcpy(ret, "field selection");
			return ret;
		case expKindAppend:
			strcpy(ret, "appends");
			return ret;
		case expKindLength:
			strcpy(ret, "length");
			return ret;
		case expKindCapacity:
			strcpy(ret, "capacity");
			return ret;
		default:
			fprintf(stderr, "This method was misused or we forgot a case :( (PRINTING TYPES)\n");
			exit(2);
	}
}
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
		//printf("%s\n", cur -> id);
		cur = cur -> next;
	}
	printf("looked for a built-in (%s), couldn't find it. You should never see this.\n", id);
	return NULL; //unreachable if id typed correctly...
}
int isNonCompositeType(TTEntry *t) { return t != NULL && t->underlyingType == identifierType;}
int isNumericType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseFloat64 || t->val.nonCompositeType.type == baseRune); }
int isIntegerType(TTEntry *t) { return t != NULL && isNonCompositeType(t) && (t->val.nonCompositeType.type == baseInt || t->val.nonCompositeType.type == baseRune); }
int isBool(TTEntry *t) { return t != NULL && isNonCompositeType(t) && t->val.nonCompositeType.type == baseBool; }
int isOrdered(TTEntry *t) { return isNumericType(t) || (isNonCompositeType(t) && t->val.nonCompositeType.type == baseString); }
int typeEquality(TTEntry *t1, TTEntry *t2)
{
	if ( t1 == NULL || t2 == NULL ) return t1 == t2;
	
	if ( t1->id == NULL && t2->id == NULL )
	{
	if ( t1->underlyingType == arrayType && t2->underlyingType == arrayType )
		{
			return typeEquality(t1->val.arrayType.type, t2->val.arrayType.type) && t1->val.arrayType.size == t2->val.arrayType.size;
		}
		else if ( t1->underlyingType == sliceType && t2->underlyingType == sliceType )
		{
			return typeEquality(t1->val.sliceType.type, t2->val.sliceType.type);
		}
		else if ( t1->underlyingType == structType && t2->underlyingType == structType )
		{
		//iterate through entries and call type equality on the fields. return false if any are false, true if all are true
			IdChain* iter1 = t1 -> val.structType.fieldNames;
			IdChain* iter2 = t2 -> val.structType.fieldNames;
			PolymorphicEntry* hold1;
			PolymorphicEntry* hold2;
			while (iter1 != NULL) {
				if (iter2 == NULL) {
					return 0;
				}
				if (strcmp(iter1 -> identifier, iter2 -> identifier) != 0) {
					return 0;
				}
				hold1 = getEntry(t1 -> val.structType.fields, iter1 -> identifier);
				hold2 = getEntry(t2 -> val.structType.fields, iter2 -> identifier);
				if (typeEquality(hold1 -> entry.s -> type, hold2 -> entry.s -> type) == 0) {
					return 0;
				}
				iter1 = iter1 -> next;
				iter2 = iter2 -> next;
			}
			if (iter2 != NULL) {
				return 0;
			}
			return 1;
		}
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

	if (t -> id == NULL) {
		if ( t->underlyingType == arrayType )
		{
			sprintf(str, "[%d]%s", t->val.arrayType.size, typeToString(t->val.arrayType.type));
		}
		else if ( t->underlyingType == sliceType )
		{
			sprintf(str, "[]%s", typeToString(t->val.sliceType.type));
		}
		else if ( t->underlyingType == structType )
		{
			sprintf(str, "struct { ");
			//iterate through entries and call print on the fields.
			IdChain* iter = t -> val.structType.fieldNames;
			PolymorphicEntry* hold;
			while (iter != NULL) {
				strcat(str, iter -> identifier);
				strcat(str, " ");
				strcat(str, typeToString(getEntry(t -> val.structType.fields, iter -> identifier) -> entry.s -> type));
				strcat(str, "; ");
				iter = iter -> next;
			}
			strcat(str, "}");
		}
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
void numericTypeError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a numeric type, incompatiable with %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void integerTypeError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a integer type, incompatiable with %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void boolTypeError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not a bool type, incompatiable with %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void comparableTypeError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not comparable, incompatiable with %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void orderedTypeError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is not ordered, incompatiable with %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void notExpressionError(TTEntry *t, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) %s is a type, cannot take part in %s\n", lineno, typeToString(t), expKindToString(e));
	exit(1);
}
void notMatchingTypes(TTEntry *t1, TTEntry *t2, ExpressionKind e, int lineno)
{
	fprintf(stderr, "Error: (%d) Operation %s requires equal types for both arguments (got %s and %s)\n", lineno, expKindToString(e), typeToString(t1), typeToString(t2));
	exit(1);
}
PolymorphicEntry *makeTypeEntry(TTEntry *t)
{
	PolymorphicEntry *p = (PolymorphicEntry *) malloc(sizeof(PolymorphicEntry));
	p->isSymbol = 0;
	p->entry.t = t;
	return p; 
}
TTEntry *_typeCheckExpression(Exp *e, int *wasType) //Note: this rejects any expressions with types as identifiers. I can't think of any instances where allowing them is desirable...
{
	TTEntry *typeLeft;
	TTEntry *typeRight;
	TTEntry *type;
	Exp *unaryExp;
	Exp *leftExp;
	Exp *rightExp;
	switch (e->kind)
	{ 
		case expKindIntLit: e->contextEntry = makeTypeEntry(getBuiltInType("int")); return getExpressionType(e);
		case expKindFloatLit: e->contextEntry = makeTypeEntry(getBuiltInType("float64")); return getExpressionType(e);
		case expKindRuneLit: e->contextEntry = makeTypeEntry(getBuiltInType("rune")); return getExpressionType(e);
		case expKindRawStringLit:
		case expKindInterpretedStringLit: e->contextEntry = makeTypeEntry(getBuiltInType("string")); return getExpressionType(e);
		case expKindIdentifier:
			if ( !e->contextEntry->isSymbol ) *wasType = 1;
			return getExpressionType(e);
		default:
			if ( isUnary(e) )
			{
				unaryExp = e->val.unary;
				type = _typeCheckExpression(unaryExp, wasType);
				e->contextEntry = makeTypeEntry(type);
				if ( *wasType ) notExpressionError(type, e->kind, e->lineno); 
				switch (e->kind)
				{
					case expKindUnaryPlus:
					case expKindUnaryMinus:
						if ( isNumericType(type) ) return type;
						numericTypeError(type, e->kind, e->lineno);
					case expKindLogicNot:
						if ( isBool(type) ) return type;
						boolTypeError(type, e->kind, e->lineno);
					case expKindBitNotUnary:
						if ( isIntegerType(type) ) return type;
						integerTypeError(type, e->kind, e->lineno);
				}
			}
			else if ( isBinary(e) )
			{
				leftExp = e->val.binary.left;
				rightExp = e->val.binary.right;

				typeLeft = _typeCheckExpression(leftExp, wasType);
				if ( *wasType ) notExpressionError(typeLeft, e->val.binary.left->kind, e->lineno); 
				typeRight = _typeCheckExpression(rightExp, wasType);
				if ( *wasType ) notExpressionError(typeRight, e->val.binary.right->kind, e->lineno); 

				if ( !typeEquality(typeLeft, typeRight) ) notMatchingTypes(typeLeft, typeRight, e->kind, e->lineno); 

				switch (e->kind)
				{
					case expKindLogicOr:
					case expKindLogicAnd:
						if ( isBool(typeLeft) ) {
							e->contextEntry = makeTypeEntry(typeLeft);
							return typeLeft;
						}
						boolTypeError(typeLeft, e->kind, e->lineno);
					case expKindEQ:
					case expKindNEQ:
						if ( typeLeft->comparable ) {
							e->contextEntry = makeTypeEntry(getBuiltInType("bool"));
							return getBuiltInType("bool");
						}
						comparableTypeError(typeLeft, e->kind, e->lineno);
					case expKindLess:
					case expKindLEQ:
					case expKindGreater:
					case expKindGEQ:
						if ( isOrdered(typeLeft) ) 
						{
							e->contextEntry = makeTypeEntry(getBuiltInType("bool"));
							return getBuiltInType("bool");
						}
						orderedTypeError(typeLeft, e->kind, e->lineno); 
					case expKindAddition:
						if ( isOrdered(typeLeft) ) 
						{
							e->contextEntry = makeTypeEntry(typeLeft);
							return typeLeft; //equivalent to being numeric or string
						}
						fprintf(stderr, "Error: (%d) %s is not a numeric type, nor string, incompatiable with +\n", e->lineno, typeToString(typeLeft));
						exit(1);
					case expKindSubtraction:
					case expKindMultiplication:
					case expKindDivision:
						if ( isNumericType(typeLeft) ) {
							e->contextEntry = makeTypeEntry(typeLeft);
							return typeLeft;
						}
						numericTypeError(typeLeft, e->kind, e->lineno);
					case expKindMod:
					case expKindBitOr:
					case expKindBitAnd:
					case expKindBitShiftLeft:
					case expKindBitShiftRight:
					case expKindBitAndNot:
					case expKindBitNotBinary:
						if ( isIntegerType(typeLeft) ) {
							e->contextEntry = makeTypeEntry(typeLeft);
							return typeLeft;
						}
						integerTypeError(typeLeft, e->kind, e->lineno);
						
				}
			}
			else
			{ //I switch to if statments here so I can declare stuff
				if ( e->kind == expKindFuncCall )
				{
					TTEntry *baseType = _typeCheckExpression(e->val.funcCall.base, wasType);
					if ( baseType->underlyingType == funcType ) /**FUNCCALL**/
					{
						ExpList * curArgPassed = e->val.funcCall.arguments;
						int paramCount = 1;
						for ( STEntry *curArgSymbolEntry = baseType->val.functionType.args; curArgSymbolEntry; curArgSymbolEntry = curArgSymbolEntry->next, paramCount++ )
						{
							if ( curArgPassed == NULL )
							{
								fprintf(stderr, "Error: (%d) %s called with too few arguments\n", e->lineno, e->val.funcCall.base->val.id);
								exit(1);
							}
							TTEntry *curArgPassedType = _typeCheckExpression(curArgPassed->cur, wasType);
							if ( *wasType ) notExpressionError(curArgPassedType, e->kind, e->lineno);
							if ( !typeEquality(curArgPassedType, curArgSymbolEntry->type) )
							{
								fprintf(stderr, "Error: (%d) The %d argument of %s expects parameter of type %s, received %s\n", e->lineno, paramCount, e->val.funcCall.base->val.id, typeToString(curArgSymbolEntry->type), typeToString(curArgPassedType));
								exit(1);
							}
							curArgPassed = curArgPassed->next;
						}
						if ( curArgPassed != NULL )
						{
							fprintf(stderr, "Error: (%d) %s called with too many arguments\n", e->lineno, e->val.funcCall.base->val.id);
							exit(1);
						}
						e->contextEntry = makeTypeEntry(baseType->val.functionType.ret);
						return baseType->val.functionType.ret;
					}
					/**TYPECAST**/
					else if ( *wasType )
					{
						if ( !(e->val.funcCall.arguments != NULL && e->val.funcCall.arguments->next == NULL ) )
						{
							fprintf(stderr, "Error: (%d) Typecasts need exactly one argument\n", e->lineno);
							exit(1);
						}
						*wasType = 0;
						TTEntry *toCast = _typeCheckExpression(e->val.funcCall.arguments->cur, wasType);
						//TODO check if we can type cast arrays and stuff. Not specified in documentation, but
						if ( *wasType ) notExpressionError(toCast, e->kind, e->lineno);
						if ( toCast->val.nonCompositeType.type == baseType->val.nonCompositeType.type  || (isNumericType(toCast) && isNumericType(baseType)) || (isIntegerType(toCast) && baseType->val.nonCompositeType.type == baseString) )
						{
							e->contextEntry = makeTypeEntry(baseType);
							return baseType;
						}
						fprintf(stderr, "Error: (%d) Typecasts need to occur with either identical underlying types, numeric types, or an integer type to a string. Received types of %s and %s\n", e->lineno, typeToString(baseType), typeToString(toCast)); 
						exit(1);
					}
					fprintf(stderr, "Error: (%d) The expression does not refer to a function nor type\n", e->lineno); 
					exit(1);
				}
				else if ( e->kind == expKindIndexing )
				{
					TTEntry * indexType = _typeCheckExpression(e->val.access.accessor, wasType);
					if ( *wasType ) notExpressionError(indexType, e->kind, e->lineno);
					TTEntry * baseType = _typeCheckExpression(e->val.access.base, wasType);
					if ( *wasType ) notExpressionError(baseType, e->kind, e->lineno);
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
					if ( baseType->underlyingType == arrayType ) {
						e->contextEntry = makeTypeEntry(baseType->val.arrayType.type);
						return baseType->val.arrayType.type;
					}
					e->contextEntry = makeTypeEntry(baseType->val.arrayType.type);
					return baseType->val.sliceType.type;

				}
				else if ( e->kind == expKindFieldSelect )
				{
					TTEntry *baseType = _typeCheckExpression(e->val.access.base, wasType);
					if ( *wasType ) notExpressionError(baseType, e->kind, e->lineno);
					if ( !(baseType->underlyingType == structType) )
					{
						fprintf(stderr, "Error (%d) Field selection requires a base expression with underlying type struct, received %s\n", e->lineno, typeToString(baseType));
						exit(1);
					}
					PolymorphicEntry *structField = getEntry(baseType->val.structType.fields, e->val.access.accessor->val.id);
					if ( structField == NULL || !structField->isSymbol )
					{
						fprintf(stderr, "Error: (%d) Struct %s has no field called %s\n", e->lineno, typeToString(baseType), e->val.access.accessor->val.id);
						exit(1);
					}
					e->contextEntry = makeTypeEntry(structField->entry.s->type);
					return structField->entry.s->type;
				}
				else if ( e->kind == expKindAppend )
				{
					TTEntry *listType = _typeCheckExpression(e->val.append.list, wasType);
					if ( *wasType ) notExpressionError(listType, e->kind, e->lineno);
					TTEntry *elemType = _typeCheckExpression(e->val.append.elem, wasType);
					if ( *wasType ) notExpressionError(elemType, e->kind, e->lineno);
					if ( !(listType->underlyingType == sliceType) )
					{
						fprintf(stderr, "Error: (%d) Append requires an expression with underlying type slice, received %s\n", e->lineno, typeToString(listType));
						exit(1);
					}
					if ( !typeEquality(elemType, listType->val.sliceType.type) )
					{
						fprintf(stderr, "Error: (%d) Cannot append elements of type %s to types %s\n", e->lineno, typeToString(elemType), typeToString(listType));
						exit(1);
					}
					e->contextEntry = makeTypeEntry(listType);
					return listType;
				}
				else if ( e->kind == expKindLength )
				{
					TTEntry *bodyType = _typeCheckExpression(e->val.builtInBody, wasType);
					if ( *wasType ) notExpressionError(bodyType, e->kind, e->lineno);
					if ( !(bodyType->underlyingType == sliceType || bodyType->underlyingType == arrayType || (bodyType->underlyingType == identifierType && bodyType->val.nonCompositeType.type == baseString)) )
					{
						fprintf(stderr, "Error: (%d) Length requires an expression with underlying type slice or array or string, received %s\n", e->lineno, typeToString(bodyType));
						exit(1);
					}
					e->contextEntry = makeTypeEntry(getBuiltInType("int"));
					return getBuiltInType("int"); 
				}
				else if (e->kind == expKindCapacity )
				{
					TTEntry *bodyType = _typeCheckExpression(e->val.builtInBody, wasType);
					if ( *wasType ) notExpressionError(bodyType, e->kind, e->lineno);
					if ( !(bodyType->underlyingType == sliceType || bodyType->underlyingType == arrayType) )
					{
						fprintf(stderr, "Error: (%d) Capacity requires an expression with underlying type slice or array, received %s\n", e->lineno, typeToString(bodyType));
						exit(1);
					}
					e->contextEntry = makeTypeEntry(getBuiltInType("int"));
					return getBuiltInType("int"); 
				}
			}


	}
}

TTEntry *typeCheckExpression(Exp *e)
{
	int *wasType = (int *) malloc(sizeof(int));
	*wasType = 0;
	return _typeCheckExpression(e, wasType);
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
				exit(1);
			}
			typeCheckStatement(stmt->val.ifStmt.block);
			typeCheckStatement(stmt->val.ifStmt.elseBlock);
			
			break;  
		case StmtKindReturn:
			if (stmt->val.returnVal.returnVal == NULL){
				if (!statementTypeEquality(NULL,globalReturnType)){
					fprintf(stderr,"Error: (line %d) return expected %s but received %s\n",stmt->lineno,typeToString(globalReturnType),typeToString(NULL));
					exit(1);
				}
			}else{
				TTEntry* type = typeCheckExpression(stmt->val.returnVal.returnVal);
				if (!statementTypeEquality(type,globalReturnType)){
					fprintf(stderr,"Error: (line %d) return expected %s but received %s\n",stmt->lineno,typeToString(globalReturnType),typeToString(type));
					exit(1);
				}

			}
			
			
			break;
		case StmtKindElse:
			typeCheckStatement(stmt->val.elseStmt.block);
			break;
		case StmtKindSwitch:
			typecheckSwitchStatements(stmt);
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

		//TODO
		//For denali to implement
		case StmtKindTypeDeclaration:
			
			break;
		case StmtKindVarDeclaration:
			typeCheckVarDecl(stmt -> val.varDeclaration);
			break;
		case StmtKindShortDeclaration:
			typeCheckVarDecl(stmt -> val.varDeclaration);
			break;
	}

	typeCheckStatement(stmt->next);


}


void typeCheckProgram(RootNode* rootNode) {
	TopDeclarationNode* topIter = rootNode -> startDecls;
	while (topIter != NULL){
		if (topIter -> declType == variDeclType) {
			typeCheckVarDecl(topIter -> actualRealDeclaration.varDecl);
		} else if (topIter -> declType == typeDeclType) {
			//Do nothing?
			//printf("types are type-correct;\n");
		} else if (topIter -> declType == funcDeclType) {
			functionWeeder(topIter -> actualRealDeclaration.funcDecl);
			typeCheckStatement(topIter -> actualRealDeclaration.funcDecl -> blockStart);
		} else {
			printf("How did I get here?\n");
		}
		
		topIter = topIter -> nextTopDecl;
	}

	trie = encodeRoot(rootNode);
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



	if (statementIsProperlyTerminated(clauseList->statementList,functionName) ){
		if (clauseList->next == NULL){
			return 1;
		}else{
			return clauseListBreakCheck(clauseList->next,functionName);
		}
	}
	
}


int weedSwitchStatementClauseList(Stmt* stmt, char* functionName){
	
	if (stmt == NULL){
		return 1;
	}

	if (isDefaultCasePresent(stmt->val.switchStmt.clauseList)){
		return clauseListBreakCheck(stmt->val.switchStmt.clauseList,functionName);

	}else{
		fprintf(stderr,"Error: line (%d) function %s does not have a terminating statement [no default case]\n",stmt->lineno,functionName);
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
			if (stmt->val.whileLoop.conditon != NULL){
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
			if (stmt->val.forLoop.condition != NULL){
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
				break; 





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
		fprintf(stderr,"Error: (line %d) print, println expect base types [received %s]\n",exp->lineno,typeToString(type));
		exit(1);
	}

	if( type->val.nonCompositeType.type == baseBool 
			|| type->val.nonCompositeType.type == baseInt
			|| type->val.nonCompositeType.type == baseFloat64 
			|| type->val.nonCompositeType.type == baseRune 
			|| type->val.nonCompositeType.type == baseString){

		return 1;	
	}else{
		fprintf(stderr,"Error: (line %d) print, println expect base types [received %s]\n",exp->lineno,typeToString(type));
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
			if (polyEntry->isSymbol && (polyEntry->entry.s->isConstant == 0)  ){ //If a symbol and not a $ constant or a functions
				return 1;
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

int isExpressionConst(Exp* exp){
	if (exp == NULL){
		return 0;
	}

	PolymorphicEntry* polyEntry  = exp->contextEntry;
	return polyEntry->isSymbol && (polyEntry->entry.s->isConstant == 1) ;
	

}

int isExpressionAssignable(Exp* exp){
	if (exp == NULL){
		return 0;
	}


	switch (exp->kind){
		case expKindIdentifier : 
			return isExpressionAddressable(exp) || isExpressionConst(exp);
		default: 
			return 1;
	}
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

void typeCheckExpList(ExpList* list,TTEntry* type){
	if (list == NULL){
		return;
	}

	TTEntry* temp = typeCheckExpression(list->cur);
	if(!statementTypeEquality(temp,type)){
		fprintf(stderr,"Error: (line %d) switch clause conditon expected type %s, but received type %s\n",list->cur->lineno,typeToString(type),typeToString(temp));
		exit(1);
	}
	typeCheckExpList(list->next,type);
}


void typecheckSwitchCaseClause(switchCaseClause* clauseList, TTEntry* type){
	if (clauseList == NULL){
		return;
	}

	typeCheckExpList(clauseList->expressionList,type);
	typeCheckStatement(clauseList->statementList);

	typecheckSwitchCaseClause(clauseList->next,type);



}

void typecheckSwitchStatements(Stmt* stmt){
	if(stmt == NULL){
		return;
	}

	typeCheckStatement(stmt->val.switchStmt.statement);

	TTEntry* type;
	if (stmt->val.switchStmt.expression == NULL){ //Empty expression corresponds to boolean
		// I'm trying to summon the boolean type entry
		Exp* exp = makeExpIntLit(0);
		Exp* exp1 = makeExpBinary(exp,exp,expKindEQ);
		stmt->val.switchStmt.expression = exp1;
		type = typeCheckExpression(exp1);
		

	}else{
		type = typeCheckExpression(stmt->val.switchStmt.expression);
		if (!type->comparable){
			fprintf(stderr,"Error: (line %d) switch case expression must be of a comparable type\n",stmt->lineno);
			exit(1);
		}
	}

	typecheckSwitchCaseClause(stmt->val.switchStmt.clauseList,type);
}


void functionWeeder(FuncDeclNode* function){
	if (function == NULL){
		return;
	}
	
	if (strcmp(function -> identifier, "init") == 0) {
		globalReturnType = NULL;
		return;
	}

	TTEntry* returnType = function->symbolEntry->type->val.functionType.ret;
	globalReturnType = returnType;
	
	if (returnType  == NULL){ //Weeder skipped if return type is NULL(void)
		return;
	}

	if (!statementIsProperlyTerminated(function->blockStart,function->identifier)){
		fprintf(stderr,"Error: (line %d) function %s does not end in a terminating statement\n",function->lineno,function->identifier);
		exit(1);
	}


}

void typeCheckVarDecl(VarDeclNode* decl) {
	
	
	if (decl == NULL) {
		return;
	}
	
	if (decl -> value == NULL){
		return;
	}
	TTEntry* expType = typeCheckExpression(decl -> value);
	if (!isExpressionAssignable(decl->value)){
		fprintf(stderr,"Error: (line %d) type %s cannot be used as a value in a short declaration\n",decl->lineno,typeToString(expType));
		exit(1);
	}
	if (decl -> typeThing -> kind == inferType) {
		if (decl -> iDoDeclare == 1) {
			decl -> whoAmI -> type = expType;
		} else {
			if (!typeEquality(decl -> whoAmI -> type, expType)) {
				
				fprintf(stderr,"Error: (line %d) %s cannot be assigned to %s\n", decl -> lineno, typeToString(expType), typeToString(decl -> whoAmI -> type));
				exit(1);
			}
		}
	} else {
		if (!typeEquality(expType, decl -> whoAmI -> type)) {
			fprintf(stderr,"Error: (line %d) %s cannot be assigned to %s\n", decl -> lineno, typeToString(expType), typeToString(decl -> whoAmI -> type));
			exit(1);
		}
	}
	
	
	
	typeCheckVarDecl(decl -> nextDecl);
	
	
	typeCheckVarDecl(decl -> multiDecl);
	
}







int structVariantCounter = 0;



Entry* makeEntry(char* id, TTEntry* type){
	Entry* ptr = malloc(sizeof(Entry));

	ptr->id = id;
	ptr->type = type;

	return ptr;
}

Trie* makeTrie(TrieType genre){
	Trie* ptr = malloc(sizeof(Trie));

	ptr->genre = genre;
	return ptr;
}


Trie* findSibling(Trie* trie,TTEntry* structEntry, char* id){
	if (trie == NULL){
		return NULL;
	}

	if (trie->genre == LabelNode){
		return findSibling(trie->sibling,structEntry,id);
	}else{
		if (strcmp(id,trie->variant.entry->id) == 0){
			TTEntry* queriedType = getEntry(structEntry->val.structType.fields,id)->entry.s->type;
			if (typeEquality(queriedType,trie->variant.entry->type)){
				return trie;
			}else{
				return findSibling(trie->sibling,structEntry,id);
			}
		}else{
			return findSibling(trie->sibling,structEntry,id);
		}
	}

}

Trie* makeLabelNode(){
	Trie* ptr = makeTrie(LabelNode);
	ptr->variant.label = structVariantCounter++;
	return ptr;
}

Trie* helperEncodeInfo(Trie* trie,TTEntry* structEntry, IdChain* chain){
	if (chain == NULL){
		Trie* cur = trie;
		while(cur != NULL){
			if (cur->genre == LabelNode){
				return trie;//Already in the trie
			}
			cur = cur->sibling;
		}
		Trie* temp = makeLabelNode();
		temp->sibling = trie;

		return temp;
		
	}

	Trie* sibling = findSibling(trie,structEntry,chain->identifier);
	if (sibling == NULL){ 
		
			Trie* ptr = makeTrie(EntryNode);
			ptr->variant.entry->type = getEntry(structEntry->val.structType.fields,chain->identifier)->entry.s->type;
			ptr->variant.entry->id = chain->identifier;

			ptr->sibling = trie;
			return ptr;
		
	}else{
		sibling->child = helperEncodeInfo(sibling->child,structEntry,chain->next);
		return trie;

	}

}

Trie* encodeInfo(Trie* trie, TTEntry* structEntry){
	return helperEncodeInfo(trie,structEntry,structEntry->val.structType.fieldNames);
}

int helperLookUpLabel(Trie* trie,TTEntry* structEntry,IdChain* chain){
	if (chain == NULL){
		Trie* cur = trie;
		while(cur != NULL){
			if (cur->genre == LabelNode){
				return cur->variant.label;
			}

			cur = cur->sibling;
		}

		return -1; //NOT FOUND
	}


	Trie* sibling = findSibling(trie,structEntry,chain->identifier);
	if (sibling == NULL){
		return -1;//NOT FOUND
	}else{
		return helperLookUpLabel(sibling->child,structEntry,chain->next);
	}
}

int LookUpLabel(Trie* trie, TTEntry* structEntry){
	return helperLookUpLabel(trie,structEntry,structEntry->val.structType.fieldNames);
}


Trie* encodeDeclNode(Trie* trie,TypeDeclNode* node){
	Trie* updatedTrie = trie;
	for(TypeDeclNode* cur = node; cur != NULL; cur = cur->nextDecl){
			TypeHolderNode* actualType = cur->actualType;
			if (actualType->kind == structType){
				updatedTrie = encodeInfo(updatedTrie,node->typeEntry);
			}
	}

	return updatedTrie;
}

Trie* encodeVarDecl(Trie* trie, VarDeclNode* node){
	if (node == NULL){
		return trie;
	}

	Trie* updatedTrie = trie;

	for(VarDeclNode* cur = node; cur != NULL; node = node->nextDecl){
		TTEntry* type = cur->whoAmI->type;
		if (type->underlyingType == structType){
			updatedTrie = encodeInfo(updatedTrie,type);
		}
	}

	return updatedTrie;
	
}

Trie* encodeStmtStruct(Stmt* stmt,Trie* trie){
	if (stmt == NULL){
		return trie;
	}

	Trie* updatedTrie = trie;

	switch (stmt->kind){
		StmtKindBlock:
			updatedTrie = encodeStmtStruct(stmt->val.block.stmt,updatedTrie);
			break;
		StmtKindIf:
			updatedTrie = encodeStmtStruct(stmt->val.ifStmt.block,updatedTrie);
			updatedTrie = encodeStmtStruct(stmt->val.ifStmt.elseBlock,updatedTrie);
			break;
		StmtKindElse:
			updatedTrie = encodeStmtStruct(stmt->val.elseStmt.block,updatedTrie);
			break;
		StmtKindSwitch:
			for(switchCaseClause* list = stmt->val.switchStmt.clauseList; list != NULL; list = list->next){
				updatedTrie = encodeStmtStruct(list->statementList,updatedTrie);
			}
			break;
		StmtKindInfLoop:
			updatedTrie = encodeStmtStruct(stmt->val.infLoop.block,updatedTrie);
			break;
		StmtKindWhileLoop:
			updatedTrie = encodeStmtStruct(stmt->val.whileLoop.block,updatedTrie);
			break;
		StmtKindThreePartLoop:
			updatedTrie = encodeStmtStruct(stmt->val.forLoop.block,updatedTrie);
			break;
		StmtKindTypeDeclaration:
			updatedTrie = encodeDeclNode(updatedTrie,stmt->val.typeDeclaration);
			break;
		StmtKindVarDeclaration:;
			updatedTrie = encodeVarDecl(updatedTrie,stmt->val.varDeclaration);
			break;
	}

	return encodeStmtStruct(stmt->next,updatedTrie);
}



Trie* encodeFunctionStruct(Trie* trie,FuncDeclNode* function){
	if (function == NULL){
		return trie;
	}

	Trie* updatedTrie = trie;

	updatedTrie = encodeVarDecl(updatedTrie,function->argsDecls);

	TTEntry* returnType = function->symbolEntry->type->val.functionType.ret;
	if (returnType != NULL && returnType->underlyingType == structType){
		//Non void type and is a struct
		updatedTrie = encodeInfo(updatedTrie,returnType);
	} 

	updatedTrie = encodeStmtStruct(function->blockStart,updatedTrie);
	return updatedTrie;

}


Trie* encodeTopDeclNode(Trie* trie,TopDeclarationNode* node){
	if (node == NULL){
		return trie;
	}

	Trie* updatedTrie = trie;
	switch(node->declType){
		case funcDeclType:
			updatedTrie =  encodeFunctionStruct(updatedTrie,node->actualRealDeclaration.funcDecl);
			break;
		case typeDeclType:
			updatedTrie = encodeDeclNode(updatedTrie,node->actualRealDeclaration.typeDecl);
			break;
		case variDeclType:
			updatedTrie = encodeVarDecl(updatedTrie,node->actualRealDeclaration.varDecl);
			break;

	}

	return encodeTopDeclNode(updatedTrie,node->nextTopDecl);
}


Trie* encodeRoot(RootNode* root){
	if (root == NULL){
		return NULL;
	}

	return encodeTopDeclNode(NULL,root->startDecls);

}
