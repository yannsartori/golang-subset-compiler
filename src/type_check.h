#ifndef typecheck
#define typecheck

#include "symbol_table.h"

extern Trie* trie;
extern List* globalList;

int LookUpLabel(TTEntry* structEntry);

#endif