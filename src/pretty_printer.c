
#include <stdlib.h>
#include <stdio.h>
#include "globalEnum.h"
#include <string.h>
#include "ast.h"



void prettyExpList(ExpList *list);
void prettyExp(Exp * exp);

void printIfStmt(Stmt* stmt,int indentLevel);
void printSwitchStmt(Stmt* stmt,int indentLevel);

void printSimpleStatement(Stmt* stmt);
void printSwitchCaseClause(switchCaseClause* clause,int indentLevel);

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
        case StmtKindBlock :    indent(indentLevel);
								printf("{\n");
                                printStmt(stmt->val.block.stmt,indentLevel + 1);
                                indent(indentLevel);
                                printf("}\n");
                                break;

        case StmtKindExpression : 	indent(indentLevel);
									printSimpleStatement(stmt);
                                    printf("\n");
                                    break;
                                    
        case StmtKindAssignment : indent(indentLevel);
									printSimpleStatement(stmt);
                                    printf("\n");
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

							Stmt* blockStmt = stmt->val.elseStmt.block;

							printf("{\n\n");
	
                            printStmt(blockStmt->val.block.stmt,indentLevel+1);
							indent(indentLevel);
							printf("}\n\n");
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
                                printStmt(stmt->val.infLoop.block,indentLevel);
                                break;

        case StmtKindWhileLoop: indent(indentLevel);
								printf("for ");
                                prettyExp(stmt->val.whileLoop.conditon);
                                printStmt(stmt->val.whileLoop.block,indentLevel);
                                break;
        case StmtKindThreePartLoop:  indent(indentLevel);
									printf("for ");
                                    printSimpleStatement(stmt->val.forLoop.init);
                                    printf(";");
                                    prettyExp(stmt->val.forLoop.condition);
                                    printf(";");
                                    printSimpleStatement(stmt->val.forLoop.inc);

                                    printStmt(stmt->val.forLoop.block,indentLevel);

                                    break;

        case StmtKindBreak:     indent(indentLevel);
								printf("break;");
                                break;
        case StmtKindContinue: indent(indentLevel);
								printf("continue;");
                                break;
        case StmtKindFallthrough: 
								indent(indentLevel);
                                printf("fallthrough;");
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
									
                                    printf(";");
                                    break;
                                    
        case StmtKindAssignment : prettyExpList(stmt->val.assignment.lhs);
                                    printf(" = ");
                                    prettyExpList(stmt->val.assignment.rhs);
                                    printf(";");
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


























void prettyExpList(ExpList *list);
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

		case expKindTypeCast:

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
					fputs(" % ", stdout);
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
				case expKindLogicNot:
					printf("!");
					prettyExp(exp->val.unary);
					break;
				case expKindUnaryMinus:
					printf("-");
					prettyExp(exp->val.unary);
					break;
				case expKindUnaryPlus:
					printf("+");
					prettyExp(exp->val.unary);
					break;
				case expKindBitNotUnary:
					printf("^");
					prettyExp(exp->val.unary);
					break;
				default:
					printf("An error occured :(");
					exit(1);
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
