#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "globalEnum.h"
#include "symbol_table.h"

TTEntry *builtInTypes;

void symbolCheckExpression(Exp *e, Context *c);
void symbolCheckExpressionList(ExpList* expressionList,Context* context);
void symbolCheckSwitchCaseClauseList(switchCaseClause* clauseList, Context* context);

void printClauseListSymbol(switchCaseClause* clauseList,int indentLevel);
TTEntry *makeNamedTTEntry(Context* contx, TypeDeclNode* declaration);
TTEntry *makeSubTTEntry(Context* contx, TypeHolderNode *holder, TTEntry* head, int inSlice);
TTEntry *makeAnonymousTTEntry(Context* contx, TypeHolderNode *holder);
TTEntry *makeGeneralTTEntry(Context* contx, TypeHolderNode *holder, char* identifier, TTEntry* head, int inSlice);
void symbolCheckVarDecl(VarDeclNode* declNode, Context* contx, int placement);



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
	
	for ( TTEntry *head = c->curTypeTable->entries[pos]; head; head = head->next ) {
		if ( strcmp(head->id, s->id) == 0 ) return 2;
		
	}
	STEntry *head = c->curSymbolTable->entries[pos];
	if (head == NULL) {
		c->curSymbolTable->entries[pos] = s;
		return 0;
	}
	while (head -> next != NULL)
	{
		if ( strcmp(head->id, s->id) == 0 ) return 1; 
		head = head->next;
	}
	if ( strcmp(head->id, s->id) == 0 ) return 1;
	head -> next = s;
	return 0;
}

