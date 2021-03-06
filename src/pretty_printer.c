
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "globalEnum.h"
#include "ast.h"
#include "pretty_printer.h"



void prettyExpList(ExpList *list);
void prettyExp(Exp * exp);

void printIfStmt(Stmt* stmt,int indentLevel);
void printSwitchStmt(Stmt* stmt,int indentLevel);

void printSimpleStatement(Stmt* stmt);
void printSwitchCaseClause(switchCaseClause* clause,int indentLevel);

void prettyTypeHolder(TypeHolderNode *node, int indentLevel);
void prettyFuncDecl(FuncDeclNode *func, int indentLevel);
void prettyTypeDecl(TypeDeclNode *type, int indentLevel);
void prettyFuncArgs(VarDeclNode *type, int indentLevel);
void prettyStructMembers(VarDeclNode *members, int indentLevel);
void prettyVarDecl(VarDeclNode *var, int indentLevel);
void prettyShortVarDecl(VarDeclNode *var, int indentLevel);
void prettyTopDeclaration(TopDeclarationNode *topDecl, int indentLevel);

void prettyVarDeclSimpleStatement(VarDeclNode *var);
void printPartiallyIndentedBlock(Stmt* stmt,int indentLevel);

void printAssignmentStmt(Stmt* stmt);
void printOpAssignmentStmt(Stmt* stmt);

void indent(int indentLevel){
    for(int i = 0 ; i < indentLevel; i++){
        printf("    ");
    }
}

void printStmt(Stmt* stmt, int indentLevel){
    if (stmt == NULL){
        return;
    }

    switch (stmt->kind){
        case StmtKindBlock :    
								indent(indentLevel);
								printf("{\n");
                                printStmt(stmt->val.block.stmt,indentLevel + 1);
                                indent(indentLevel);
                                printf("}\n");
                                break;

        case StmtKindExpression : 	indent(indentLevel);
									printSimpleStatement(stmt);
									printf(";");
                                    printf("\n");
                                    break;
                                    
        case StmtKindAssignment : indent(indentLevel);
									printSimpleStatement(stmt);
									printf(";");
                                    printf("\n");
                                    break;
		case StmtKindShortDeclaration: 
			prettyShortVarDecl(stmt->val.varDeclaration, indentLevel);
			break;
		case StmtKindVarDeclaration:
			prettyVarDecl(stmt->val.varDeclaration, indentLevel);
			break;
		case StmtKindTypeDeclaration:
			prettyTypeDecl(stmt->val.typeDeclaration, indentLevel);
			break;
        case StmtKindPrint : 	indent(indentLevel);
								printf("print(");
                                prettyExpList(stmt->val.print.list);
                                printf(");\n");
                                break;

        case StmtKindPrintln : indent(indentLevel);
								printf("println(");
                                prettyExpList(stmt->val.println.list);
                                printf(");\n");
                                break;
        case StmtKindIf :   indent(indentLevel);
							printIfStmt(stmt,indentLevel);
                            break;
        case StmtKindElse: printf("else");
							printPartiallyIndentedBlock(stmt->val.elseStmt.block,indentLevel);
                            break;
        case StmtKindReturn: 
							indent(indentLevel);
                            printf("return ");
                            if (stmt->val.returnVal.returnVal != NULL){
                                prettyExp(stmt->val.returnVal.returnVal);
                            }
                            printf(";\n");
                            break;

        case StmtKindSwitch: 	indent(indentLevel);
								printSwitchStmt(stmt,indentLevel);
								break;

        case StmtKindInfLoop:   indent(indentLevel);
								printf("for ");
                                printPartiallyIndentedBlock(stmt->val.infLoop.block,indentLevel);
                                break;

        case StmtKindWhileLoop: indent(indentLevel);
								printf("for ");
                                prettyExp(stmt->val.whileLoop.conditon);
                                printPartiallyIndentedBlock(stmt->val.whileLoop.block,indentLevel);
                                break;
        case StmtKindThreePartLoop:  indent(indentLevel);
									printf("for ");
                                    printSimpleStatement(stmt->val.forLoop.init);
                                    printf(";");
                                    prettyExp(stmt->val.forLoop.condition);
                                    printf(";");
                                    printSimpleStatement(stmt->val.forLoop.inc);

                                    printPartiallyIndentedBlock(stmt->val.forLoop.block,indentLevel);

                                    break;

        case StmtKindBreak:     indent(indentLevel);
								printf("break;\n");
                                break;
        case StmtKindContinue: indent(indentLevel);
								printf("continue;\n");
                                break;
		
		case StmtKindInc: indent(indentLevel);
							printSimpleStatement(stmt);
							printf("\n");
							break;
		case StmtKindDec: indent(indentLevel);
							printSimpleStatement(stmt);
							printf("\n");
							break;
		case StmtKindOpAssignment : 
						indent(indentLevel);
						printSimpleStatement(stmt);
						printf("\n");
						break;

        
    }

    printStmt(stmt->next,indentLevel);
}


