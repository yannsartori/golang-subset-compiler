#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globalEnum.h"
#include "ast.h"
#include "symbol_table.h"
#include "type_check.h"

void generateOurTypes(TTEntry *t, FILE *f);
void codegenStructDeclaration(int indentLevel,FILE* fp);
int hashCode(char * id); //symbol_table.c
TTEntry *getExpressionType(Exp *e); //type_check.c
void expCodeGen(Exp *exp, FILE *f);
void stmtCodeGen(Stmt* stmt,int indentLevel, FILE* fp);

void halt(){
    puts("Here");
    exit(1);
}

typedef struct UniqueId UniqueId;
struct UniqueId {
    void * pointerAddress;
    UniqueId *next;
};

UniqueId * idTable[TABLE_SIZE];
int initCount = 0;
int tempVarCount = 0;
int labelCount = 0;




char *tmpVarGen()

{
    char *retVal = (char *) malloc(sizeof(char) * 30);
    sprintf(retVal, "__golite_temp_%d", tempVarCount++);
    return retVal;
}

char *enterInTable(char *id, void * pointer) /* Puts id in the UniqueId table. This is a helper funciton. */
{
    int hash = hashCode(id);
    int count = 0;
    UniqueId *cur = idTable[hash];
    char* retVal;

    if ( cur == NULL )
    {
        UniqueId *entry = (UniqueId *) malloc(sizeof(UniqueId));
        entry->pointerAddress = pointer;
        entry->next = NULL;
        idTable[hash] = entry;

        retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
        sprintf(retVal, "__golite_decl_%s_%d", id, count);
        return retVal;
    }

    while ( cur->next != NULL )
    {
        if ( cur->pointerAddress == pointer )
        {
            retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
            sprintf(retVal, "__golite_decl_%s_%d", id, count);
            return retVal;
        }
        count++;
        cur = cur->next;
    }

    if ( cur->pointerAddress == pointer )
    {
        retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
        sprintf(retVal, "__golite_decl_%s_%d", id, count);
        return retVal;
    }

    UniqueId *entry = (UniqueId *) malloc(sizeof(UniqueId));
    entry->pointerAddress = pointer;
    entry->next = NULL;
    cur->next = entry;

    retVal = (char *) malloc(sizeof(char) * (50 + strlen(id)));
    sprintf(retVal, "__golite_decl_%s_%d", id, count + 1);
    return retVal;
}
char *idGen(PolymorphicEntry *e) //creates and/or returns the "correct" id
//This should be able to be used for functions, struct types, and variable ids.
{  
    char *id, *retVal;
    if ( e->isSymbol ) {
        id = e->entry.s->id;
        if ( e->entry.s->isConstant == 2 && strcmp(id, "main") == 0 )
        {
           retVal = (char *) malloc(sizeof(char) * 20);
           strcpy(retVal, "__golite_main");
           return retVal;
        }
        //generates init names in declaration order
        //so in the generated main, while (i := 0) < initCount, generate __golite_init_i(), then generate __golite_main
        else if ( e->entry.s->isConstant == 2 && strcmp(id, "init") == 0 )
        {
            retVal = (char *) malloc(sizeof(char) * 20);
            sprintf(retVal, "__golite_init_%d", initCount++); 
            return retVal;
        }
        else if ( e->entry.s->isConstant == 1 ) //an unshadowed boolean
        {
            retVal = (char *) malloc(sizeof(char) * 10);
            strcpy(retVal, id); //either "true" or "false". If we import stdbool this works
            return retVal;
        }
        // this check can potentially be moved in the generation of declarations
        else if ( strcmp(id, "_") == 0 ) return tmpVarGen(); 
        else return enterInTable(e->entry.s->id, e->entry.s);
    }
    else return enterInTable(e->entry.t->id, e->entry.t);
}
char *idGenJustType(TTEntry *t) //used for structs(TODO)
{

    char* id = malloc(sizeof(char)*50);
            
    sprintf(id,"__struct_variant__%d__",LookUpLabel(t));
    return id;
}

