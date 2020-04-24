/* The following are helper functions for the generated code
 * These get loaded up at the beginning of our generated code file
 * by using standard file operations.
 * In our generated code, we can refer to these functions using their
 * signatures, as is normal...
*/

/* Also, depending on things go, we might want to have *another* header
 * file, which contains our typedefs and function headers, since c isn't
 * groovy and doesn't allow for function calls before they are declared.
 * This is primarily arising from the fear that we write stuff in a weird order
 * Regardless, this shouldn't be too grave of an issue to resolve
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct __golite_slice __golite_slice;
typedef union __golite_poly_entry __golite_poly_entry;
union __golite_poly_entry { 
    int intVal;
    double floatVal;
    char runeVal;
    char * stringVal;
    void * polyVal;
};
struct __golite_slice {
    int size;
    int capacity;
    __golite_poly_entry **arrPointer;
};
/*Make sure this works!*/
__golite_poly_entry createPolyInt(int x)
{
    __golite_poly_entry p;
    p.intVal = x;
    return p;
}
__golite_poly_entry createPolyFloat(double x)
{
    __golite_poly_entry p;
    p.floatVal = x;
    return p;
}
__golite_poly_entry createPolyRune(char x)
{
    __golite_poly_entry p;
    p.runeVal = x;
    return p;
}
//I think doing pointer copying is fine for the following. Test!
__golite_poly_entry createPolyString(char * x)
{
    __golite_poly_entry p;
    p.stringVal = x;
    return p;
}
__golite_poly_entry createPolyVoid(void * x)
{
    __golite_poly_entry p;
    p.polyVal = x;
    return p;
}
__golite_poly_entry *arrCopy(__golite_poly_entry *arr, char *typeChain, int arrLength);

__golite_slice *append(__golite_slice *slice, __golite_poly_entry elem, char *typeChain)
{
    __golite_slice *newSlice = (__golite_slice *) malloc(sizeof(__golite_slice));

    if ( slice == NULL || slice->capacity == 0 )
    {
        newSlice->size = 1;
        newSlice->capacity = 2;
        newSlice->arrPointer = (__golite_poly_entry **) malloc(sizeof(__golite_poly_entry *));
        *(newSlice->arrPointer) = (__golite_poly_entry *) malloc(sizeof(__golite_poly_entry) * newSlice->capacity);
        *(*(newSlice->arrPointer) + 0) = elem; 
    }
    else if ( slice->size < slice->capacity )
    {
        *(*(slice->arrPointer) + slice->size) = elem;
        newSlice->size = slice->size + 1;
        newSlice->capacity = slice->capacity;
        newSlice->arrPointer = (__golite_poly_entry **) malloc(sizeof(__golite_poly_entry *));
        *(newSlice->arrPointer) = *(slice->arrPointer);
    }
    else 
    {
        newSlice->size = slice->size + 1;
        newSlice->capacity = slice->capacity * 2;
        newSlice->arrPointer = (__golite_poly_entry **) malloc(sizeof(__golite_poly_entry *));
       // *(newSlice->arrPointer) = (__golite_poly_entry *) malloc(sizeof(__golite_poly_entry) * newSlice->capacity);
        *(newSlice->arrPointer) = arrCopy(*(slice->arrPointer), typeChain, slice->size);
        *(newSlice->arrPointer) = realloc(*(newSlice->arrPointer), sizeof(__golite_poly_entry) * newSlice->capacity);
        *(*(newSlice->arrPointer) + slice->capacity) = elem;
    }
    return newSlice;
}
__golite_poly_entry arrGet(__golite_poly_entry * arr, int pos, int length, int lineno)
{
    if ( pos >= length || pos < 0 )
    {
        fprintf(stderr, "Error: (runtime, %d) index out of range [%d] with length %d\n", lineno, pos, length);
        exit(1);
    }
    return *(arr + pos);
}
__golite_poly_entry sliceGet(__golite_slice *slice, int pos, int lineno)
{
    if ( pos >= slice->size || pos < 0 )
    {
        fprintf(stderr, "Error: (runtime, %d) index out of range [%d] with length %d\n", lineno, pos, slice->size);
        exit(1);
    }
    return *(*(slice->arrPointer) + pos);
}
void arrSet(__golite_poly_entry * arr, int pos, int length, int lineno, __golite_poly_entry e)
{
    arrGet(arr, pos, length, lineno); //does outofbounds check
    *(arr + pos) = e;
}
void sliceSet(__golite_slice *slice, int pos, int lineno, __golite_poly_entry e)
{
    sliceGet(slice, pos, lineno);
    *(*slice->arrPointer + pos) = e;
}
int structEquality(void * struct1, void * struct2, char * structName);
void * structCopy(void * struct1, char * structName);
char *concat(char *__s1, char *__s2);
char *stringCast(char c);

