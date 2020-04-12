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
        if ( e->entry.s->isConstant == 2 && strcmp(id, "main") == 0 )
        {
           return strcpy(retVal, "__golite_main");
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
    fprintf(f, "int %s_equality(%s x, %s y) {\n\treturn ", idGenJustType(structType), idGenJustType(structType), idGenJustType(structType));
    /* 
    int STRUCT_NAME_equality(STRUCT_NAME x, STRUCT_NAME y) {
        return 
    
    IdChain *cur = structType->val.structType.fieldNames;
    Context *ctx = structType->val.structType.fields;
    while ( cur != NULL )
    {
        TTEntry *t = getEntry(c, cur->id)->entry.t; //Gets the type of the id
        char *fieldName = structMemb(cur->id);
        if ( strcmp(cur->id, "_") == 0 )
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
                        fprintf(f, "x.%s == y.%s && ", fieldName, fieldName);
                        break;
                    case baseString:
                        fprintf(f, "strcmp(x.%s, y.%s) == 0 && ", fieldName, fieldName);
                        break;
                }
                break;
            case structType:
                fprintf(f, "%s_equality(x.%s, y.%s) && ", idGenJustType(t), fieldName, fieldName);
                break;
            case arrType:
                char * typeChain = (char *) malloc(sizeof(char) * 999);
                generateTypeChain(t->val.arrayType.type, typeChain);
                fprintf(f, "arrEquality(x.%s, y.%s, %s, %d) && ", fieldName, fieldName, typeChain, t->val.arrayType.size);
                free(typeChain);
                break;
        }
        free(fieldName);
        cur = cur->next;
    }*/
    fprintf(f,"1;\n}\n"); //Handles if all our fields are blank.
    /* && 1;
    }
    */
}
//AY->Array, ST->Struct, SG->String, RE->rune, IR->Integer, FT->Float
//I don't think there's a better way to do this...
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
		//expListCodeGen(list->next, f);
	}

}


/*
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
            //TODO fix to account for call-by-value
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
                    //TODO clean up the ordered expression codes
                    //low priority
                    TEntry *type = getExpressionType(e->val.left);
                    switch ( type->val.nonCompositeType.type )
                    {
                        case baseInt:
                        case baseBool:
                        case baseFloat64:
                        case baseRune:
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, " <= " );
                            expCodeGen(exp->val.binary.right, f);
                            break;
                        case baseString:
                            fprintf(f, "strcmp(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ") <= 0");
                            break;
                    }
					break;
				case expKindGEQ:
					TEntry *type = getExpressionType(e->val.left);
                    switch ( type->val.nonCompositeType.type )
                    {
                        case baseInt:
                        case baseBool:
                        case baseFloat64:
                        case baseRune:
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, " >= " );
                            expCodeGen(exp->val.binary.right, f);
                            break;
                        case baseString:
                            fprintf(f, "strcmp(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ") >= 0");
                            break;
                    }
					break;
				case expKindLess:
					TEntry *type = getExpressionType(e->val.left);
                    switch ( type->val.nonCompositeType.type )
                    {
                        case baseInt:
                        case baseBool:
                        case baseFloat64:
                        case baseRune:
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, " < " );
                            expCodeGen(exp->val.binary.right, f);
                            break;
                        case baseString:
                            fprintf(f, "strcmp(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ") < 0");
                            break;
                    }
					break;
				case expKindGreater:
					TEntry *type = getExpressionType(e->val.left)
                    switch ( type->val.nonCompositeType.type )
                    {
                        case baseInt:
                        case baseBool:
                        case baseFloat64:
                        case baseRune:
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, " > " );
                            expCodeGen(exp->val.binary.right, f);
                            break;
                        case baseString:
                            fprintf(f, "strcmp(");
                            expCodeGen(exp->val.binary.left, f);
                            fprintf(f, ", ");
                            expCodeGen(exp->val.binary.right, f);
                            fprintf(f, ") > 0");
                            break;
                    }
					break;
				case expKindMod:
					expCodeGen(exp->val.binary.left, f);
					fputs(" % ", f); //for denali <3
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

*/


void expCodeGen(Exp *exp, FILE *f){

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
    TTEntry* type = exp->contextEntry->entry.s->type;
    //Implicit assumption that we have a type that resolves to a printable base type
    indent(indentLevel,fp);
    switch (type->val.nonCompositeType.type){
        case baseInt:
            fprintf(fp,"printf(\"%%d\",");
            break;
        case baseFloat64:
            fprintf(fp,"printf(\"%%lf\",");
            break;
        case baseRune:
            fprintf(fp,"printf(\"%%d\",(int)");
            break;
        case baseString:
            fprintf(fp,"printf(\"%%s\",");
            break;
        case baseBool: //QUITE UNCERTAIN DEPENDS ON REPRESENTATION OF BOOL
            fprintf(fp,"printf(\"%%d\",");
            break;
    }

    expCodeGen(exp,fp);
    fprintf(fp,");\\n");

    printCodeGen(list->next,indentLevel,fp);
}