char* idGenJustVar(STEntry* s) 
{

	char* retVal;
	if ( s -> isConstant == 2 && strcmp(s -> id, "main") == 0 )
	{
        retVal = (char *) malloc(sizeof(char) * 20);     
		strcpy(retVal, "__golite_main");
        return retVal;

	}
	//generates init names in declaration order
	//so in the generated main, while (i := 0) < initCount, generate __golite_init_i(), then generate __golite_main
	else if ( s -> isConstant == 2 && strcmp(s -> id, "init") == 0 )
	{
		retVal = (char *) malloc(sizeof(char) * 20);
		sprintf(retVal, "__golite_init_%d", initCount++); 
		return retVal;
	}
	else if ( strcmp(s -> id, "_") == 0 ) 
	{
		return tmpVarGen(); 
	}
	else return enterInTable(s->id, s);
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

void standardBinaryGen(Exp *exp, const char *op, FILE *f)
{
    expCodeGen(exp->val.binary.left, f);
	fprintf(f, " %s ", op );
	expCodeGen(exp->val.binary.right, f);
}
void orderedBinaryGen(Exp *exp, const char *op, FILE *f)
{
    TTEntry *type = getExpressionType(exp->val.binary.left);
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
void generateCast(TTEntry *t, FILE *f)
{
    if ( t->underlyingType == arrayType )
    {
        fprintf(f, "(__golite_poly_entry *)");
    } else if ( t->underlyingType == sliceType )
    {
        fprintf(f, "(__golite_slice *)");
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
void generateUnionAccess(TTEntry *t, FILE *f)
{
    if ( t->underlyingType == identifierType )
    {
        switch ( t->val.nonCompositeType.type )
        {
            case baseBool:
            case baseInt: fprintf(f, ".intVal"); return;
            case baseFloat64: fprintf(f, ".floatVal"); return;
            case baseRune: fprintf(f, ".runeVal"); return;
            case baseString: fprintf(f, ".stringVal"); return;
        }
    }
    else
    {
            fprintf(f, ".polyVal"); 
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
void generateStructEquality(TTEntry *sType, FILE *f)
{
    fprintf(f, "int %s_equality(%s *x, %s *y) {\n\treturn ", idGenJustType(sType), idGenJustType(sType), idGenJustType(sType));
    /* 
    int STRUCT_NAME_equality(STRUCT_NAME *x, STRUCT_NAME *y) {
        return 
    */
    IdChain *cur = sType->val.structType.fieldNames;
    Context *ctx = sType->val.structType.fields;
    while ( cur != NULL )
    {
        TTEntry *t = getEntry(ctx, cur->identifier)->entry.t; //Gets the type of the id
        char *fieldName = structMemb(cur->identifier);
        if ( strcmp(cur->identifier, "_") == 0 ) //we don't compare blanks
        {
            cur = cur->next;
            free(fieldName);
            continue;
        }
        switch ( t->underlyingType )
        {
            case identifierType:
                switch ( t->val.nonCompositeType.type )
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
            case arrayType:
                if ( 0 ) break;
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                strcpy(typeChain, "");
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
void generateStructCopy(TTEntry *structType_, FILE *f)
{
    fprintf(f, "void* %s_copy(%s *x) {\n", idGenJustType(structType_), idGenJustType(structType_));
    fprintf(f, "\t%s *y = (%s *) malloc(sizeof(%s));\n", idGenJustType(structType_), idGenJustType(structType_), idGenJustType(structType_));
    /* 
    void * STRUCT_NAME_copy(STRUCT_NAME *x) {
        STRUCT_NAME *y = (STRUCT_NAME *) malloc(sizeof(STRUCT_NAME));
    */
    IdChain *cur = structType_->val.structType.fieldNames;
    Context *ctx = structType_->val.structType.fields;
    while ( cur != NULL )
    {
        TTEntry *t = getEntry(ctx, cur->identifier)->entry.t; //Gets the type of the id
        char *fieldName = structMemb(cur->identifier);
        if ( strcmp(cur->identifier, "_") == 0 ) //we don't copy....
        {
            cur = cur->next;
            free(fieldName);
            continue;
        }
        switch (t->underlyingType)
        {
            case identifierType:
                switch ( t->val.nonCompositeType.type)
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
            case arrayType:
                if ( 0 ) break;
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                strcpy(typeChain, "");
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
/*
void expListCodeGen(ExpList *list, FILE *f)
{
	if ( list == NULL || list->cur == NULL ) return;
	expCodeGen(list->cur, f);
	if ( list->next != NULL )
	{
		fprintf(f, ", ");
		//expListCodeGen(list->next, f);
	}

}
*/
// These are pretty much duplicated because I wasn't sure if anyone 
// else needs a "pure" expression list generation... If noone else 
// needs it we can just remove it.
void funcExpListCodeGen(ExpList *list, FILE *f)
{
    if ( list == NULL || list->cur == NULL ) return;
    TTEntry *type = getExpressionType(list->cur);

    //If you have var x [5][][5]int, we can stop copying at the slice
    
    switch ( type->underlyingType )
    {
        case identifierType: //normally c call b value
        case sliceType: //copies the pointer
            expCodeGen(list->cur, f); 
            break;
        case arrayType:
            if ( 0 ) break;
            char * typeChain = (char *) malloc(sizeof(char) * 999);
            strcpy(typeChain, "");
            generateTypeChain(type->val.arrayType.type, typeChain);
            fprintf(f, "arrayCopy(");
            expCodeGen(list->cur, f);
            fprintf(f, ", \"%s\", %d)", typeChain, type->val.arrayType.size);
            free(typeChain);
            break;
        case structType:
            fprintf(f, "structCopy(");
            expCodeGen(list->cur, f); 
            fprintf(f, ", \"%s\")", idGenJustType(type));
    }
	if ( list->next != NULL )
	{
		fprintf(f, ", ");
		funcExpListCodeGen(list->next, f);
	}
}
char * expKindToString(ExpressionKind e);
void expCodeGen(Exp *exp, FILE *f)
{
    TTEntry *type;
    if ( exp == NULL ) return;
	if ( exp->kind == expKindIdentifier )
	{
        char *retVal = idGen(exp->contextEntry);
        fprintf(f, "%s",  retVal);
        free(retVal);
        return;
    }
	else if ( exp->kind == expKindIntLit )
    {
        fprintf(f, "%d", exp->val.intLit);
        return;
    }
	else if ( exp->kind == expKindFloatLit )
    {
        fprintf(f, "%lf", exp->val.floatLit);
        return;
    }
    else if ( exp->kind == expKindRuneLit ) //test
    {
        fprintf(f, "%s", exp->val.runeLit);
        return;
    }
	else if ( exp->kind == expKindRawStringLit )
    {
        rawStringCodeGen(exp->val.stringLit, f);
        return;
    }
	else if ( exp->kind == expKindInterpretedStringLit )
    {
        fprintf(f, "%s", exp->val.stringLit);
        return;
    }
	else if ( exp->kind == expKindFuncCall )
    {
        if ( exp->val.funcCall.base->contextEntry->isSymbol ) //true function call not a type cast
        {
            fprintf(f, "%s", idGen(exp->val.funcCall.base->contextEntry));
            fprintf(f, "(");
            funcExpListCodeGen(exp->val.funcCall.arguments, f);
            fprintf(f, ")");
        } else //type cast
        {
            if ( getExpressionType(exp->val.funcCall.base)->val.nonCompositeType.type == baseString )
            {
                fprintf(f, "stringCast("); //helper function in templateCode.h
                funcExpListCodeGen(exp->val.funcCall.arguments, f); //maybeExpList
                fprintf(f, ")");
            } else //trust c's implicit cast. Since we only do this on non composite types we should be good
            {
                funcExpListCodeGen(exp->val.funcCall.arguments, f); //maybeExpList
            }
        }
        return;
    }
	else if ( exp->kind == expKindIndexing )
    {
        if ( getExpressionType(exp->val.access.base)->underlyingType == sliceType )
        {
            fprintf(f, "(");
            generateCast(getExpressionType(exp->val.access.base)->val.sliceType.type, f);
            fprintf(f, "(sliceGet(");
            expCodeGen(exp->val.access.base, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.access.accessor, f);
            fprintf(f, ", %d))", exp->lineno);
            generateUnionAccess(getExpressionType(exp->val.access.base)->val.sliceType.type, f);
            fprintf(f, ")");
        }
        else {
            fprintf(f, "(");
            generateCast(getExpressionType(exp->val.access.base)->val.arrayType.type, f);
            fprintf(f, "(arrGet(");
            expCodeGen(exp->val.access.base, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.access.accessor, f);
            fprintf(f, ", %d, %d))", getExpressionType(exp->val.access.base)->val.arrayType.size, exp->lineno);
            generateUnionAccess(getExpressionType(exp->val.access.base)->val.arrayType.type, f);
            fprintf(f, ")");
        }
        return;
    }
	else if ( exp->kind == expKindFieldSelect )
    {
        fprintf(f, "(");
        generateCast(getExpressionType(exp->val.access.base), f);
        expCodeGen(exp->val.access.base, f);
        fprintf(f, ")->");
        char * retVal = structMemb(exp->val.access.accessor->val.id);
        fprintf(f, "%s", retVal);
        free(retVal);
        return;
    }
	else if ( exp->kind == expKindAppend )
    {
        fprintf(f, "append(");
        expCodeGen(exp->val.append.list, f);
        fprintf(f, ", ");
        type = getExpressionType(exp->val.append.elem);
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
        }
        expCodeGen(exp->val.append.elem, f);
        fprintf(f, "))");
        return;
    }
	else if ( exp->kind == expKindLength )
    {
        type = getExpressionType(exp->val.builtInBody);
        switch ( type->underlyingType )
        {
            case arrayType:
                fprintf(f, "%d", type->val.arrayType.size); //cheeky
                break;
            case sliceType:
                fprintf(f, "(");
                expCodeGen(exp->val.builtInBody, f);
                fprintf(f, ")->size");
                break;
            case identifierType:
                fprintf(f, "strlen(");
                expCodeGen(exp->val.builtInBody, f);
                fprintf(f, ")");
                break;
        }
        return;
    }
	else if ( exp->kind == expKindCapacity )
    {
        type = getExpressionType(exp->val.builtInBody);
        switch ( type->underlyingType )
        {
            case arrayType:
                fprintf(f, "%d", type->val.arrayType.size);
                break;
            case sliceType:
                fprintf(f, "(");
                expCodeGen(exp->val.builtInBody, f);
                fprintf(f, ")->capacity");
                break;
        }
        return;
    }
	else if ( exp->kind == expKindLogicNot )
    { 
        fprintf(f, "!");
        fprintf(f, "(");
        expCodeGen(exp->val.unary, f);
        fprintf(f, ")");
        return;
    }
	else if ( exp->kind == expKindUnaryMinus )
    {
        fprintf(f, "-");
        fprintf(f, "(");
        expCodeGen(exp->val.unary, f);
        fprintf(f, ")");
        return;
    }
	else if ( exp->kind == expKindUnaryPlus )
    {
        fprintf(f, "+");
        fprintf(f, "(");
        expCodeGen(exp->val.unary, f);
        fprintf(f, ")");
        return;
    }
	else if ( exp->kind == expKindBitNotUnary )
    {
        fprintf(f, "~");
        fprintf(f, "(");
        expCodeGen(exp->val.unary, f);
        fprintf(f, ")");
		return;
    }


    fprintf(f, "(");
    if ( exp->kind == expKindAddition )
    {
        type = getExpressionType(exp->val.binary.left);
        if (type->val.nonCompositeType.type == baseString)
        {
            fprintf(f, "concat(");
            expCodeGen(exp->val.binary.left, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.binary.right, f);
            fprintf(f, ")");  
        }
        else {
            standardBinaryGen(exp, "+", f);
        }
    }
    else if ( exp->kind == expKindSubtraction )
        standardBinaryGen(exp, "-", f);
    else if ( exp->kind == expKindMultiplication )
        standardBinaryGen(exp, "*", f);
    else if ( exp->kind == expKindDivision )
        standardBinaryGen(exp, "/", f);
    else if ( exp->kind == expKindLogicOr )
        standardBinaryGen(exp, "||", f);
    else if ( exp->kind == expKindLogicAnd )
        standardBinaryGen(exp, "&&", f);
    else if ( exp->kind == expKindNEQ )
    {
        type = getExpressionType(exp->val.binary.left);
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
                fprintf(f, "\"%s\", %d)", typeChain, type->val.arrayType.size);
                free(typeChain);
                break;
        }
    }
    else if ( exp->kind == expKindEQ )
    {
        type = getExpressionType(exp->val.binary.left);
    
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
                fprintf(f, ", \"%s\", %d)", typeChain, type->val.arrayType.size);
                free(typeChain);
                break;
        }
    }
    else if ( exp->kind == expKindLEQ )
        orderedBinaryGen(exp, "<=", f);
    else if ( exp->kind == expKindGEQ )
        orderedBinaryGen(exp, ">=", f);
    else if ( exp->kind == expKindLess )
        orderedBinaryGen(exp, "<", f);
    else if ( exp->kind ==  expKindGreater )
        orderedBinaryGen(exp, ">", f);
    else if ( exp->kind == expKindMod )
        standardBinaryGen(exp, "%", f);//%% -> % didn't work before
    else if ( exp->kind == expKindBitAnd )
        standardBinaryGen(exp, "&", f);
    else if ( exp->kind == expKindBitOr )
        standardBinaryGen(exp, "|", f);
    else if ( exp->kind == expKindBitNotBinary ) //lol
        standardBinaryGen(exp, "^", f);
    else if ( exp->kind == expKindBitShiftLeft )
        standardBinaryGen(exp, "<<", f);
    else if ( exp->kind == expKindBitShiftRight )
        standardBinaryGen(exp, ">>", f);
    else if ( exp->kind == expKindBitAndNot )
        standardBinaryGen(exp, "&~", f);
    fprintf(f, ")");
    return;
} 

static void indent(int indentLevel,FILE* fp){
    for(int i = 0; i < indentLevel; i++){
        fprintf(fp,"\t");
    }
}



void printCodeGen(ExpList* list,int indentLevel,FILE* fp){
    if (list == NULL){
        return;
    }

    Exp* exp = list->cur;
    TTEntry* type = getExpressionType(exp);
    //Implicit assumption that we have a type that resolves to a printable base type
    indent(indentLevel,fp);


    switch (type->val.nonCompositeType.type){
        case baseInt:
            fprintf(fp,"printf(\"%%d\",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseFloat64:
            fprintf(fp,"printf(\"%%10e\",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseRune:
            fprintf(fp,"printf(\"%%d\",(int)(");
            expCodeGen(exp,fp);
            fprintf(fp,"));\n");
            break;
        case baseString:
            fprintf(fp,"printf(\"%%s\",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseBool: //QUITE UNCERTAIN DEPENDS ON REPRESENTATION OF BOOL
            fprintf(fp,"printf(\"%%s\",(0==");
            expCodeGen(exp,fp);
            fprintf(fp,")?\"false\":\"true\");\n");
            break;
    }

    printCodeGen(list->next,indentLevel,fp);
}


void printlnCodeGen(ExpList* list,int indentLevel,FILE* fp){
    if (list == NULL){
        indent(indentLevel,fp);
        fprintf(fp,"printf(\"\\n\");\n");
        return;
    }

    Exp* exp = list->cur;
    TTEntry* type = getExpressionType(exp);
    //Implicit assumption that we have a type that resolves to a printable base type
    indent(indentLevel,fp);
    switch (type->val.nonCompositeType.type){
        case baseInt:
            fprintf(fp,"printf(\"%%d \",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseFloat64:
            fprintf(fp,"printf(\"%%10e \",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseRune:
            fprintf(fp,"printf(\"%%d \",(int)(");
            expCodeGen(exp,fp);
            fprintf(fp,"));\n");
            break;
        case baseString:
            fprintf(fp,"printf(\"%%s \",");
            expCodeGen(exp,fp);
            fprintf(fp,");\n");
            break;
        case baseBool: //QUITE UNCERTAIN DEPENDS ON REPRESENTATION OF BOOL
            fprintf(fp,"printf(\"%%s \",(0==");
            expCodeGen(exp,fp);
            fprintf(fp,")?\"false\":\"true\");\n");
            break;
    }


    printlnCodeGen(list->next,indentLevel,fp);
}


int expCodeGenSet(Exp *exp, FILE *f)
{
    if ( exp->kind == expKindIndexing )
    {
        if ( getExpressionType(exp->val.access.base)->underlyingType == sliceType )
        {
            fprintf(f, "sliceSet(");
            expCodeGen(exp->val.access.base, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.access.accessor, f);
            fprintf(f, ", %d, ", exp->lineno);
        } else {
            fprintf(f, "arrSet(");
            expCodeGen(exp->val.access.base, f);
            fprintf(f, ", ");
            expCodeGen(exp->val.access.accessor, f);
            fprintf(f, ", %d, %d, ", getExpressionType(exp->val.access.base)->val.arrayType.size, exp->lineno);
        }
        return 1;
    }
    else {
        expCodeGen(exp, f);
        return 0;
    }
}
void assignStmtCodeGen(ExpList* left, ExpList* right,int indentLevel,FILE* fp){
    if (left == NULL || right == NULL){
        return;
    }
	
    
    //Broken (replace void* by actual type)
    //Generate all temp assignments then assign them after temp assignments complete
    char* temp = tmpVarGen();
    indent(indentLevel,fp);
	
    generateOurTypes(getExpressionType(right->cur),fp);
    fprintf(fp," %s = ",temp);
    expCodeGen(right->cur,fp);
    fprintf(fp,";\n");

	
    assignStmtCodeGen(left->next,right->next,indentLevel,fp);
	
    if (!isBlank(left->cur)){ 
        indent(indentLevel,fp);
        int isIndex = expCodeGenSet(left->cur, fp);
        TTEntry *type = getExpressionType(right->cur);
        if ( isIndex )
        {    
            if ( type->underlyingType == identifierType )
            {
                switch ( type->val.nonCompositeType.type )
                {
                    case baseBool:
                    case baseInt:
                        fprintf(fp, "createPolyInt(");
                        break;
                    case baseFloat64:
                        fprintf(fp, "createPolyFloat(");
                        break;
                    case baseRune:
                        fprintf(fp, "createPolyRune(");
                        break;
                    case baseString:
                        fprintf(fp, "createPolyString(");
                        break;
                }
            } else
            {
                fprintf(fp, "createPolyVoid(");
            }
            fprintf(fp, "%s));\n", temp);
            //arr/sliceSet(variable, position, lineNo, polyCreate(itemToSet));
        } else
        {
            if ( type->underlyingType == arrayType )
            {
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                strcpy(typeChain, "");
                generateTypeChain(type->val.arrayType.type, typeChain);
                fprintf(fp," = arrCopy(%s, \"%s\", %d);\n", temp, typeChain, type->val.arrayType.size);
                free(typeChain);
            }
            else if ( type->underlyingType == structType )
            {
                fprintf(fp, " = %s_copy(%s);", idGenJustType(type), temp);
            }
            else fprintf(fp," = %s;\n", temp);
        }
    }


}





void expListToBool( ExpList* list, char* exp1,FILE* fp){
    //explistToBool should never be called with an empty list
    
    
    Exp* exp = list->cur;
    TTEntry* type = getExpressionType(exp);
        
            switch ( type->underlyingType )
            {
                case identifierType:
                    switch ( type->val.nonCompositeType.type )
                    {
                        case baseInt:
                        case baseBool:
                        case baseFloat64:
                        case baseRune:
                            expCodeGen(exp,fp);
                            fprintf(fp, " == " );
                            fprintf(fp,"%s",exp1);
                            break;
                        case baseString:
                            fprintf(fp, "strcmp(");
                            expCodeGen(exp, fp);
                            fprintf(fp, ", ");
                            fprintf(fp,"%s",exp1);
                            fprintf(fp, ") == 0");
                            break;
                    }
                    break;
                case structType:
                    fprintf(fp, "%s_equality(", idGenJustType(type));
                    expCodeGen(exp, fp);
                    fprintf(fp, ", ");
                    fprintf(fp,"%s",exp1);
                    fprintf(fp, ")");
                    break;
                case arrayType:
                    fprintf(fp, "arrEquality(");
                    expCodeGen(exp, fp);
                    fprintf(fp, ", ");
                    fprintf(fp,"%s, ",exp1);
                    char * typeChain = (char *) malloc(sizeof(char) * 999);
                    strcpy(typeChain, "");
                    generateTypeChain(type->val.arrayType.type, typeChain);
                    fprintf(fp, "%s, %d)", typeChain, type->val.arrayType.size);
                    free(typeChain);
                    break;
            }

    if (list->next == NULL){
        return;
    }else{
        fprintf(fp, " || ");
        expListToBool(list->next,exp1,fp);
    }
}

void switchToIfCodeGen(char* exp,switchCaseClause* list,int indentLevel, FILE* fp){
    if (list == NULL){
        return;
    }
    if (list->expressionList == NULL){ //Default case (I'm not generating code after default cases as it cannot execute)
        indent(indentLevel,fp);
        fprintf(fp,"if (1) {\n");

        stmtCodeGen(list->statementList,indentLevel+1,fp);

        indent(indentLevel,fp);
        fprintf(fp,"}\n");

        return;
    }else{
        
        indent(indentLevel,fp);
        fprintf(fp,"if(");
        expListToBool(list->expressionList,exp,fp);
        fprintf(fp,"){\n");
        
        stmtCodeGen(list->statementList,indentLevel+1,fp);
        
        indent(indentLevel,fp);
        fprintf(fp,"}\n");

        indent(indentLevel,fp);
        fprintf(fp,"else{\n");

        switchToIfCodeGen(exp,list->next,indentLevel+1,fp);

        indent(indentLevel,fp);
        fprintf(fp,"}\n");
         

    }
}

char* makeLabel(){
	char* ptr = malloc(sizeof(char)*32);
	sprintf(ptr,"label%d",labelCount++);
	return ptr;
}

void localContinueReplace(Stmt* stmt,char* label){//Replaces or loop continues by appropriate gotos
		if (stmt == NULL){
			return;
		}

		Stmt* next;

		switch (stmt->kind){
			case StmtKindBlock:
                localContinueReplace(stmt->val.block.stmt,label);
				break;
    		case StmtKindIf:
    			localContinueReplace(stmt->val.ifStmt.block,label);
    			localContinueReplace(stmt->val.ifStmt.elseBlock,label);
    			break;
    		case StmtKindElse:
    			localContinueReplace(stmt->val.elseStmt.block,label);
    			break;
    		case StmtKindSwitch:
    			for(switchCaseClause* cur = stmt->val.switchStmt.clauseList; cur != NULL; cur = cur->next){
    				localContinueReplace(cur->statementList,label);
    			}
    			break;
    		case StmtKindContinue:
    			//Transmute a continue node into a goto node
    			next = stmt->next;
    			stmt->kind = StmtKindGoto;
    			stmt->val.gotoStmt.label = label;
    			stmt->next = next;
    			break;

		}

		localContinueReplace(stmt->next,label);
}

void varDeclAssignCodeGen(VarDeclNode* decl, int indentLevel, FILE* fp);
void varJustDeclNoVal(VarDeclNode* varDecl, int indentLevel, FILE* fp);
void specialIncDecStatementCodeGen(Stmt* stmt,int indentLevel, FILE* fp);
void specialOpStatementCodeGen(Stmt* stmt, int indentLevel,FILE* fp);

void stmtCodeGen(Stmt* stmt,int indentLevel, FILE* fp){
    if (stmt == NULL){
        return;
    }


    switch (stmt->kind){
        case StmtKindBlock:
            indent(indentLevel,fp);
            fprintf(fp, "{\n");
            stmtCodeGen(stmt->val.block.stmt,indentLevel+1,fp);
            indent(indentLevel,fp);
            fprintf(fp, "}\n");
            break;
        case StmtKindExpression:
            indent(indentLevel,fp);
            expCodeGen(stmt->val.expression.expr,fp);
            fprintf(fp,";\n");
            break;
        case StmtKindAssignment:
            //This is broken (need to assign types properly for temp vars,using void* for now)
            assignStmtCodeGen(stmt->val.assignment.lhs,stmt->val.assignment.rhs,indentLevel,fp);
            break;
    
        case StmtKindPrint:
            printCodeGen(stmt->val.print.list,indentLevel,fp);
            break;
        case StmtKindPrintln:
            printlnCodeGen(stmt->val.println.list,indentLevel,fp);
            break;
        case StmtKindIf:
            indent(indentLevel,fp);
            fprintf(fp,"{\n");

            if(stmt->val.ifStmt.statement != NULL){
                stmtCodeGen(stmt->val.ifStmt.statement,indentLevel+1,fp);
            }

            indent(indentLevel+1,fp);
            fprintf(fp,"if(");
            expCodeGen(stmt->val.ifStmt.expression,fp);
            fprintf(fp,")\n");

            stmtCodeGen(stmt->val.ifStmt.block,indentLevel+1,fp);
            stmtCodeGen(stmt->val.ifStmt.elseBlock,indentLevel+1,fp);

            indent(indentLevel,fp);
            fprintf(fp,"}\n");
            

            break;


            
        case StmtKindReturn:
            indent(indentLevel,fp);
            fprintf(fp,"return ");
            if (stmt->val.returnVal.returnVal != NULL){
                expCodeGen(stmt->val.returnVal.returnVal,fp);
            }
            fprintf(fp,";\n");
            break;

        case StmtKindElse:
            indent(indentLevel,fp);
            fprintf(fp,"else \n");


            stmtCodeGen(stmt->val.elseStmt.block,indentLevel,fp);
            
            break;
        case StmtKindSwitch: 
            //Enclosing the switch stmt in a while loop to handle break stmts was Alex's idea
            indent(indentLevel,fp);
            fprintf(fp,"while(1){\n");

            stmtCodeGen(stmt->val.switchStmt.statement,indentLevel,fp);//initial declaration

            char* temp = tmpVarGen();
            indent(indentLevel+1,fp);
            generateOurTypes(getExpressionType(stmt->val.switchStmt.expression),fp);
            fprintf(fp," %s = ",temp);
            expCodeGen(stmt->val.switchStmt.expression,fp);
            fprintf(fp,";\n");
            

            
            switchToIfCodeGen(temp,stmt->val.switchStmt.clauseList,indentLevel+1,fp);
            

            indent(indentLevel+1,fp);
            fprintf(fp,"break;\n");

            indent(indentLevel,fp);
            fprintf(fp,"}\n");
            break;
        case StmtKindInfLoop:
            indent(indentLevel,fp);
            fprintf(fp,"while(1)\n");
            stmtCodeGen(stmt->val.infLoop.block,indentLevel,fp);
            break;

        case StmtKindWhileLoop:
            indent(indentLevel,fp);
            fprintf(fp,"while(");
            expCodeGen(stmt->val.whileLoop.conditon,fp);
            fprintf(fp,")\n");
            stmtCodeGen(stmt->val.whileLoop.block,indentLevel,fp);
            break;
        case StmtKindThreePartLoop:
            
            indent(indentLevel,fp);
            fprintf(fp,"{\n");

            stmtCodeGen(stmt->val.forLoop.init,indentLevel+1,fp);

            indent(indentLevel+1,fp);
            fprintf(fp,"while(");
            if (stmt->val.forLoop.condition == NULL){
                fprintf(fp,"1");
            }else{
                expCodeGen(stmt->val.forLoop.condition,fp);
            }  
            fprintf(fp,"){\n");

            char* label = makeLabel();
            localContinueReplace(stmt->val.forLoop.block,label);
            stmtCodeGen(stmt->val.forLoop.block->val.block.stmt,indentLevel+2,fp);


            indent(indentLevel+2,fp);
            fprintf(fp,"%s:;\n",label);

            stmtCodeGen(stmt->val.forLoop.inc,indentLevel+2,fp);

            indent(indentLevel+1,fp);
            fprintf(fp,"}\n");

            indent(indentLevel,fp);
            fprintf(fp,"}\n");
            break;

        case StmtKindGoto:
            indent(indentLevel,fp);
            fprintf(fp,"goto %s;\n",stmt->val.gotoStmt.label);
            break;


        case StmtKindBreak:
            indent(indentLevel,fp);
            fprintf(fp,"break;\n");
            break;
        case StmtKindContinue:
            indent(indentLevel,fp);
            fprintf(fp,"continue;\n");
            break;

        case StmtKindOpAssignment:
            {

                if (stmt->val.incStmt.exp->kind == expKindIndexing){
                    specialOpStatementCodeGen(stmt,indentLevel,fp);
                    break;
                }
                Exp* lhs = stmt->val.opAssignment.lhs;
                Exp* rhs = stmt->val.opAssignment.rhs;
                TTEntry* type;

                switch (stmt->val.opAssignment.kind) {
				case expKindAddition:
					type = getExpressionType(lhs);
                    if(type->val.nonCompositeType.type == baseString){
                        char* temp1 = tmpVarGen();
                        char* temp2 = tmpVarGen();
                        
                        //eval right, save location in memory
                        //char* temp1 = evaluated(rhs);
                        indent(indentLevel,fp);
                        fprintf(fp,"char* %s = ",temp1);
                        expCodeGen(rhs,fp);
                        fprintf(fp,";\n");

                        //eval left, store location in moemory
                        //char* temp2 = evaluated(lhs);
                        indent(indentLevel,fp);
                        fprintf(fp,"char** %s = &",temp2);
                        expCodeGen(lhs,fp);
                        fprintf(fp,";\n");

                        //assign concatnation of left and right to the left
                        //temp2 = concat(temp2,temp1);
                        indent(indentLevel,fp);
                        fprintf(fp,"*%s = concat(*%s,%s);\n",temp2,temp2,temp1);
                        

                    }else{
                        indent(indentLevel,fp);
                        expCodeGen(lhs,fp);
                        fprintf(fp," += " );
                        expCodeGen(rhs,fp);
                        fprintf(fp,";\n");
                    }
					break;
				case expKindSubtraction:
                    indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," -= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
					break;
				case expKindMultiplication:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," *= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
					break;
				case expKindDivision:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," /= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindMod:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," %%= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitAnd:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," &= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitOr:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," |= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitNotBinary:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," ^= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitShiftLeft:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," <<= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitShiftRight:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
					fprintf(fp," >>= " );
                    expCodeGen(rhs,fp);
                    fprintf(fp,";\n");
                    break;
				case expKindBitAndNot:
					indent(indentLevel,fp);
                    expCodeGen(lhs,fp);
                    fprintf(fp," &= ~(");
                    expCodeGen(rhs,fp);
                    fprintf(fp,");\n");
					break;
			
            }
                break;
            case StmtKindInc:
                if (stmt->val.incStmt.exp->kind == expKindIndexing){
                    specialIncDecStatementCodeGen(stmt,indentLevel,fp);
                    break;
                }
                
                indent(indentLevel,fp);
                expCodeGen(stmt->val.incStmt.exp,fp);
                fprintf(fp,"++;\n");
                break;
            case StmtKindDec:
                if (stmt->val.decStmt.exp->kind == expKindIndexing){
                    specialIncDecStatementCodeGen(stmt,indentLevel,fp);
                    break;
                }
                indent(indentLevel,fp);
                expCodeGen(stmt->val.decStmt.exp,fp);
                fprintf(fp,"--;\n");
                break;

		case StmtKindTypeDeclaration:
			/* this one actually is just break*/
			break;
		case StmtKindVarDeclaration:
			/* I think I can just copy the other one */
		case StmtKindShortDeclaration: 
			/* this might be awful */
			varJustDeclNoVal(stmt -> val.varDeclaration, indentLevel, fp);
			varDeclAssignCodeGen(stmt -> val.varDeclaration, indentLevel, fp);
			break;
        
    }

    
    }
    stmtCodeGen(stmt->next,indentLevel,fp);
}

void specialIncDecStatementCodeGen(Stmt* stmt,int indentLevel, FILE* fp){
     if (stmt == NULL){
        return;
    }

    Exp* lhs;
    lhs = stmt->val.incStmt.exp;

    char* index = tmpVarGen();
    indent(indentLevel,fp);
    fprintf(fp,"int %s = ",index);
    expCodeGen(lhs->val.access.accessor,fp);
    fprintf(fp,";\n");
    int arrLength = getExpressionType(lhs->val.access.base)->val.arrayType.size;

    char* base = tmpVarGen();
    indent(indentLevel,fp);
    generateOurTypes(getExpressionType(lhs->val.access.base),fp);
    indent(indentLevel,fp);
    fprintf(fp,"%s = ",base);
    expCodeGen(lhs->val.access.base,fp);
    fprintf(fp,";\n");

    int lineno = lhs->lineno;

    indent(indentLevel,fp);

    char* dotAccess;
    TTEntry* type = getExpressionType(lhs);

    switch (type->val.nonCompositeType.type) {
        case baseInt:
            dotAccess = "intVal";
            break;
        case baseFloat64:
            dotAccess = "floatVal";
            break;
        case baseRune:
            dotAccess = "charVal";
            break;
    }

    fprintf(fp,"arrSet(%s,%s,%d,%d,arrGet(%s,%s,%d,%d).%s",base,index,arrLength,lineno,base,index,arrLength,lineno,dotAccess);
    switch(stmt->kind){ 
        case StmtKindInc:   
            fprintf(fp,"+1);\n");
            break;
        case StmtKindDec:
            fprintf(fp,"-1);\n");
            break;
    }

}


void specialOpStatementCodeGen(Stmt* stmt, int indentLevel,FILE* fp){
    if (stmt == NULL){
        return;
    }

    
    Exp* lhs = stmt->val.opAssignment.lhs;
    Exp* rhs = stmt->val.opAssignment.rhs;


    char* right = tmpVarGen();
    indent(indentLevel,fp);
    generateOurTypes(getExpressionType(rhs),fp);
    fprintf(fp,"%s = ",right);
    expCodeGen(rhs,fp);
    fprintf(fp,";\n");


    char* index = tmpVarGen();
    indent(indentLevel,fp);
    fprintf(fp,"int %s = ",index);
    expCodeGen(lhs->val.access.accessor,fp);
    fprintf(fp,";\n");

    char* base = tmpVarGen();
    indent(indentLevel,fp);
    generateOurTypes(getExpressionType(lhs->val.access.base),fp);
    fprintf(fp,"%s = ",base);
    expCodeGen(lhs->val.access.base,fp);
    fprintf(fp,";\n");

    int arrLength = getExpressionType(lhs->val.access.base)->val.arrayType.size;
    int lineno = lhs->lineno;


    char* dotAccess;

    switch (getExpressionType(lhs)->val.nonCompositeType.type) {
        case baseInt:
            dotAccess = "intVal";
            break;
        case baseFloat64:
            dotAccess = "floatVal";
            break;
        case baseRune:
            dotAccess = "charVal";
            break;
        case baseString:
            dotAccess = "stringVal";
            break;
        default:
            dotAccess = "intVal";
            break;
    }

    indent(indentLevel,fp);
    fprintf(fp,"arrSet(%s,%s,%d,%d,",base,index,arrLength,lineno);

    char arrGet[500];
    sprintf(arrGet,"(arrGet(%s,%s,%d,%d).%s)",base,index,arrLength,lineno,dotAccess);



    switch (stmt->val.opAssignment.kind) {
				case expKindAddition:
                    if (getExpressionType(lhs)->val.nonCompositeType.type == baseString){
                        fprintf(fp,"concat(%s,%s));\n",arrGet,right);
                    }else{
                        fprintf(fp,"%s+%s);\n",arrGet,right);
                    }
					break;
				case expKindSubtraction:
					fprintf(fp,"%s+%s);\n",arrGet,right);
					break;
				case expKindMultiplication:
					fprintf(fp,"%s*%s);\n",arrGet,right);
					break;
				case expKindDivision:
					fprintf(fp,"%s/%s);\n",arrGet,right);
					break;
				case expKindMod:
					fprintf(fp,"%s%%%s);\n",arrGet,right);
					break;
				case expKindBitAnd:
					fprintf(fp,"%s&%s);\n",arrGet,right);
					break;
				case expKindBitOr:
					fprintf(fp,"%s|%s);\n",arrGet,right);
					break;
				case expKindBitNotBinary:
					fprintf(fp,"%s^%s);\n",arrGet,right);
					break;
				case expKindBitShiftLeft:
					fprintf(fp,"%s<<%s);\n",arrGet,right);
					break;
				case expKindBitShiftRight:
					fprintf(fp,"%s>>%s);\n",arrGet,right);
					break;
				case expKindBitAndNot:
					fprintf(fp,"%s&(~%s));\n",arrGet,right);
					break;
			}


}



void generateOurTypes(TTEntry *t, FILE *f)
{
	if (t == NULL)
	{
		fprintf(f, "void");
	} else if ( t->underlyingType == arrayType )
	{
		fprintf(f, "__golite_poly_entry *");
	} else if ( t->underlyingType == sliceType )
	{
		fprintf(f, "__golite_slice *");
	} else if ( t->underlyingType == structType )
	{
		fprintf(f, "%s *", idGenJustType(t));
	} else if ( t->underlyingType == identifierType )
	{
		switch ( t->val.nonCompositeType.type )
		{
			case baseBool:
			case baseInt: fprintf(f, "int"); return;
			case baseFloat64: fprintf(f, "double"); return;
			case baseRune: fprintf(f, "char"); return;
			case baseString: fprintf(f, "char *"); return;
		}
	} else {
		
		fprintf(stderr, "There is a buggggg fix it (generate our types)\n");
		exit(1);
	}
}






void funcCodeGen(FuncDeclNode* func, FILE* fp) {
	
	
	generateOurTypes(func -> symbolEntry -> type -> val.functionType.ret, fp);
	
	fprintf(fp, " ");
	
	char* funcName;
	funcName = idGenJustVar(func -> symbolEntry);
	
	fprintf(fp,"%s", funcName);
	
	fprintf(fp, "(");

	STEntry* argsIter = func -> symbolEntry -> type -> val.functionType.args;
	char* argNameHolder;
	
	if (argsIter != NULL) {
		generateOurTypes(argsIter -> type, fp);
		fprintf(fp, " ");
		argNameHolder = idGenJustVar(argsIter);
		fprintf(fp,"%s", argNameHolder);
		argsIter = argsIter -> next;
	}
	
	
	while (argsIter != NULL) {
		fprintf(fp, ", ");
		generateOurTypes(argsIter -> type, fp);
		fprintf(fp, " ");
		argNameHolder = idGenJustVar(argsIter);
		fprintf(fp, "%s",argNameHolder);
		argsIter = argsIter -> next;
	}
	
	
	fprintf(fp, ")\n"); /* ask neil abouat brackets */
	stmtCodeGen(func -> blockStart,0,fp);
	
}

void appendToChain(char* thing, IdChain* curChain) {
	
	if (curChain == NULL) {
		return;
	}
	IdChain* iter = curChain;
	while (iter -> next != NULL) {
		iter = iter -> next;
	}
	iter -> next = makeIdChain(thing, NULL);
	return;
}

IdChain* copyChain(IdChain* curChain) {
	
	if (curChain == NULL) {
		return NULL;
	}
	IdChain* iter = curChain;
	IdChain* retable = makeIdChain(iter -> identifier, NULL);
	IdChain* retIter = retable;
	while (iter -> next != NULL) {
		iter = iter -> next;
		retIter -> next = makeIdChain(iter -> identifier, NULL);
		retIter = retIter -> next;
	}
	return retable;
}


void genInitAndZero(IdChain* curName, TTEntry* curType, int indentLevel, FILE* fp){
	indent(indentLevel, fp);
	IdChain* iter;
	for (iter = curName; iter != NULL; iter = iter -> next) {
		fprintf(fp, "%s",iter -> identifier);
	}
	fprintf(fp, " = ");
	if (curType -> underlyingType == identifierType) {
		
		if (curType -> val.nonCompositeType.type == baseString) {
			fprintf(fp, "\"\";\n");
		} else {
			fprintf(fp, "0;\n");
		}
	} else if (curType -> underlyingType == sliceType) {
		fprintf(fp, "malloc(sizeof(__golite_slice));\n");
		indent(indentLevel, fp);
		for (iter = curName; iter != NULL; iter = iter -> next) {
			fprintf(fp,"%s", iter -> identifier);
		}
		fprintf(fp, " -> size = 0;\n");
		indent(indentLevel, fp);
		for (iter = curName; iter != NULL; iter = iter -> next) {
			fprintf(fp, "%s",iter -> identifier);
		}
		fprintf(fp, " -> capacity = 0;\n");
	} else if (curType -> underlyingType == arrayType) {
		fprintf(fp, "malloc(%d * sizeof(__golite_poly_entry));\n", curType -> val.arrayType.size);
		indent(indentLevel, fp);
		fprintf(fp, "for(int golite_iter_%d = 0; golite_iter_%d < %d; golite_iter_%d++) {\n", indentLevel, indentLevel, curType -> val.arrayType.size, indentLevel);
		IdChain* nextName = copyChain(curName);
		char arrayPositions[40];
		sprintf(arrayPositions, "[golite_iter_%d].", indentLevel);
		if (curType -> val.arrayType.type -> underlyingType == identifierType) {
			if (curType -> val.arrayType.type -> val.nonCompositeType.type == baseInt || curType -> val.arrayType.type -> val.nonCompositeType.type == baseBool) {
				strcat(arrayPositions, "intVal");
			} else if (curType -> val.arrayType.type -> val.nonCompositeType.type == baseRune) {
				strcat(arrayPositions, "runeVal");
			} else if (curType -> val.arrayType.type -> val.nonCompositeType.type == baseString) {
				strcat(arrayPositions, "stringVal");
			} else if (curType -> val.arrayType.type -> val.nonCompositeType.type == baseFloat64) {
				strcat(arrayPositions, "floatVal");
			}
		} else {
			strcat(arrayPositions, "polyVal");
		}
		appendToChain(arrayPositions, nextName);
		genInitAndZero(nextName, curType -> val.arrayType.type, indentLevel+1, fp);
		indent(indentLevel, fp);
		fprintf(fp, "}\n");
	} else if (curType -> underlyingType == structType) {
		fprintf(fp, "malloc(sizeof(%s));\n", idGenJustType(curType));
		STEntry *s;
		IdChain* nextName;
		for (iter = curType -> val.structType.fieldNames; iter != NULL; iter = iter -> next) {
			s = getEntry(curType -> val.structType.fields, iter->identifier)->entry.s;
			if (strcmp(s -> id, "_") != 0) {
				char fieldNameAddition [900];
				sprintf(fieldNameAddition, " -> %s", structMemb(s -> id));
				nextName = copyChain(curName);
				appendToChain(fieldNameAddition, nextName);
				genInitAndZero(nextName, s -> type, indentLevel, fp);
			}
		}
	}
}





void varDeclAssignCodeGen(VarDeclNode* decl, int indentLevel, FILE* fp) {
	
	if(decl == NULL) {
		return;
	}
	
	char * varName = idGenJustVar(decl -> whoAmI);
	
	
	if (decl -> value == NULL) {
		genInitAndZero(makeIdChain(varName, NULL), decl -> whoAmI -> type, indentLevel, fp);
	} else {
	
		indent(indentLevel, fp);
		
		
		fprintf(fp,"%s", varName);
		fprintf(fp, " = ");
		if (decl -> whoAmI -> type -> underlyingType == identifierType) {
			expCodeGen(decl -> value, fp);
			fprintf(fp, ";\n");
		} else if (decl -> whoAmI -> type -> underlyingType == arrayType) {
			char * typeChain = (char *) malloc(sizeof(char) * 999);
            strcpy(typeChain, "");
			generateTypeChain(decl -> whoAmI -> type -> val.arrayType.type, typeChain);
			
			fprintf(fp, "arrCopy(");
			expCodeGen(decl -> value, fp);
			fprintf(fp, ", %s, %d);\n", typeChain, decl -> whoAmI -> type -> val.arrayType.size);
			
		} else if (decl -> whoAmI -> type -> underlyingType == sliceType) {
			expCodeGen(decl -> value, fp);
            fprintf(fp,";\n"); //@Denali - Neil added this
		} else if (decl -> whoAmI -> type -> underlyingType == structType) {
			
			fprintf(fp, "%s_copy(", idGenJustType(decl -> whoAmI -> type));
			
			expCodeGen(decl -> value, fp);
			fprintf(fp, ");\n");
			
		} else {
			fprintf(stderr, "fuck (this is impossible), bad type during var decl");
			exit(1);
		}
	}
	
	varDeclAssignCodeGen(decl -> nextDecl, indentLevel, fp);
	varDeclAssignCodeGen(decl -> multiDecl, indentLevel, fp);
}


void varJustDeclNoVal(VarDeclNode* varDecl, int indentLevel, FILE* fp) {
	
	if (varDecl == NULL) {
		return;
	}
	if (varDecl -> iDoDeclare = 1) {
		indent(indentLevel, fp);
		generateOurTypes(varDecl -> whoAmI -> type, fp);
		fprintf(fp, " ");
		char * varName = idGenJustVar(varDecl -> whoAmI);
		fprintf(fp, "%s;\n", varName);
	}
	
	varJustDeclNoVal(varDecl -> nextDecl, indentLevel, fp);
	varJustDeclNoVal(varDecl -> multiDecl, indentLevel, fp);
}


/*

void typeDeclCodeGen(TypeDeclNode* decl, FILE* fp) {
	return;
	/*
	char* oldtype, newtype;
	fprintf(fp, "typedef %s %s;\n\n", oldtype, newtype);
	
}*/



void totalCodeGen(RootNode* root) {
	/*
	 * Open a file
	 */
	
	FILE* output = fopen("go.out.c", "w");
	fprintf(output, "#include \"templateCode.h\"\n");
	
	trie  = encodeRoot(root);
    /*
	 * print headers, maybe. Idk
	 */
	/*
	
	printHeaders(root);
	*/

	codegenStructDeclaration(0,output);
	
	TopDeclarationNode* mainIter = root -> startDecls;
	
	while (mainIter != NULL) {
		if (mainIter -> declType == funcDeclType) {
			
			funcCodeGen(mainIter -> actualRealDeclaration.funcDecl, output);
		} else if (mainIter -> declType == variDeclType){
			varJustDeclNoVal(mainIter -> actualRealDeclaration.varDecl, 0, output);
		} else if (mainIter -> declType == typeDeclType){
			/*
			 * Do nothing
			 * 
			typeDeclCodeGen(mainIter -> actualRealDeclaration.typeDecl, output);
			*/
		}
		mainIter = mainIter -> nextTopDecl;
	}
        
	
	 
	/*
	 * 
	 * print out main function which also calls inits
	 * 
	 */
	
	
	fprintf(output, "int main() {\n");
	mainIter = root -> startDecls;
	while (mainIter != NULL) {
		if (mainIter -> declType == variDeclType) {
			varDeclAssignCodeGen(mainIter -> actualRealDeclaration.varDecl, 1, output);
		}
		mainIter = mainIter -> nextTopDecl;
	}
	
	for (int i=0; i<initCount; i++) {
		fprintf(output, "\t__golite_init_%d();\n",i);
	}
	
	fprintf(output, "\t__golite_main();\n");
	
	/*
	 * 
	 * Run the inits
	 * 
	 * Then run the golite main
	 * 
	 */
	
	

	fprintf(output,"}\n");


	
	
	fclose(output);
	
	
}




void codegenStructDeclaration(int indentLevel,FILE* fp){
    if (globalList->structChain[0] == NULL){
        return; //small quirk but no structs leave a list of length 1
    }

    
    for(int i = 0; i < globalList->size; i++){
        Trie* cur = globalList->structChain[i];


        char* name = idGenJustType(cur->type);
        indent(indentLevel,fp);
        fprintf(fp,"typedef struct %s %s;\n",name,name);
        free(name);
    }

    for(int i = 0; i < globalList->size; i++){
        Trie* cur = globalList->structChain[i];
        char* name = idGenJustType(cur->type);

        fprintf(fp,"\n");
        indent(indentLevel,fp);
        fprintf(fp,"struct %s{\n",name);

        Trie* entry = cur->child;//Skipping first element as the first node is a label

        while(entry != NULL){
            if (strcmp("_",entry->variant.entry->id) == 0){
                entry = entry->child;//skip blank identifier
                continue;
            }


            indent(indentLevel+1,fp);
            generateOurTypes(entry->variant.entry->type,fp);
            char* temp = structMemb(entry->variant.entry->id);
            fprintf(fp," %s;\n",temp);
            free(temp);



            entry = entry->child;
        }


        indent(indentLevel,fp);
        fprintf(fp,"};\n\n");

        free(name);

        
    }
}
