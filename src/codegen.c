#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globalEnum.h"
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
// URGENT*** This method *needs* to get called after each struct type declaration 
// (and placed in the correct spot of the code i.e. outermost scope)

// Also I don't know how were are going to do this exactly yet, but this needs to occur:
// We need to *******generate******** a function 
// int structEquality(void * struct1, void * struct2, char * structName)
// which basically is a giant if else on the struct name that checks,
// if (structName == structTypeA), 
//     return structTypeA_equality((structTypeA *) struct1, (structTypeA *) struct2);
// for every struct equality we generate. This is necessary for array comparisons.
void generateStructEquality(TTEntry *structType, FILE *f)
{
    fprintf(f, "int %s_equality(%s *x, %s *y) {\n\treturn ", idGenJustType(structType), idGenJustType(structType), idGenJustType(structType));
    /* 
    int STRUCT_NAME_equality(STRUCT_NAME *x, STRUCT_NAME *y) {
        return 
    */
    IdChain *cur = structType->val.structType.fieldNames;
    Context *ctx = structType->val.structType.fields;
    while ( cur != NULL )
    {
        TTEntry *t = getEntry(ctx, cur->id)->entry.t; //Gets the type of the id
        char *fieldName = structMemb(cur->id);
        if ( strcmp(cur->id, "_") == 0 ) //we don't compare blanks
        {
            cur = cur->next;
            free(fieldName);
            continue;
        }
        switch (t->underlyingType)
        {
            case identifierType:
                switch ( t->val.nonCompositeType.base )
                {
                    case baseInt:
                    case baseBool:
                    case baseFloat64:
                    case baseRune:
                        fprintf(f, "x->%s == y->%s && ", fieldName, fieldName); //ez
                        break;
                    case baseString:
                        fprintf(f, "strcmp(x->%s, y->%s) == 0 && ", fieldName, fieldName);
                        break;
                }
                break;
            case structType:
                //this calls directly the struct equality, because we can mainly. Calling the
                // struct equality that switches on name works as well
                fprintf(f, "%s_equality(x->%s, y->%s) && ", idGenJustType(t), fieldName, fieldName);
                break;
            case arrType:
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                generateTypeChain(t->val.arrayType.type, typeChain);
                fprintf(f, "arrEquality(x->%s, y->%s, %s, %d) && ", fieldName, fieldName, typeChain, t->val.arrayType.size);
                free(typeChain);
                break;
        }
        free(fieldName);
        cur = cur->next;
    }
    fprintf(f, "1;\n}\n"); //Handles if all our fields are blank, trivially equal......
    /* && 1;
    }
    */
}
//The same deal as above...
//we need a void *structCopy(void *struct, char * structName), which
//acts identically to the above
//The function generated gets used in ensuring call by value.
//I don't think memcpy would work... If there was a field that was
//an array, for instance, it would copy by reference which is not
//desired. Also I couldn't figure out how to propagate the size of
//the struct
void generateStructCopy(TTEntry *structType, FILE *f)
{
    fprintf(f, "void* %s_copy(%s *x) {\n", idGenJustType(structType), idGenJustType(structType), idGenJustType(structType));
    fprintf(f, "\t%s *y = (%s *) malloc(sizeof(%s));\n", idGenJustType(structType), idGenJustType(structType), idGenJustType(structType));
    /* 
    void * STRUCT_NAME_copy(STRUCT_NAME *x) {
        STRUCT_NAME *y = (STRUCT_NAME *) malloc(sizeof(STRUCT_NAME));

    */
    IdChain *cur = structType->val.structType.fieldNames;
    Context *ctx = structType->val.structType.fields;
    while ( cur != NULL )
    {
        TTEntry *t = getEntry(ctx, cur->id)->entry.t; //Gets the type of the id
        char *fieldName = structMemb(cur->id);
        if ( strcmp(cur->id, "_") == 0 ) //we don't copy....
        {
            cur = cur->next;
            free(fieldName);
            continue;
        }
        switch (t->underlyingType)
        {
            case identifierType:
                switch ( t->val.nonCompositeType.base )
                {
                    case baseInt:
                    case baseBool:
                    case baseFloat64:
                    case baseRune:
                        fprintf(f, "\ty->%s = x->%s;\n", fieldName, fieldName);
                        break;
                    case baseString:
                        fprintf(f, "\tstrcpy(y->%s, x->%s);\n", fieldName, fieldName);
                        break;
                }
                break;
            case structType:
                fprintf(f, "\ty->%s = %s_copy(x->%s);\n", idGenJustType(t), fieldName, fieldName);
                break;
            case sliceType:
                fprintf(f, "\ty->%s = x->%s;\n", fieldName, fieldName);
                break;
            case arrType:
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                generateTypeChain(t->val.arrayType.type, typeChain);
                fprintf(f, "\ty->%s = arrCopy(x->%s, %s, %d);\n", fieldName, fieldName, typeChain, t->val.arrayType.size);
                free(typeChain);
                break;
        }
        free(fieldName);
        cur = cur->next;
    }
    fprintf(f, "\t return y;\n}"); 
}
//AY->Array, ST->Struct, SG->String, RE->rune, IR->Integer, FT->Float, 
//I don't think there's a better way to do this...
//Note: I intentionally don't add slices. This is because the ret val
//of this function is only used in arrayEquality, which would reject
//slices in equality in typechecking, and in arrCopy, but that doesn't
//matter since the reference gets copied.
void generateTypeChain(TTEntry *t, char *typeChain)
{
    switch (t->underlyingType)
    {
        case identifierType:
            switch (t->val.nonCompositeType.type)
            {
                case baseInt:
                case baseBool:
                    strcat(typeChain, "IR");
                    return;
                case baseRune:
                    strcat(typeChain, "RE");
                    return;
                case baseFloat64:
                    strcat(typeChain, "FT");
                    return;
                case baseString:
                    strcat(typeChain, "SG");
                    return;
            }
            break; //i know its redundant but they reassure me <3
        case arrayType:
            strcat(typeChain, "AY");
            char size[10];
            sprintf(size, "%d", t->val.arrayType.size);
            strcat(typeChain, size);
            generateTypeChain(t->val.arrayType.type, typeChain);
            break;
        case structType:
            strcat(typeChain, "ST");
            strcat(typeChain, idGenJustType(t));
            break;
    }
    return;
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
// These are pretty much duplicated because I wasn't sure if anyone 
// else needs a "pure" expression list generation... If noone else 
// needs it we can just remove it.
void funcExpListCodeGen(ExpList *list, int curScope, FILE *f)
{
    if ( list == NULL || list->cur == NULL ) return;
    TTEntry *type = getExpressionType(list->cur);

    //If you have var x [5][][5]int, we can stop copying at the slice
    
    switch ( type->baseType )
    {
        case identifierType: //normally c call b value
        case sliceType: //copies the pointer
            expCodeGen(list->cur, f); 
            break;
        case arrayType:
            char * typeChain = (char *) malloc(sizeof(char) * 999);
            strcpy(typeChain, "");
            generateTypeChain(type->val.arrayType.type, typeChain);
            fprintf(f, "arrayCopy(");
            expCodeGen(list->cur, f);
            fprintf(f, ", %s, %d)", typeChain, type->val.arrayType.size);
            free(typeChain);
        case structType:
            fprintf(f, "structCopy(");
            expCodeGen(list->cur, f); 
            fprintf(f, ", %s)", idGenJustType(t));
    }
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
			    funcExpListCodeGen(exp->val.funcCall.arguments, f);
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
                fprintf(f, "(")
                generateCast(exp->val.access.base->contextEntry->entry.t->val.sliceType.type);
                fprintf(f, "(sliceGet(");
			    expCodeGen(exp->val.access.base, f);
                fprintf(f, ", ");
			    expCodeGen(exp->val.access.accessor, f);
			    fprintf(f, ", %d)))", exp->lineno);
                break;
            }
            else {
                fprintf(f, "(")
                generateCast(exp->val.access.base->contextEntry->entry.t->val.arrayType.type);
                fprintf(f, "(arrGet(");
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
            //I don't think we need a cast........
            fprintf(f, "append(");
            expCodeGen(exp->val.append.list);
            fprintf(f, ", ");
            TTEntry *type = getExpressionType(exp->val.append.elem);
            if ( type->underlyingType == identifierType )
            {
                switch ( type->val.nonCompositeType.type )
                {
                    case baseBool:
                    case baseInt:
                        fprintf(f, "createPolyInt(");
                        break;
                    case baseFloat64:
                        fprintf(f, "createPolyFloat(");
                        break;
                    case baseRune:
                        fprintf(f, "createPolyRune(");
                        break;
                    case baseString:
                        fprintf(f, "createPolyString(");
                        break;
                }
            } else
            {
                fprintf(f, "createPolyVoid(");
                break;
            }
            expCodeGen(exp->val.append.elem);
            fprintf(f, "))");
			break;
		case expKindLength:;
            TTEntry *type = getExpressionType(e->val.builtInBody);
			switch ( type->underlyingType )
            {
                case arrayType:
                    fprintf(f, "%d", type->val.arrayType.size); //cheeky
                    break;
                case sliceType:
                    fprintf(f, "(");
                    expCodeGen(e->val.builtInBody, f);
                    fprintf(f, ")->size");
                    break;
                case identifierType:
                    fprintf(f, "strlen(");
                    expCodeGen(e->val.builtInBody, f);
                    fprintf(f, ")");
                    break;
            }
			break;
		case expKindCapacity:;
			TEntry *type = getExpressionType(e->val.builtInBody);
			switch ( type->underlyingType )
            {
                case arrayType:
                    fprintf(f, "%d", type->val.arrayType.size);
                    break;
                case sliceType:
                    fprintf(f, "(");
                    expCodeGen(e->val.builtInBody, f);
                    fprintf(f, ")->capacity");
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
					    standardBinaryGen(exp, f, "+");
                    }
                    break;
				case expKindSubtraction:
                    standardBinaryGen(exp, f, "-");
					break;
				case expKindMultiplication:
                    standardBinaryGen(exp, f, "*");
					break;
				case expKindDivision:
                    standardBinaryGen(exp, f, "/");
					break;
				case expKindLogicOr:
                    standardBinaryGen(exp, f, "||");
					break;
				case expKindLogicAnd:
                    standardBinaryGen(exp, f, "&&");
					break;
				case expKindNEQ:
					TEntry *type = getExpressionType(e->val.left);
                    switch ( type->underlyingType )
                    {
                        case identifierType:
                            switch ( type->val.nonCompositeType.type )
                            {
                                case baseInt:
                                case baseBool:
                                case baseFloat64:
                                case baseRune:
                                    expCodeGen(exp->val.binary.left, f);
                                    fprintf(f, " != " );
                                    expCodeGen(exp->val.binary.right, f);
                                    break;
                                case baseString:
                                    fprintf(f, "strcmp(");
                                    expCodeGen(exp->val.binary.left, f);
                                    fprintf(f, ", ");
                                    expCodeGen(exp->val.binary.right, f);
                                    fprintf(f, ") != 0");
                                    break;
                            }
                            break;
                        case structType:
                            fprintf(f, "!%s_equality(", idGenJustType(type));
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ")");
                            break;
                        case arrayType:
                            fprintf(f, "!arrEquality(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            char * typeChain = (char *) malloc(sizeof(char) * 999);
                            strcpy(typeChain, "");
                            generateTypeChain(type->val.arrayType.type, typeChain);
                            fprintf(f, "%s, %d)", typeChain, type->val.arrayType.size);
                            free(typeChain);
                            break;
                    }
					break;
				case expKindEQ:;
                    TEntry *type = getExpressionType(e->val.left);
                    switch ( type->underlyingType )
                    {
                        case identifierType:
                            switch ( type->val.nonCompositeType.type )
                            {
                                case baseInt:
                                case baseBool:
                                case baseFloat64:
                                case baseRune:
                                    expCodeGen(exp->val.binary.left, f);
                                    fprintf(f, " == " );
                                    expCodeGen(exp->val.binary.right, f);
                                    break;
                                case baseString:
                                    fprintf(f, "strcmp(");
                                    expCodeGen(exp->val.binary.left, f);
                                    fprintf(f, ", ");
                                    expCodeGen(exp->val.binary.right, f);
                                    fprintf(f, ") == 0");
                                    break;
                            }
                            break;
                        case structType:
                            fprintf(f, "%s_equality(", idGenJustType(type));
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ")");
                            break;
                        case arrayType:
                            fprintf(f, "arrEquality(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            char * typeChain = (char *) malloc(sizeof(char) * 999);
                            strcpy(typeChain, "");
                            generateTypeChain(type->val.arrayType.type, typeChain);
                            fprintf(f, "%s, %d)", typeChain, type->val.arrayType.size);
                            free(typeChain);
                            break;
                    }
					break;
                case expKindLEQ:
                    orderedBinaryGen(exp, f, "<=");
					break;
				case expKindGEQ:
                    orderedBinaryGen(exp, f, ">=");
					break;
				case expKindLess:
                    orderedBinaryGen(exp, f, "<");
					break;
				case expKindGreater:
                    orderedBinaryGen(exp, f, ">");
					break;
				case expKindMod:
                    standardBinaryGen(exp, f, "%%");
					break;
				case expKindBitAnd:
                    standardBinaryGen(exp, f, "&");
					break;
				case expKindBitOr:
                    standardBinaryGen(exp, f, "|");
					break;
				case expKindBitNotBinary: //lol
                    standardBinaryGen(exp, f, "^");
					break;
				case expKindBitShiftLeft:
                    standardBinaryGen(exp, f, "<<");
					break;
				case expKindBitShiftRight:
                    standardBinaryGen(exp, f, ">>");
					break;
				case expKindBitAndNot:
                    standardBinaryGen(exp, f, "&~");
					break;
			}
			fprintf(f, ")");
			
	}
}
void standardBinaryGen(Exp *exp, FILE *f, char *op)
{
    expCodeGen(exp->val.binary.left, f);
	fprintf(f, " %s ", op );
	expCodeGen(exp->val.binary.right, f);
}
void orderedBinaryGen(Exp *exp, FILE *f, char *op)
{
    TEntry *type = getExpressionType(exp->val.left)
    switch ( type->val.nonCompositeType.type )
    {
        case baseInt:
        case baseBool:
        case baseFloat64:
        case baseRune:
            expCodeGen(exp->val.binary.left, f);
            fprintf(f, " %s ", op);
            expCodeGen(exp->val.binary.right, f);
            break;
        case baseString:
            fprintf(f, "strcmp(");
            expCodeGen(exp->val.binary.left, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.binary.right, f);
            fprintf(f, ") %s 0", op);
            break;
    }
}