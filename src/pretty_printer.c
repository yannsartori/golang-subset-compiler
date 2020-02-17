#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "globalEnum.h"

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