int addTypeEntry(Context *c, TTEntry *t)
{
	int pos = hashCode(t->id);
	
	for(STEntry*head=c->curSymbolTable->entries[pos];head;head=head->next) {
		if ( strcmp(head->id, t->id) == 0 ) return 1;
		
	}
	TTEntry *head = c->curTypeTable->entries[pos];
	if (head == NULL) {
		c->curTypeTable->entries[pos] = t;
		return 0;
	}
	while (head -> next != NULL)
	{
		if ( strcmp(head->id, t->id) == 0 ) return 1; 
		head = head->next;
	}
	if ( strcmp(head->id, t->id) == 0 ) return 1;
	head -> next = t;
	return 0;
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






void symbolCheckExpression(Exp *e, Context *c)
{
	if ( e->kind == expKindIdentifier )
	{
		if ( getEntry(c, e->val.id) == NULL )
		{
			fprintf(stderr, "Error: (%d) %s not declared as a variable, nor type\n", e->lineno, e->val.id);
			exit(1);
		}
		e->contextEntry = getEntry(c, e->val.id);
	}
	else if ( e->kind == expKindFieldSelect || e->kind == expKindIndexing )
	{
		symbolCheckExpression(e->val.access.base, c);
	}
	else if ( e->kind == expKindFuncCall )
	{ 
		symbolCheckExpression(e->val.funcCall.base, c);
		ExpList *curArg = e->val.funcCall.arguments;
		while ( curArg != NULL )
		{
			symbolCheckExpression(curArg->cur, c);
			curArg = curArg->next;
		}
	}
	else if ( isBinary(e) )
	{
		symbolCheckExpression(e->val.binary.left, c);
		symbolCheckExpression(e->val.binary.right, c);
	}
	else if ( isUnary(e) )
	{
		symbolCheckExpression(e->val.unary, c);		
	}
	else if ( e->kind == expKindCapacity || e->kind == expKindLength )
	{
		symbolCheckExpression(e->val.builtInBody, c);
	}
	else if ( e->kind == expKindAppend )
	{
		symbolCheckExpression(e->val.append.list, c);
		symbolCheckExpression(e->val.append.elem, c);
	}
}

int expressionIsAFunctionCall(Exp* exp, Context* context);

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


		
		case StmtKindSwitch :
							newContext = scopedContext(context);
							
							symbolCheckStatement(stmt->val.switchStmt.statement,newContext);
							
							if (stmt->val.switchStmt.expression != NULL)
								symbolCheckExpression(stmt->val.switchStmt.expression,newContext);
							

							//I'm in sense encasing the clasue list in a block from the perspective of scope
							symbolCheckSwitchCaseClauseList(stmt->val.switchStmt.clauseList,scopedContext(newContext));
							break;



		case StmtKindBlock :
						newContext = scopedContext(context);
						symbolCheckStatement(stmt->val.block.stmt,newContext);
						break;




    	case StmtKindExpression: //TODO Needs to be an actual function call(distinguish between type casts and function calls) cannot be append, len or cap either
						expressionIsAFunctionCall(stmt->val.expression.expr,context);
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

						symbolCheckStatement(stmt->val.ifStmt.elseBlock,newContext);

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
			if (stmt->val.whileLoop.conditon != NULL)
				symbolCheckExpression(stmt->val.whileLoop.conditon,context);
			symbolCheckStatement(stmt->val.whileLoop.block,context);
			break;
		case StmtKindThreePartLoop :
			newContext = scopedContext(context);
			symbolCheckStatement(stmt->val.forLoop.init,newContext);
			
			if (stmt->val.forLoop.condition != NULL){
				symbolCheckExpression(stmt -> val.forLoop.condition, newContext);
			}
			symbolCheckStatement(stmt->val.forLoop.inc,newContext);
			symbolCheckStatement(stmt->val.forLoop.block,newContext);
			
			break;


		//Trivially symbolcheck
		case StmtKindBreak :break;
		case StmtKindContinue :break;


		case StmtKindInc : 
			symbolCheckExpression(stmt->val.incStmt.exp,context);
			break;
		case StmtKindDec :
			symbolCheckExpression(stmt->val.decStmt.exp,context);
			break;
		case StmtKindOpAssignment:
			symbolCheckExpression(stmt->val.opAssignment.lhs,context);
			symbolCheckExpression(stmt->val.opAssignment.rhs, context);
			break;
		



	
		//For Denali to implement (Probably want to modify declaration nodes to include symbol references)
		case StmtKindTypeDeclaration :
			if(0) {}
			TypeDeclNode *typeDeclIter = stmt -> val.typeDeclaration;
			TTEntry *t;
			while (typeDeclIter != NULL) {
				t = makeNamedTTEntry(context, typeDeclIter);
				if (t -> underlyingType == badType) {
					fprintf(stderr, "Error: (line %d) %s\n", stmt -> lineno, t -> id);
					exit(1);
				}
				
				if (strcmp(t -> id, "_") != 0){
					if (addTypeEntry(context, t) != 0) {
						fprintf(stderr, "Error: (line %d) identifier %s has already been declared in this scope\n", stmt -> lineno, t -> id);
						exit(1);
					}
				}
				typeDeclIter = typeDeclIter -> nextDecl;
			}
			
			break;
		case StmtKindVarDeclaration :
			symbolCheckVarDecl(stmt -> val.varDeclaration, context, 1);
			break;
		case StmtKindShortDeclaration : 
			symbolCheckVarDecl(stmt -> val.varDeclaration, context, 2);
			break;

			
	}

	symbolCheckStatement(stmt->next,context);
}

void symbolCheckExpressionList(ExpList* expressionList,Context* context){
	if (expressionList == NULL){
		return;
	}

	//symbolCheckExpression doesn't treat the blank identifier specially so I handle it here
	if (!isBlank(expressionList->cur))
		symbolCheckExpression(expressionList->cur,context);

	
	symbolCheckExpressionList(expressionList->next,context);

	
}


void symbolCheckSwitchCaseClauseList(switchCaseClause* clauseList, Context* context){
	if (clauseList == NULL){
		return;
	}

	symbolCheckExpressionList(clauseList->expressionList,context);
	symbolCheckStatement(clauseList->statementList,scopedContext(context)); // Each stmt list in a clause has its own scope

	symbolCheckSwitchCaseClauseList(clauseList->next,context);
}


