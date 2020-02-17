#include <stdio.h>


int yyparse();

int main(void){
    int n = yyparse();
    if (n == 0){
        puts("OK");
    }
}