#include <stdio.h>
#include "ast.h"
#include "symbol_table.h"

UniqueId * idTable[TABLE_SIZE];
int initCount = 0;
int tempVarCount = 0;

typedef struct UniqueId UniqueId;
struct UniqueId {
    void * pointerAddress;
    UniqueId *next;
};

char *tmpVarGen()
{
    char *retVal = (char *) malloc(sizeof(char) * 30);
    sprintf(retVal, "__golite_temp_%d", tempVarCount++);
    return retVal;
}
char *enterInTable(char *id, void * pointer)
{
    int hash = hashCode(id);
    int count = 0;
    UniqueId *cur = idTable[hash];

    if ( cur == NULL )
    {
        UniqueId *entry = (UniqueId *) malloc(sizeof(UniqueId));
        entry->pointerAddress = pointer;
        entry->next = NULL;
        idTable[hash] = entry;

        retVal = (char *) malloc(sizeof(char) * (50 + strlen(id));
        sprintf(retVal, "__golite_decl_%s_%d", id, count);
        return retVal;
    }

    while ( cur->next != NULL )
    {
        if ( cur->pointerAddress == pointer )
        {
            retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
            sprintf(retVal, "__golite_decl_%s_%d",id, count);
            return retVal;
        }
        count++;
        cur = cur->next;
    }

    if ( cur->pointerAddress == pointer )
    {
        retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
        sprintf(retVal, "__golite_decl_%s_%d",id, count);
        return retVal;
    }

    UniqueId *entry = (UniqueId *) malloc(sizeof(UniqueId));
    entry->pointerAddress = pointer;
    entry->next = NULL;
    cur->next = entry;

    retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
    sprintf(retVal, "__golite_decl_%s_%d",id, count + 1);
    return retVal;
}
char *idGen(PolymorphicEntry *e) //creates and/or returns the "correct" id
//This should be able to be used for functions, struct types, and variable ids.
{
    char *id, *retVal;
    if ( e->isSymbol ) {
        id = e->entry.s->id;
        if ( e->entry.s.isConstant == 2 && strcmp(id, "main") == 0 )
        {
           return strcpy(retVal, "__golite_main");
        }
        //generates init names in declaration order
        //so in the generated main, while (i := 0) < initCount, generate __golite_init_i(), then generate __golite_main
        else if ( e->entry.s.isConstant == 2 && strcmp(id, "init") == 0 )
        {
            retVal = (char *) malloc(sizeof(char) * 20);
            sprintf(retVal, "__golite_init_%d", initCount++); 
            return retVal;
        }
        else if ( e->entry.s.isConstant == 1 ) //an unshadowed boolean
        {
            return strcpy(retVal, id); //either "true" or "false". If we import stdbool this works
        }
        // this check can potentially be moved in the generation of declarations
        else if ( strcmp(id, "_") == 0 ) return tmpVarGen(); 
        else return enterInTable(e->entry.s->id, e->entry.s);
    }
    else return enterInTable(e->entry.t->id, e->entry.t);
}
char *idGenJustType(TTEntry *t) //used for structs
{
    return enterInTable(t->id, t);
}
char *structMemb(char *memb)
{
    //caution will have to be expressed when dealing with blank fields...
    //in particular with equality ( i think tentatively )
    if ( strcmp(memb, "_") == 0 ) return tmpVarGen();

    char * retVal = (char *) malloc(sizeof(char) * (50 + strlen(memb)));
    sprintf(retVal, "__golite_structmemb_%s", memb);
    return retVal;
}
void rawStringCodeGen(char *s, FILE *f)
{
    fprintf(f, "\"");
    for (int i = 1; i < strlen(s) - 1; i++)
    {
        fputc(*(s + i), f);
        if ( *(s + i) == '\\' ) //ugh
        {
            fputc('\\', f);
        } else if ( *(s + i) == '\n' )
        {
            fputc('\t', f);
        }
    }
    fprintf(f, "\"");
}
void generateCast(TTEntry *t, FILE *f)
{
    if ( t->underlyingType == arrayType )
    {
        fprintf(f, "(__golite_poly_entry *)"); return;
    } else if ( t->underlyingType == sliceType )
    {
        //TODO
    } else if ( t->underlyingType == structType )
    {
        fprintf(f, "(%s *)", idGenJustType(t));
    } else if ( t->underlyingType == identifierType )
    {
        switch ( t->val.nonCompositeType.type )
        {
            case baseBool:
            case baseInt: fprintf(f, "(int)"); return;
            case baseFloat64: fprintf(f, "(double)"); return;
            case baseRune: fprintf(f, "(char)"); return;
            case baseString: fprintf(f, "(char *)"); return;
        }
    } else {
        fprintf(stderr, "There is a buggggg fix it (generate cast)");
        exit(1);
    }
}
void expListCodeGen(ExpList *list, int curScope, FILE *f)
{
	if ( list == NULL || list->cur == NULL ) return;
	expCodeGen(list->cur, f);
	if ( list->next != NULL )
	{
		fprintf(f, ", ");
		expListCodeGen(list->next, f);
	}

}
void expCodeGen(Exp *exp, FILE *f)
{
    if ( exp == NULL ) return;
	switch ( exp->kind )
	{
		case expKindIdentifier:
            if ( e->contextEntry->entry.s->isConstant == 1 ) //is true or false, no shadowing...
            {
                fprintf(f, "%s", exp->val.id); //use stdbool!!!!
            } else
            {
                char *retVal = idGen(exp->contextEntry);
                fprintf(f, "%s",  retVal));
                free(retVal);
            }
            break;
		case expKindIntLit:
			fprintf(f, "%d", exp->val.intLit);
			break;
		case expKindFloatLit:
			fprintf(f, "%lf", exp->val.floatLit);
			break;
		case expKindRuneLit: //might need to take special care idk
			fprintf(f, "%s", exp->val.runeLit);
			break;
		case expKindRawStringLit:
            rawStringCodeGen(exp->val.stringLit, f);
            break;
		case expKindInterpretedStringLit:
			fprintf(f, "%s", exp->val.stringLit);
			break;
		case expKindFuncCall:
			if (exp->val.funcCall.base->contextEntry->isSymbol) //true function call not a type cast
            {
                fprintf(idGen(exp->val.funcCall.base->contextEntry));
                fprintf(f, "(");
			    expListCodeGen(exp->val.funcCall.arguments, f);
			    fprintf(f, ")");
            } else //type cast
            {
                if ( exp->val.funcCall.base->contextEntry->entry.type->val.nonCompositeType.type == baseString )
                {
                    fprintf(f, "stringCast("); //helper function in templateCode.h
                    expListCodeGen(exp->val.funcCall.arguments);
                    fprintf(f, ")");
                } else //trust c's implicit cast. Since we only do this on non composite types we should be good
                {
                    expListCodeGen(exp->val.funcCall.arguments);
                }
            }
			break;
		case expKindIndexing:
            if ( exp->val.access.base->contextEntry->entry.t->underlyingType == sliceType )
            {
                    //TODO
            }
            else {
                fprintf(f, "(")
                generateCast(exp->val.access.base->contextEntry->entry.t->val.arrayType.type);
                fprintf(f, "(");
                fprintf(f, "arrGet(");
			    expCodeGen(exp->val.access.base, f);
                fprintf(f, ", ");
			    expCodeGen(exp->val.access.accessor, f);
                fprintf(f, ", ")
                fprintf(f, exp->val.access.base->contextEntry->entry.t->val.arrayType.size);
			    fprintf(f, ", %d)))", exp->lineno);
                break;
            }
			break;
		case expKindFieldSelect:
			expCodeGen(exp->val.access.base, f);
			fprintf(f, ".");
            char * retVal = structMemb(exp->val.access.accessor->val.id);
			fprintf(f, "%s", retVal);
            free(retVal);
			break;
		case expKindAppend:
            //TODO
			break;
		case expKindLength:;
            TTEntry *type = getExpressionType(e->val.builtInBody);
			switch(type->underlyingType)
            {
                case arrayType:
                    fprintf(f, "%d", type->val.arrayType.size);
                    break;
                case sliceType:
                    //TODO
                    break;
                case identifierType:
                    fprintf(f, "strlen(");
                    expCodeGen(e->val.builtInBody, f);
                    fprintf(f, ")");
                    break;
            }
			break;
		case expKindCapacity:;
			TEntry *type = typeCheckExpression(e->val.builtInBody);
			switch(type->underlyingType)
            {
                case arrayType:
                    fprintf(f, "%d", type->val.arrayType.size);
                    break;
                case sliceType:
                    //TODO
                    break;
		case expKindLogicNot: //switch statements <3
		case expKindUnaryMinus:
		case expKindUnaryPlus:
		case expKindBitNotUnary:
			switch (exp->kind) {
				case expKindLogicNot:
					fprintf(f, "!");
					break;
				case expKindUnaryMinus:
					fprintf(f, "-");
					break;
				case expKindUnaryPlus:
					fprintf(f, "+");
					break;
				case expKindBitNotUnary:
					fprintf(f, "~");
					break;
			}
			fprintf(f, "(");
			expCodeGen(exp->val.unary, f);
			fprintf(f, ")");
			break;
		default:
			fprintf(f, "(");
			switch (exp->kind) {
				case expKindAddition:;
                    TEntry *type = getExpressionType(e->val.left);
                    if (type->val.nonCompositeType.type == baseString)
                    {
                        fprintf(f, "concat(");
                        expCodeGen(exp->val.binary.left, f);
                        fprintf(f, ", ");
                        expCodeGen(exp->val.binary.right, f);
                        fprintf(f, ");");  
                    }
                    else {
					    expCodeGen(exp->val.binary.left, f);
					    fprintf(f, " + " );
					    expCodeGen(exp->val.binary.right, f);
                    }
                    break;
				case expKindSubtraction:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " - " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindMultiplication:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " * " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindDivision:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " / " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindLogicOr:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " || " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindLogicAnd:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " && " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindLEQ:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " <= " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindGEQ:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " >= " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindNEQ:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " != " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindEQ:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " == " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindLess:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " < " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindGreater:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " > " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindMod:
					expCodeGen(exp->val.binary.left, f);
					fputs(" % ", stdout); //for denali <3
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitAnd:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " & " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitOr:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " | " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitNotBinary:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " ^ " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitShiftLeft:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " << " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitShiftRight:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " >> " );
					expCodeGen(exp->val.binary.right, f);
					break;
				case expKindBitAndNot:
					expCodeGen(exp->val.binary.left, f);
					fprintf(f, " &~ " );
					expCodeGen(exp->val.binary.right, f);
					break;
			}
			fprintf(f, ")");
			
	}
}