void symbolCheckProgram(RootNode* root) {
	Context* masterContx = newContext();
	builtInTypes = malloc(sizeof(TTEntry));
	TTEntry* placeHolder = builtInTypes;
	builtInTypes -> id = "int";
	builtInTypes -> underlyingType = identifierType;
	builtInTypes -> val.nonCompositeType.type = baseInt;
	builtInTypes -> comparable = 1;
	addTypeEntry(masterContx, builtInTypes);
	builtInTypes -> next = malloc(sizeof(TTEntry));
	builtInTypes = builtInTypes -> next;
	builtInTypes -> id = "float64";
	builtInTypes -> underlyingType = identifierType;
	builtInTypes -> val.nonCompositeType.type = baseFloat64;
	builtInTypes -> comparable = 1;
	addTypeEntry(masterContx, builtInTypes);
	builtInTypes -> next = malloc(sizeof(TTEntry));
	builtInTypes = builtInTypes -> next;
	builtInTypes -> id = "bool";
	builtInTypes -> underlyingType = identifierType;
	builtInTypes -> val.nonCompositeType.type = baseBool;
	builtInTypes -> comparable = 1;
	addTypeEntry(masterContx, builtInTypes);
	STEntry *builtInVals = malloc(2*sizeof(STEntry));
	builtInVals -> id = "true";
	builtInVals -> type = builtInTypes;
	builtInVals -> isConstant = 1;
	addSymbolEntry(masterContx, builtInVals);
	builtInVals ++;
	builtInVals -> id = "false";
	builtInVals -> type = builtInTypes;
	builtInVals -> isConstant = 1;
	addSymbolEntry(masterContx, builtInVals);
	builtInTypes -> next = malloc(sizeof(TTEntry));
	builtInTypes = builtInTypes -> next;
	builtInTypes -> id = "rune";
	builtInTypes -> underlyingType = identifierType;
	builtInTypes -> val.nonCompositeType.type = baseRune;
	builtInTypes -> comparable = 1;
	addTypeEntry(masterContx, builtInTypes);
	builtInTypes -> next = malloc(sizeof(TTEntry));
	builtInTypes = builtInTypes -> next;
	builtInTypes -> id = "string";
	builtInTypes -> underlyingType = identifierType;
	builtInTypes -> val.nonCompositeType.type = baseString;
	builtInTypes -> comparable = 1;
	addTypeEntry(masterContx, builtInTypes);
	builtInTypes -> next = NULL;
	builtInTypes = placeHolder;
	masterContx = scopedContext(masterContx);
	TopDeclarationNode *iter = root -> startDecls;
	while (iter != NULL) {
		if (iter -> declType == typeDeclType) {
			TypeDeclNode *typeDeclIter = iter -> actualRealDeclaration.typeDecl;
			TTEntry *t;
			while (typeDeclIter != NULL) {
				if (strcmp(typeDeclIter -> identifier, "main") == 0 || strcmp(typeDeclIter -> identifier, "init") == 0) {
					fprintf(stderr, "Error: (line %d) can only declare %s as a funciton at toplevel\n", typeDeclIter -> lineno, typeDeclIter -> identifier);
					exit(1);
				}
				t = makeNamedTTEntry(masterContx, typeDeclIter);
				if (t -> underlyingType == badType) {
					fprintf(stderr, "Error: (line %d) %s\n", typeDeclIter -> lineno, t -> id);
					exit(1);
				}
				if (strcmp(t -> id, "_") != 0){
					if (addTypeEntry(masterContx, t) != 0) {
						fprintf(stderr, "Error: (line %d) identifier (%s) has already been declared in this scope\n", typeDeclIter -> lineno, t -> id);
						exit(1);
					}
				}
				typeDeclIter = typeDeclIter -> nextDecl;
			}
		} else if (iter -> declType == variDeclType) {
			symbolCheckVarDecl(iter -> actualRealDeclaration.varDecl, masterContx, 0);
		} else if (iter -> declType == funcDeclType){
			
			
			STEntry *s = malloc(sizeof(STEntry));
			s -> id = iter -> actualRealDeclaration.funcDecl -> identifier;
			s -> isConstant = 1;
			iter -> actualRealDeclaration.funcDecl -> symbolEntry = s;
			
			Context* functionContext = scopedContext(masterContx);
			
			if (strcmp(s -> id, "init") == 0) {
				if (iter -> actualRealDeclaration.funcDecl -> returnType != NULL || iter -> actualRealDeclaration.funcDecl -> argsDecls != NULL) {
					fprintf(stderr, "Error: (line %d) init must have no arguments and void return type\n", iter -> actualRealDeclaration.funcDecl -> lineno);
					exit(1);
				}
				symbolCheckStatement(iter -> actualRealDeclaration.funcDecl -> blockStart, functionContext);
			} else  {
				
				
				if (strcmp(s -> id, "main") == 0) {
					if (iter -> actualRealDeclaration.funcDecl -> returnType != NULL || iter -> actualRealDeclaration.funcDecl -> argsDecls != NULL) {
						fprintf(stderr, "Error: (line %d) main must have no arguments and void return type\n", iter -> actualRealDeclaration.funcDecl -> lineno);
						exit(1);
					}
				}
				
				if (strcmp(s -> id, "_") != 0){
					if (addSymbolEntry(masterContx, s) != 0) {
						fprintf(stderr, "Error: (line %d) identifier (%s) has already been declared in this scope\n", iter -> actualRealDeclaration.funcDecl -> lineno, s -> id);
						exit(1);
					}
				}
				
				s -> type = malloc(sizeof(TTEntry));
				s -> type -> id = NULL;
				s -> type -> underlyingType = funcType;
				s -> type -> comparable = 0;
				
				if (iter -> actualRealDeclaration.funcDecl -> returnType == NULL) {
					s -> type -> val.functionType.ret = NULL;
				} else {
					s -> type -> val.functionType.ret = makeAnonymousTTEntry(masterContx, iter -> actualRealDeclaration.funcDecl -> returnType);
					if (s -> type -> val.functionType.ret -> underlyingType == badType) {
						fprintf(stderr, "Error: (line %d) problem with function return type: %s\n", iter -> actualRealDeclaration.funcDecl -> lineno, s -> type -> val.functionType.ret -> id);
						exit(1);
					}
				}
				
				VarDeclNode* argsIter = iter -> actualRealDeclaration.funcDecl -> argsDecls;
				if (argsIter != NULL){
					int returnCode;
					STEntry *argEntryIter = malloc(sizeof(STEntry));
					argEntryIter -> id = argsIter -> identifier;
					argEntryIter -> type = makeAnonymousTTEntry(masterContx, argsIter -> typeThing);
					argEntryIter -> isConstant = 0;
					if (argEntryIter -> type -> underlyingType == badType) {
						fprintf(stderr, "Error: (line %d) %s\n", argsIter -> lineno, argEntryIter -> type -> id);
						exit(1);
					};
					if (strcmp(argEntryIter -> id, "_") != 0) {
						returnCode = addSymbolEntry(functionContext, argEntryIter);
					}
					s -> type -> val.functionType.args = argEntryIter;
					
					argsIter = argsIter -> nextDecl;
					
					while (argsIter != NULL) {
						argEntryIter -> next = malloc(sizeof(STEntry));
						argEntryIter = argEntryIter -> next;
						argEntryIter -> id = argsIter -> identifier;
						argEntryIter -> type = makeAnonymousTTEntry(masterContx, argsIter -> typeThing);
						argEntryIter -> isConstant = 0;
						if (argEntryIter -> type -> underlyingType == badType) {
							fprintf(stderr, "Error: (line %d) %s\n", argsIter -> lineno, argsIter -> identifier, argEntryIter -> id);
							exit(1);
						}
						if (strcmp(argEntryIter -> id, "_") != 0) {
							returnCode = addSymbolEntry(functionContext, argEntryIter);
							if (returnCode != 0) {
								fprintf(stderr, "Error: (line %d) function arguments must have unique names\n", argsIter -> lineno);
								exit(1);
							}
						}
						
						argsIter = argsIter -> nextDecl;
					}
					argEntryIter -> next = NULL;
				}
				
				symbolCheckStatement(iter -> actualRealDeclaration.funcDecl -> blockStart, functionContext);
			}
			
		} else {
			fprintf(stderr, "I don't know what the fuck just happened, but I don't really care: I'm a get the fuck up out of here. Fuck this shit, I'm out.\n");
			exit(1);
		}
		iter = iter -> nextTopDecl;
	}
}







