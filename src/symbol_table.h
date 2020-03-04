#ifndef SYMBOLH
#define SYMBOLH
#define TABLE_SIZE 997
#include "globalEnum.h"
//typedefs in globalEnum
struct Context {
	TypeTable *curTypeTable;
	SymbolTable *curSymbolTable;
	Context *parent;
};
struct SymbolTable {
	STEntry *entries[TABLE_SIZE];
};
struct TypeTable {
	TTEntry *entries[TABLE_SIZE];
};
struct PolymorphicEntry { // helper. Used for the getEntry method.
	int isSymbol;
	union {
		STEntry *s;
		TTEntry *t;
	} entry;
};
struct STEntry {
	char *id;
	TTEntry *type; //points to a location in the TypeTable of the appropriate scope ofc
	STEntry *next; //Bucket Collisions
};
struct TTEntry { //base types have id = type and basetype = basetype to simplify (hopefully) checks in the future
	char *id;
	TypeType underlyingTypeType; //array struct slice etc TODO discuss if we shoudl add funcDeclType to TypeType (it is currently not) 
	BaseType underlyingType; //an enum int float etc
	union {
		struct {TTEntry *type;} normalType; //encompasses slice, array, boring type
		struct {TTEntryList *fields;} structType; 
		struct {TTEntryList *args; TTEntry *ret;} functionType;
	} val;
	TTEntry *next;
};
struct TTEntryList {
	TTEntry *cur;
	TTEntryList *next;
};
void symbolCheckExpression(Exp *e, SymbolTable *s, TypeTable *t);
#endif