void printlnCodeGen(ExpList* list,int indentLevel,FILE* fp){
    if (list == NULL){
        fprintf(fp,"printf(\"\\n\")\n");
        return;
    }

    Exp* exp = list->cur;
    TTEntry* type = exp->contextEntry->entry.s->type;
    //Implicit assumption that we have a type that resolves to a printable base type
    indent(indentLevel,fp);
    switch (type->val.nonCompositeType.type){
        case baseInt:
            fprintf(fp,"printf(\"%%d \",");
            break;
        case baseFloat64:
            fprintf(fp,"printf(\"%%lf \",");
            break;
        case baseRune:
            fprintf(fp,"printf(\"%%d \",(int)");
            break;
        case baseString:
            fprintf(fp,"printf(\"%%s \",");
            break;
        case baseBool: //QUITE UNCERTAIN DEPENDS ON REPRESENTATION OF BOOL
            fprintf(fp,"printf(\"%%d \",");
            break;
    }

    expCodeGen(exp,fp);
    fprintf(fp,");\n");

    printlnCodeGen(list->next,indentLevel,fp);
}

//TODO, does not involve code formatting
void simpleStmtCodeGen(Stmt* stmt,int indentLevel,FILE* fp){
    if (stmt == NULL){
        return;
    }

    switch (stmt->kind){
        case StmtKindAssignment:
            break;
        case StmtKindShortDeclaration:
            break;
        case StmtKindExpression :
            break;
        case StmtKindInc:
            break;
        case StmtKindDec:
            break;
        case StmtKindOpAssignment:
            break;
    }
}

void stmtCodeGen(Stmt* stmt,int indentLevel, FILE* fp){
    if (stmt == NULL){
        return;
    }

    switch (stmt->kind){
        //WARNING RENAME VARIABLES IN INNER SCOPE USING SCOPELEVEL
        case StmtKindBlock:
            indent(indentLevel,fp);
            printf("{\n");
            stmtCodeGen(stmt->val.block.stmt,indentLevel+1,fp);
            indent(indentLevel,fp);
            printf("}\n");
            break;
        case StmtKindExpression:
            indent(indentLevel,fp);
            simpleStmtCodeGen(stmt,indentLevel,fp);
            fprintf(fp,";\n");
            break;
        StmtKindAssignment:
            //LOTS TO DO, MULTIPLE ASSIGNMENT.  TEMP VARS
            simpleStmtCodeGen(stmt,indentLevel,fp);
            break;
    
        StmtKindPrint:
            printCodeGen(stmt->val.print.list,indentLevel,fp);
            break;
        StmtKindPrintln:
            printlnCodeGen(stmt->val.println.list,indentLevel,fp);
            break;
        StmtKindIf:
            indent(indentLevel,fp);
            fprintf(fp,"{\n");

            if(stmt->val.ifStmt.statement != NULL){
                stmtCodeGen(stmt->val.ifStmt.statement,indentLevel+1,fp);
            }

            indent(indentLevel+1,fp);
            fprintf(fp,"if(");
            expCodeGen(stmt->val.ifStmt.expression,fp);
            fprintf(fp,")\n");

            stmtCodeGen(stmt->val.ifStmt.block,indentLevel,fp);


            indent(indentLevel,fp);
            fprintf(fp,"}\n");

            break;


            
        StmtKindReturn:
            indent(indentLevel,fp);
            fprintf(fp,"return ");
            if (stmt->val.returnVal.returnVal != NULL){
                expCodeGen(stmt->val.returnVal.returnVal,fp);
            }
            fprintf(fp,";\n");
            break;

        StmtKindElse:
            indent(indentLevel,fp);
            fprintf(fp,"else \n");


            stmtCodeGen(stmt->val.elseStmt.block,indentLevel,fp);
            
            break;
        StmtKindSwitch: // Lots to do
            break;
        StmtKindInfLoop:
            indent(indentLevel,fp);
            fprintf(fp,"while(1)\n");
            stmtCodeGen(stmt->val.infLoop.block,indentLevel,fp);
            break;

        StmtKindWhileLoop:
            indent(indentLevel,fp);
            fprintf(fp,"while(");
            expCodeGen(stmt->val.whileLoop.conditon,fp);
            fprintf(fp,")\n");
            stmtCodeGen(stmt->val.whileLoop.block,indentLevel,fp);
            break;
        StmtKindThreePartLoop:
            //Subtle issues need to be dealt with
           break;


        StmtKindBreak:
            indent(indentLevel,fp);
            fprintf(fp,"break;\n");
            break;
        StmtKindContinue:
            indent(indentLevel,fp);
            fprintf(fp,"continue;\n");
            break;

        StmtKindOpAssignment:
            //Subtle issues (+= with a string for example)
            break;
        StmtKindInc:
            simpleStmtCodeGen(stmt,indentLevel,fp);
            fprintf(fp,";\n");
            break;
        StmtKindDec:
            simpleStmtCodeGen(stmt,indentLevel,fp);
            fprintf(fp,";\n");
            break;

        //Note for Denali, remember that simple statements must not always end in a newline
        StmtKindTypeDeclaration:
            break;
        StmtKindVarDeclaration:
            break;
        StmtKindShortDeclaration: 
            break;
        
    }

    stmtCodeGen(stmt->next,indentLevel,fp);
}