TTEntry *makeNamedTTEntry(Context* contx, TypeDeclNode* declaration){
	return makeGeneralTTEntry(contx, declaration -> actualType, declaration -> identifier, NULL, 0);
}

TTEntry *makeSubTTEntry(Context* contx, TypeHolderNode *holder, TTEntry* head, int inSlice) {
	return makeGeneralTTEntry(contx, holder, NULL, head, inSlice);
}

TTEntry *makeAnonymousTTEntry(Context* contx, TypeHolderNode *holder) {
	return makeGeneralTTEntry(contx, holder, NULL, NULL, 0);
}


TTEntry *makeGeneralTTEntry(Context* contx, TypeHolderNode *holder, char* identifier, TTEntry* head, int inSlice){
	
	
	TTEntry *t = malloc(sizeof(TTEntry));
	t -> id = identifier;
	
	if (holder -> kind == identifierType) {
		if (head != NULL){
			if (strcmp(holder -> identification, head -> id) == 0){
				if (inSlice == 0) {
					t -> id = "invalid recursive type";
					t -> underlyingType = badType;
					return t;
				} else {
					return head;
				}
			}
		} else if (identifier != NULL) {
			if (strcmp(identifier, holder -> identification) == 0) {
				t -> id = "invalid recursive type";
				t -> underlyingType = badType;
				return t;
			}
		}
		PolymorphicEntry* assignee = getEntry(contx, holder -> identification);
		if (assignee == NULL) {
			t -> id = "using a type which has not been declared";
			t -> underlyingType = badType;
			return t;
		}
		if (assignee -> isSymbol == 1) {
			t -> id = "cannot use a variable as a type";
			t -> underlyingType = badType;
			return t;
		}
		if (identifier == NULL) {
			return assignee -> entry.t;
		}
		
		t -> underlyingType = assignee -> entry.t -> underlyingType;
		t -> val.nonCompositeType.type = assignee -> entry.t -> val.nonCompositeType.type;
		t -> val.sliceType.type = assignee -> entry.t -> val.sliceType.type;
		t -> val.arrayType.type = assignee -> entry.t -> val.arrayType.type;
		t -> val.arrayType.size = assignee -> entry.t -> val.arrayType.size;
		t -> val.structType.fields = assignee -> entry.t -> val.structType.fields;
		t -> comparable = assignee -> entry.t -> comparable;
		return t;
	}
	TTEntry *innerType;
	t -> underlyingType = holder -> kind;
	if (holder -> kind == inferType) {
		return t;
	}
	if (t -> underlyingType == arrayType) {
		if (head == NULL && identifier == NULL){
			innerType = makeAnonymousTTEntry(contx, holder -> underlyingType);
		} else if (head == NULL) {
			innerType = makeSubTTEntry(contx, holder -> underlyingType, t, 0);
		} else {
			innerType = makeSubTTEntry(contx, holder -> underlyingType, head, inSlice);
		}
		if (innerType -> underlyingType == badType) {
			t -> id = innerType -> id;
			t -> underlyingType = badType;
			return t;
		}
		t -> val.arrayType.type = innerType;
		t -> val.arrayType.size = holder -> arrayDims;
		t -> comparable = innerType -> comparable;
	} else if (t -> underlyingType == sliceType) {
		
		if (head == NULL && identifier == NULL){
			innerType = makeAnonymousTTEntry(contx, holder -> underlyingType);
		} else if (head == NULL) {
			innerType = makeSubTTEntry(contx, holder -> underlyingType, t, 1);
		} else {
			innerType = makeSubTTEntry(contx, holder -> underlyingType, head, 1);
		}
		if (innerType -> underlyingType == badType) {
			t -> id = innerType -> id;
			t -> underlyingType = badType;
			return t;
		}
		t -> val.sliceType.type = innerType;
		t -> comparable = 0;
	} else if (t -> underlyingType == structType){
		t -> comparable = 1;
		VarDeclNode *sMembs = holder -> structMembers;
		Context *tentativeContext = newContext();
		STEntry *memberEntry;
		IdChain* idMover = malloc(sizeof(IdChain));
		t -> val.structType.fieldNames = idMover;
		int returnCode;
		while (sMembs != NULL) {
			if (head == NULL && identifier == NULL){
				innerType = makeAnonymousTTEntry(contx, sMembs -> typeThing);
			} else if (head == NULL) {
				innerType = makeSubTTEntry(contx, sMembs -> typeThing, t, 0);
			} else {
				innerType = makeSubTTEntry(contx, sMembs -> typeThing, head, inSlice);
			}
			if (innerType -> underlyingType == badType) {
				printf("hello\n");
				t -> id = innerType -> id;
				t -> underlyingType = badType;
				return t;
			}
			t -> comparable *= innerType -> comparable;   //this line scares me
			
			
			memberEntry = malloc(sizeof(STEntry));
			memberEntry -> id = sMembs -> identifier;
			memberEntry -> type = innerType;
			returnCode = addSymbolEntry(tentativeContext, memberEntry);
			if (returnCode != 0) {
				t -> id = "duplicate struct members";
				t -> underlyingType = badType;
				return t;
			}
			
			idMover -> identifier = sMembs -> identifier;
			sMembs = sMembs -> nextDecl;
			
			if (sMembs == NULL) {
				idMover -> next = NULL;
			} else {
				
				idMover -> next = malloc(sizeof(IdChain));
				idMover = idMover -> next;
			}
			
			
		}
		t -> val.structType.fields = tentativeContext;
		
		IdChain* temp = t -> val.structType.fieldNames;
		
		
	} else {
		fprintf(stderr, "I was passed a bad TypeDeclNode. You shouldn't ever see this.\n");
		return NULL;
	}
	return t;
}


