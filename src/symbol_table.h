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
	int isConstant;			//0 for variables, 1 for constants and 2 for functions
};
struct TTEntry { //base types have id = type and basetype = basetype to simplify (hopefully) checks in the future
	char *id;
	TypeType underlyingType; //array struct slice etc TODO discuss if we shoudl add funcDeclType to TypeType (it is currently not)
	union {
		struct {BaseType type;} nonCompositeType; //int float etc 
		struct {TTEntry *type;} sliceType; //if slice []T, returns T
		struct {TTEntry *type; int size;} arrayType; //if arr [n]T, n == size, T == type
		struct {Context *fields; IdChain* fieldNames;} structType; // (id, type)1, (id,type)2, ...
		struct {STEntry *args; TTEntry *ret;} functionType; //(Type1, Type2, ...) : TypeRet
	} val;
	int comparable;
	TTEntry *next;
};
PolymorphicEntry *getEntry(Context *c, char *id);


typedef struct Entry{
	char* id;
	TTEntry* type;
}Entry;



typedef enum{
	EntryNode,
	LabelNode
}TrieType;

typedef struct Trie{
	TrieType genre;
	union{
		Entry* entry;
		int label;
	}variant;

	struct Trie* sibling;
	struct Trie* child;
}Trie;

typedef struct List{
	Trie** structChain;
	int size;
}List;

Trie* encodeRoot(RootNode* root);

#endif
