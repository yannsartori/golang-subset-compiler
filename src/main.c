#include<stdio.h>
#include<string.h>
#include "ast.h"
//#include "pretty_printer.h"
void yyparse();
int yylex();
//Stmt * root;
int printTokens = 0;
int printSymbols = 0;

int main(int argc, char * argv[])
{
	if ( !strcmp(argv[1], "scan") )
	{
		while(yylex()) {}
		printf("OK\n");
	} else if ( !strcmp(argv[1], "tokens") )
	{
		printTokens = 1;
		while(yylex()) {}
	} else if ( !strcmp(argv[1], "parse") )
	{
		yyparse();
		printf("OK\n");
	} /*else if ( !strcmp(argv[1], "pretty"))
	{
		yyparse();
		prettyStmt(root);
	}*/
	return 0;
}