void symbolCheckVarDecl(VarDeclNode* declNode, Context* contx, int placement) { /* 0 for toplevel (main, init), 1 for inside function, 2 for shortDecl */
	if (declNode == NULL) {return;}
	int shortDeclMustDecl = 0;
	VarDeclNode* varDeclIter = declNode;
	STEntry *s;
	while (varDeclIter != NULL) {
		if (varDeclIter -> value != NULL) {
			symbolCheckExpression(varDeclIter -> value, contx);
		}
		varDeclIter = varDeclIter -> nextDecl;
	}
	varDeclIter = declNode;
	while (varDeclIter != NULL) {
		if (placement == 0) {
			if (strcmp(varDeclIter -> identifier, "main") == 0 || strcmp(varDeclIter -> identifier, "init") == 0) {
				fprintf(stderr, "Error: (line %d) may only declare %s as a funciton at toplevel\n", varDeclIter -> lineno, varDeclIter -> identifier);
				exit(1);
			}
		} 
		
		
		s = malloc(sizeof(STEntry));
		s -> id = varDeclIter -> identifier;
		s -> type = makeAnonymousTTEntry(contx, varDeclIter -> typeThing);
		s -> isConstant = 0;
		varDeclIter -> whoAmI = s;
		
		if (s -> type -> underlyingType == badType) {
			fprintf(stderr, "Error: (line %d) invalid type used in variable declaration: %s\n", varDeclIter -> lineno, s -> id);
			exit(1);
		}
		if (strcmp(s -> id, "_") != 0){
			if (addSymbolEntry(contx, s) != 0) {
				if (placement == 2) {
					varDeclIter -> iDoDeclare = 0;
					PolymorphicEntry* toBeShort = getEntry(contx, s -> id);
					if (toBeShort -> isSymbol == 0) {
						fprintf(stderr, "Error: (line %d) identifier (%s) has already been declared as a type in this scope\n", varDeclIter -> lineno, s -> id);
						exit(1);
					}
					varDeclIter -> whoAmI = toBeShort -> entry.s;
				} else {
					fprintf(stderr, "Error: (line %d) identifier (%s) has already been declared in this scope\n", varDeclIter -> lineno, s -> id);
					exit(1);
				}
				
			} else {
				shortDeclMustDecl ++;
				varDeclIter -> iDoDeclare = 1;
			}
		}
		
		varDeclIter = varDeclIter -> nextDecl;
	}
	
	if (shortDeclMustDecl == 0 && placement == 2) {
		fprintf(stderr, "Error: (line %d) short declarations must declare at least one new variable\n", declNode -> lineno);
		exit(1);
	}
	symbolCheckVarDecl(declNode -> multiDecl, contx, placement);
	
}