char *concat(char *s1, char *s2)
{
    char *cat = (char *) malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
    strcpy(cat, s1);
    strcat(cat, s2);
    return cat;
}
char *stringCast(char c)
{
    char *ret = malloc(sizeof(char) * 2);
    *ret = c;
    *(ret + 1) = '\0';
    return ret;
}
__golite_poly_entry *arrCopy(__golite_poly_entry *arr, char *typeChain, int arrLength)
{
    //AY->Array, ST->Struct, SG->String, RE->rune, IR->Integer, FT->Float
    __golite_poly_entry *newArr = (__golite_poly_entry *) malloc(sizeof(__golite_poly_entry) * arrLength);
    char code[3];
    char size[10];
    char structName[100];
    code[0] = typeChain[0];
    code[1] = typeChain[1];
    code[2] = '\0';

    int comparisonMode = 0;
    int curPos = 2; 
    if ( strcmp(code, "AY") == 0 )
    {
        do
        {
            size[curPos - 2] = *(typeChain + curPos);
            curPos++;
        } while ( '0' <= *(typeChain + curPos) && *(typeChain + curPos) <= '9');
        size[curPos - 2] = '\0';
        comparisonMode = 1;
    } else if ( strcmp(code, "ST") == 0 )
    {
        do
        {
            structName[curPos - 2] = *(typeChain + curPos);
            curPos++;
        } while ( *(typeChain + curPos) != '\0');
        structName[curPos - 2] = '\0';
        comparisonMode = 2;
    }
    if ( comparisonMode == 0 )
    {
        for ( int i = 0; i < arrLength; i++ )
        {
            newArr[i] = arr[i];
        }
    } else if ( comparisonMode == 1 )
    {
        char * newType = malloc(sizeof(char) * (strlen(typeChain) - curPos + 1));
        strcpy(newType, typeChain + curPos);
        for ( int i = 0; i < arrLength; i++ )
        {
            newArr[i].polyVal = arrCopy((__golite_poly_entry *) arr[i].polyVal, newType, atoi(size)); 
        }
        free(newType);
    } else if ( comparisonMode == 2 )
    {
        for ( int i = 0; i < arrLength; i++ )
        {
            newArr[i].polyVal = structCopy(arr[i].polyVal, structName); //we can put the header rn.
        }
    }
    return newArr;
}
int arrEquality(__golite_poly_entry *arr1, __golite_poly_entry *arr2, char *typeChain, int arrLength)
{
    //AY->Array, ST->Struct, SE->Slice, SG->String, RE->rune, IR->Integer, FT->Float
    char code[3];
    char size[10];
    char structName[100];
    code[0] = typeChain[0];
    code[1] = typeChain[1];
    code[2] = '\0';

    int comparisonMode = 0;
    int curPos = 2; 
    //we don't worry about slices becuase they would have been rejected by typechecker
    if ( strcmp(code, "IR") == 0 ) comparisonMode = 0;
    else if ( strcmp(code, "RE") == 0 ) comparisonMode = 1;
    else if ( strcmp(code, "FT") == 0 ) comparisonMode = 2;
    else if ( strcmp(code, "SG") == 0 ) comparisonMode = 3;
    else if ( strcmp(code, "ST") == 0 )
    {
        do
        {
            structName[curPos - 2] = *(typeChain + curPos);
            curPos++;
        } while ( *(typeChain + curPos) != '\0');
        structName[curPos - 2] = '\0';
        comparisonMode = 4;
    } else if ( strcmp(code, "AY") == 0 )
    {
        do
        {
            size[curPos - 2] = *(typeChain + curPos);
            curPos++;
        } while ( '0' <= *(typeChain + curPos) && *(typeChain + curPos) <= '9');
        size[curPos - 2] = '\0';
        comparisonMode = 5;
    }
    //I think doing the comparison outside the for loop makes the code slightly more efficient
    //albeit gross
    switch( comparisonMode )
    {
        case 0:
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( arr1[i].intVal != arr2[i].intVal ) return 0;
        }
        break;

        case 1:
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( arr1[i].runeVal  != arr2[i].runeVal  ) return 0;
        }
        break;
        
        case 2:
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( arr1[i].floatVal  != arr2[i].floatVal  ) return 0;
        }
        break;
        
        case 3:
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( strcmp(arr1[i].stringVal, arr2[i].stringVal) != 0 ) return 0;
        }
        break;
        
        case 4:
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( !structEquality(arr1[i].polyVal, arr2[i].polyVal, structName) ) return 0;
        }
        break;
        
        case 5:;
        char * newType = malloc(sizeof(char) * (strlen(typeChain) - curPos + 1));
        strcpy(newType, typeChain + curPos);
        for ( int i = 0; i < arrLength; i++ )
        {
            if ( !arrEquality((__golite_poly_entry *) (arr1[i].polyVal), (__golite_poly_entry *) (arr2[i].polyVal), newType, atoi(size)) ) {
                free(newType);
                return 0;
            }
        }
        free(newType);
        break;
    }
    return 1;
    
}