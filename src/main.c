#include <stdio.h>

#include "AST.h"
#include "pretty.h"


Stmt* root;


int yyparse();



int main(void){
    int n = yyparse();
    if (n == 0){
        puts("OK");
        printStmt(root,0);
    }
}