static void indent(int indentLevel){
	for(int i = 0; i < indentLevel; i++){
		printf("\t");
	}
}


void symbolPrintTypeDecl(TypeDeclNode *type, int indentLevel);
void symbolPrintTypeHolder(TypeHolderNode *node, int indentLevel);
void symbolPrintStructMembers(VarDeclNode *type, int indentLevel);
void symbolPrintVarDecl(VarDeclNode *var, int indentLevel);
void symbolPrintShortVarDecl(VarDeclNode *var, int indentLevel);
void symbolPrintFuncDecl(FuncDeclNode *func);
void symbolPrintFuncArgs(VarDeclNode *args);
 
void printStatementSymbol(Stmt* stmt,int indentLevel){

		if (stmt == NULL){
			return;
		}


		switch (stmt->kind){

			
			case StmtKindSwitch : 
								indent(indentLevel);
								printf("{\n");

								printStatementSymbol(stmt->val.switchStmt.statement,indentLevel+1);
								printClauseListSymbol(stmt->val.switchStmt.clauseList,indentLevel+1);

								
								indent(indentLevel);
								printf("}\n");
								break;

			case StmtKindBlock : indent(indentLevel);
								printf("{\n");
								printStatementSymbol(stmt->val.block.stmt,indentLevel+1);
								
								indent(indentLevel);
								printf("}\n");
								break;


			//Cannot introduce symbol, scope
			case StmtKindExpression : break; 
			case StmtKindAssignment : break;
			case StmtKindPrintln : break;
			case StmtKindPrint : break;
			case StmtKindReturn : break;
			case StmtKindBreak : break;
			case StmtKindContinue : break;




			case StmtKindIf : indent(indentLevel);
								printf("{\n");

								printStatementSymbol(stmt->val.ifStmt.statement,indentLevel+1);

								printStatementSymbol(stmt->val.ifStmt.block,indentLevel+1);


								printStatementSymbol(stmt->val.ifStmt.elseBlock,indentLevel+1);

								printf("\n");
								indent(indentLevel);
								printf("}\n");

								

								
								break;

			case StmtKindElse : printStatementSymbol(stmt->val.elseStmt.block,indentLevel);
								break;
			case StmtKindInfLoop : printStatementSymbol(stmt->val.infLoop.block,indentLevel);
									break;
			case StmtKindWhileLoop : printStatementSymbol(stmt->val.whileLoop.block,indentLevel);
									break;
			case StmtKindThreePartLoop :indent(indentLevel);
										printf("{\n");

										printStatementSymbol(stmt->val.forLoop.init,indentLevel+1);
										printStatementSymbol(stmt->val.forLoop.inc,indentLevel+1);

										printStatementSymbol(stmt->val.forLoop.block,indentLevel+1);

										printf("\n");
										indent(indentLevel);
										printf("}\n");
										break;


			
			//For Denali to implement (I also designed the rest with the assumption that the following are terminated with newline characters)
			case StmtKindTypeDeclaration:
				symbolPrintTypeDecl(stmt -> val.typeDeclaration, indentLevel);
				break;
			case StmtKindVarDeclaration:
				symbolPrintVarDecl(stmt -> val.varDeclaration, indentLevel);
				break;
			case StmtKindShortDeclaration: 
				symbolPrintShortVarDecl(stmt -> val.varDeclaration, indentLevel);
				break;

			



		}

		printStatementSymbol(stmt->next,indentLevel);
}

