#include <stdio.h>

#include "ast.h"
#include "pretty_printer.h"


Stmt* root;


int yyparse();



int main(void){
    int n = yyparse();
    if (n == 0){
        puts("OK");
        printStmt(root,0);
    }
}