void printIfStmt(Stmt* stmt,int indentLevel){
    Stmt* simpleStmt = stmt->val.ifStmt.statement;
    Exp* expr = stmt->val.ifStmt.expression;
    Stmt* block = stmt->val.ifStmt.block;

	

   
	printf("if ");


    if (simpleStmt != NULL){
        printSimpleStatement(simpleStmt);
		printf(";");
    }


	
    if (expr != NULL){
        prettyExp(expr);
        
    }

	printf("{\n\n");

    printStmt(block->val.block.stmt,indentLevel+1);
	printf("\n");
	indent(indentLevel);
	printf("}");

	if (stmt->val.ifStmt.elseBlock == NULL){
		printf("\n\n");
	}

    printStmt(stmt->val.ifStmt.elseBlock,indentLevel);
}


void printSwitchStmt(Stmt* stmt,int indentLevel){
    if (stmt == NULL){
        return;
    }
    Stmt* simpleStatement = stmt->val.switchStmt.statement;
    Exp* expr = stmt->val.switchStmt.expression;
    switchCaseClause* clause = stmt->val.switchStmt.clauseList;

    printf("switch ");

    if (simpleStatement != NULL){
        printSimpleStatement(simpleStatement);
		printf(";");
        
    }

    if (expr != NULL){
        prettyExp(expr);
    }

    printf("{\n");

    printSwitchCaseClause(clause,indentLevel+1);

    indent(indentLevel);

    printf("}\n");


}

void printSimpleStatement(Stmt* stmt){
    if (stmt == NULL){
        return;
    }
    switch (stmt->kind){

        case StmtKindExpression : prettyExp(stmt->val.expression.expr);  
                                    break;
                                    
        case StmtKindAssignment : printAssignmentStmt(stmt);
                                    break;
		case StmtKindShortDeclaration : prettyVarDeclSimpleStatement(stmt->val.varDeclaration);
										break;
		case StmtKindInc:
			prettyExp(stmt->val.incStmt.exp);
			printf("++");
			break;
		case StmtKindDec:
			prettyExp(stmt->val.decStmt.exp);
			printf("--");
			break;
		case StmtKindOpAssignment:
			printOpAssignmentStmt(stmt);
			break;
       
    }
}





void printSwitchCaseClause(switchCaseClause* clause,int indentLevel){
    if (clause == NULL){
        return;
    }

    indent(indentLevel);
    if (clause->expressionList == NULL){
        printf("default : \n");
    }else{
        printf("case ");
        prettyExpList(clause->expressionList);
        printf(" : \n");
    }

    printStmt(clause->statementList,indentLevel + 1);
	printf("\n");
    printSwitchCaseClause(clause->next,indentLevel);


}