void printSymbolProgram(RootNode* rootNode) {
	printf("{\n");
	printf("\t\"int\" type defined as: int\n");
	printf("\t\"string\" type defined as: string\n");
	printf("\t\"float64\" type defined as: float64\n");
	printf("\t\"bool\" type defined as: bool\n");
	printf("\t\"rune\" type defined as: rune\n");
	
	printf("\t\"true\" constant of type: bool\n");
	printf("\t\"false\" constant of type: bool\n");
	printf("\t{\n");
	
	TopDeclarationNode *iter = rootNode -> startDecls;
	while (iter != NULL) {
		if (iter -> declType == typeDeclType) {
			symbolPrintTypeDecl(iter -> actualRealDeclaration.typeDecl, 2);
		} else if (iter -> declType == variDeclType) {
			symbolPrintVarDecl(iter -> actualRealDeclaration.varDecl, 2);
		} else if (iter -> declType == funcDeclType) {
			symbolPrintFuncDecl(iter -> actualRealDeclaration.funcDecl);
		} else {
			fprintf(stderr, "something went wrong, go fuck yourself\n");
			exit(1);
		}
		iter = iter -> nextTopDecl;
	}
	printf("\t}\n");
	printf("}\n");
}




void printClauseListSymbol(switchCaseClause* clauseList,int indentLevel){
	if (clauseList == NULL){
		return;
	}

	indent(indentLevel);
	printf("{\n");
	printStatementSymbol(clauseList->statementList,indentLevel+1);
	//printf("\n");
	indent(indentLevel);
	printf("}\n");

	printClauseListSymbol(clauseList->next,indentLevel);
}


