/* The following are helper functions for the generated code
 * These get loaded up at the beginning of our generated code file
 * by using standard file operations.
 * In our generated code, we can refer to these functions using their
 * signatures, as is normal...
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
    int __capacity;
    __golite_poly_entry *data;
};
__golite_poly_entry * arrGet(__golite_poly_entry * arr, int pos, int length, int lineno)
{
    if ( pos >= length || pos < 0 )
    {
        fprintf(stderr, "Error: (runtime, %d) index out of range [%d] with length %d", lineno, pos, length);
        exit(1);
    }
    return *(arr + pos);
}
char *concat(char *__s1, char *__s2)
{
    char *__cat = (char *) malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
    strcpy(__cat, s1);
    strcat(__cat, s2);
    return __cat;
}
char *stringCast(char c)
{
    char *ret = malloc(sizeof(char) * 2);
    *ret = c;
    *(ret + 1) = '\0';
    return ret;
}