void prettyExp(Exp * exp)
{
	if ( exp == NULL ) return;
	switch ( exp->kind )
	{
		case expKindIdentifier:
			printf("%s", exp->val.id);
			break;
		case expKindIntLit:
			printf("%d", exp->val.intLit);
			break;
		case expKindFloatLit:
			printf("%lf", exp->val.floatLit);
			break;
		case expKindRuneLit: 
			printf("%s", exp->val.runeLit);
			break;
		case expKindRawStringLit:
		case expKindInterpretedStringLit:
			printf("%s", exp->val.stringLit);
			break;
		case expKindFuncCall:
			prettyExp(exp->val.funcCall.base);
			printf("(");
			prettyExpList(exp->val.funcCall.arguments);
			printf(")");
			break;
		case expKindIndexing:
			prettyExp(exp->val.access.base);
			printf("[");
			prettyExp(exp->val.access.accessor);
			printf("]");
			break;
		case expKindFieldSelect:
			prettyExp(exp->val.access.base);
			printf(".");
			prettyExp(exp->val.access.accessor);
			break;
		case expKindAppend:
			printf("append(");
			prettyExp(exp->val.append.list);
			printf(", ");
			prettyExp(exp->val.append.elem);
			printf(")");
			break;
		case expKindLength:
			printf("len(");
			prettyExp(exp->val.builtInBody);
			printf(")");
			break;
		case expKindCapacity:
			printf("cap(");
			prettyExp(exp->val.builtInBody);
			printf(")");
			break;
		case expKindLogicNot: //switch statements <3
		case expKindUnaryMinus:
		case expKindUnaryPlus:
		case expKindBitNotUnary:
			switch (exp->kind) {
				case expKindLogicNot:
					printf("!");
					break;
				case expKindUnaryMinus:
					printf("-");
					break;
				case expKindUnaryPlus:
					printf("+");
					break;
				case expKindBitNotUnary:
					printf("^");
					break;
			}
			printf("(");
			prettyExp(exp->val.unary);
			printf(")");
			break;
		default:
			printf("(");
			switch (exp->kind) {
				case expKindAddition:
					prettyExp(exp->val.binary.left);
					printf(" + " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindSubtraction:
					prettyExp(exp->val.binary.left);
					printf(" - " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindMultiplication:
					prettyExp(exp->val.binary.left);
					printf(" * " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindDivision:
					prettyExp(exp->val.binary.left);
					printf(" / " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindLogicOr:
					prettyExp(exp->val.binary.left);
					printf(" || " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindLogicAnd:
					prettyExp(exp->val.binary.left);
					printf(" && " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindLEQ:
					prettyExp(exp->val.binary.left);
					printf(" <= " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindGEQ:
					prettyExp(exp->val.binary.left);
					printf(" >= " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindNEQ:
					prettyExp(exp->val.binary.left);
					printf(" != " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindEQ:
					prettyExp(exp->val.binary.left);
					printf(" == " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindLess:
					prettyExp(exp->val.binary.left);
					printf(" < " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindGreater:
					prettyExp(exp->val.binary.left);
					printf(" > " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindMod:
					prettyExp(exp->val.binary.left);
					fputs(" % ", stdout); //for denali <3
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitAnd:
					prettyExp(exp->val.binary.left);
					printf(" & " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitOr:
					prettyExp(exp->val.binary.left);
					printf(" | " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitNotBinary:
					prettyExp(exp->val.binary.left);
					printf(" ^ " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitShiftLeft:
					prettyExp(exp->val.binary.left);
					printf(" << " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitShiftRight:
					prettyExp(exp->val.binary.left);
					printf(" >> " );
					prettyExp(exp->val.binary.right);
					break;
				case expKindBitAndNot:
					prettyExp(exp->val.binary.left);
					printf(" &^ " );
					prettyExp(exp->val.binary.right);
					break;
			}
			printf(")");
			
	}
}
void prettyExpList(ExpList *list)
{
	if ( list == NULL || list->cur == NULL ) return;
	prettyExp(list->cur);
	if ( list->next != NULL )
	{
		printf(", ");
		prettyExpList(list->next);
	}

}
void prettyTypeHolder(TypeHolderNode *node, int indentLevel)
{
	if ( node == NULL ) return;
	switch (node->kind)
	{
		case sliceType:
			printf("[]");
			prettyTypeHolder(node -> underlyingType, indentLevel);
			break;
		case arrayType:
			printf("[%d]", node->arrayDims);
			prettyTypeHolder(node -> underlyingType, indentLevel);
			break;
		case identifierType:
			printf(" %s", node->identification);
			break;
		case structType:
			printf(" struct {\n");
			prettyStructMembers(node->structMembers, indentLevel + 1);
			indent(indentLevel);
			printf("}");
			break;
		case inferType:
			printf(" ");
			break;
	}
}
void prettyFuncDecl(FuncDeclNode *func, int indentLevel)
{
	if ( func == NULL ) return;
	indent(indentLevel);
	printf("func %s(", func->identifier);
	prettyFuncArgs(func->argsDecls, indentLevel);
	printf(")");
	if ( func->returnType != NULL )
	{
		prettyTypeHolder(func->returnType, indentLevel);
	}
	printf(" ");
	printStmt(func->blockStart, indentLevel );	

}
void prettyFuncArgs(VarDeclNode *args, int indentLevel)
{
	if ( args == NULL ) return;
	printf("%s", args->identifier);
	if ( args->typeThing != NULL )
	{
		prettyTypeHolder(args->typeThing, indentLevel);
	}
	if ( args->nextDecl != NULL )
	{
		printf(", ");
		prettyFuncArgs(args->nextDecl, indentLevel);
	}
}
void prettyStructMembers(VarDeclNode *members, int indentLevel)
{
	if ( members == NULL ) return;
	indent(indentLevel);
	printf("%s", members->identifier);
	if ( members->typeThing != NULL )
	{
		prettyTypeHolder(members->typeThing, indentLevel);
	}
	printf("\n");
	prettyStructMembers(members->nextDecl, indentLevel);
}
void prettyTypeDecl(TypeDeclNode *type, int indentLevel)
{
	if ( type == NULL ) return;
	indent(indentLevel);
	printf("type %s", type->identifier);
	prettyTypeHolder(type->actualType, indentLevel);
	printf("\n");
	prettyTypeDecl(type->nextDecl, indentLevel);
}
void prettyVarDecl(VarDeclNode *var, int indentLevel)
{
	if ( var == NULL ) return;
	indent(indentLevel);
	VarDeclNode *iter = var;
	while(iter -> nextDecl != NULL){
		printf("%s, ", iter->identifier);
		iter = iter -> nextDecl;
	}
	printf("%s", iter->identifier);
	
	prettyTypeHolder(var->typeThing, indentLevel);
	
	printf("= ");
	
	iter = var;
	while(iter -> nextDecl != NULL){
		prettyExp(iter->value);
		printf(", ");
		iter = iter -> nextDecl;
	}
	prettyExp(iter->value);
	printf("\n");
	prettyVarDecl(var -> multiDecl, indentLevel);
	
}
void prettyShortVarDecl(VarDeclNode *var, int indentLevel)
{
	indent(indentLevel);
	VarDeclNode *iter = var;
	while(iter -> nextDecl != NULL){
		printf("%s, ", iter->identifier);
		iter = iter -> nextDecl;
	}
	printf("%s := ", iter->identifier);
	iter = var;
	while(iter -> nextDecl != NULL){
		prettyExp(iter->value);
		printf(", ");
		iter = iter -> nextDecl;
	}
	prettyExp(iter->value);
	printf("\n");
}
void prettyTopDeclaration(TopDeclarationNode *topDecl, int indentLevel)
{
	if ( topDecl == NULL ) return;
	indent(indentLevel);
	switch (topDecl->declType)
	{
		case variDeclType:
			prettyVarDecl(topDecl->actualRealDeclaration.varDecl, indentLevel);
			break;
		case typeDeclType:
			if ( topDecl->actualRealDeclaration.typeDecl == NULL ) puts("something went wrong");
			prettyTypeDecl(topDecl->actualRealDeclaration.typeDecl, indentLevel);
			break;
		case funcDeclType:
			prettyFuncDecl(topDecl->actualRealDeclaration.funcDecl, indentLevel);
			break;
	}
	prettyTopDeclaration(topDecl->nextTopDecl, indentLevel);
}
void printRoot(RootNode *root)
{
	printf("package %s\n", root->packageName);
	prettyTopDeclaration(root->startDecls, 0);
}








//Neil (Needs a touch up)

void prettyVarDeclSimpleStatement(VarDeclNode *var)
{
	if ( var == NULL ) return;

	printf("%s", var->identifier);
	
	if ( var->value != NULL )
	{
		printf(" := ");
		prettyExp(var->value);
	}
	
}




void printPartiallyIndentedBlock(Stmt* stmt,int indentLevel){
	if (stmt == NULL){
		return;
	}
	printf("{\n");
	printStmt(stmt->val.block.stmt,indentLevel+1);
	indent(indentLevel);
	printf("}\n");

}


void printAssignmentStmt(Stmt* stmt){
		prettyExpList(stmt->val.assignment.lhs);
        printf(" = ");
        prettyExpList(stmt->val.assignment.rhs);
}



void printOpAssignmentStmt(Stmt* stmt){
	if (stmt == NULL){
		return;
	}

	prettyExp(stmt->val.opAssignment.lhs);

	switch (stmt->val.opAssignment.kind) {
				case expKindAddition:
					printf(" += " );
					break;
				case expKindSubtraction:
					printf(" -= " );
					break;
				case expKindMultiplication:
					printf(" *= " );
					break;
				case expKindDivision:
					printf(" /= " );
					break;
				case expKindMod:
					fputs(" %= ", stdout); //for denali <3
					break;
				case expKindBitAnd:
					printf(" &= " );
					break;
				case expKindBitOr:
					printf(" |= " );
					break;
				case expKindBitNotBinary:
					printf(" ^= " );
					break;
				case expKindBitShiftLeft:
					printf(" <<= " );
					break;
				case expKindBitShiftRight:
					printf(" >>= " );
					break;
				case expKindBitAndNot:
					printf(" &^= " );
					break;
			}

	prettyExp(stmt->val.opAssignment.rhs);

}