int expressionIsAFunctionCall(Exp* exp, Context* context){
	if (exp == NULL){
		return 0;
	}

	if (exp->kind != expKindFuncCall){
		return 0;
	}

	PolymorphicEntry* entry = getEntry(context,exp->val.funcCall.base->val.id);

	if (entry == NULL){ //Should not symbol check, let the expression symbol checker throw the error
		symbolCheckExpression(exp,context);
	}else if (entry->isSymbol != 1) {//not a symbol => is a type{
		fprintf(stderr, "Error: (line %d) expression statement must be a function call \n", exp->lineno);
		exit(1);
	}

	//Should symbolcheck by now (unless something is up with the args)
	symbolCheckExpression(exp,context);


	return 1;
}



void symbolPrintTypeDecl(TypeDeclNode *type, int indentLevel)
{
	if ( type == NULL ) return;
	indent(indentLevel);
	printf("\"%s\" type defined as: ", type->identifier);
	symbolPrintTypeHolder(type->actualType, indentLevel);
	printf("\n");
	symbolPrintTypeDecl(type->nextDecl, indentLevel);
}

void symbolPrintTypeHolder(TypeHolderNode *node, int indentLevel)
{
	if ( node == NULL ) return;
	switch (node->kind)
	{
		case sliceType:
			printf("[]");
			symbolPrintTypeHolder(node -> underlyingType, indentLevel);
			break;
		case arrayType:
			printf("[%d]", node->arrayDims);
			symbolPrintTypeHolder(node -> underlyingType, indentLevel);
			break;
		case identifierType:
			printf("%s", node->identification);
			break;
		case structType:
			printf(" struct {\n");
			symbolPrintStructMembers(node->structMembers, indentLevel + 1);
			indent(indentLevel);
			printf("}");
			break;
		case inferType:
			printf("<infer>");
			break;
	}
}


void symbolPrintStructMembers(VarDeclNode *members, int indentLevel)
{
	if ( members == NULL ) return;
	indent(indentLevel);
	printf("%s", members->identifier);
	if ( members->typeThing != NULL )
	{
		symbolPrintTypeHolder(members->typeThing, indentLevel);
	}
	printf("\n");
	symbolPrintStructMembers(members->nextDecl, indentLevel);
}

void symbolPrintVarDecl(VarDeclNode *var, int indentLevel)
{
	if ( var == NULL ) return;
	indent(indentLevel);
	printf("\"%s\" variable of type ", var->identifier);
	symbolPrintTypeHolder(var->typeThing, indentLevel);
	printf("\n");
	symbolPrintVarDecl(var->nextDecl, indentLevel);
	symbolPrintVarDecl(var -> multiDecl, indentLevel);
}

void symbolPrintShortVarDecl(VarDeclNode *var, int indentLevel)
{
	if ( var == NULL ) return;
	if (var -> iDoDeclare == 1) {
		indent(indentLevel);
		printf("\"%s\" variable of type <infer>", var->identifier);
		printf("\n");
	}
	symbolPrintShortVarDecl(var->nextDecl, indentLevel);
}

void symbolPrintFuncDecl(FuncDeclNode *func)
{
	if ( func == NULL ) return;
	if (strcmp(func -> identifier, "init") == 0) {
		printf("\t\t\"init\" function: [unmapped]");
	} else {
		printf("\t\t\"%s\" function: (", func->identifier);
		symbolPrintFuncArgs(func -> argsDecls);
		printf(") -> ");
		if ( func->returnType != NULL )
		{
			symbolPrintTypeHolder(func->returnType, 2);
		} else {
			printf("void");
		}
	}
	printf("\n\t\t{\n");
	symbolPrintVarDecl(func -> argsDecls, 3);
	printStatementSymbol(func->blockStart -> val.block.stmt, 3);
	printf("\t\t}\n");
}

void symbolPrintFuncArgs(VarDeclNode *args)
{
	if ( args == NULL ) return;
	if ( args->typeThing != NULL )
	{
		symbolPrintTypeHolder(args->typeThing, 2);
	}
	if ( args->nextDecl != NULL )
	{
		printf(", ");
		symbolPrintFuncArgs(args->nextDecl);
	}
}



