
#include<stdio.h>
#include<string.h>
#include "ast.h"
#include "pretty_printer.h"

void symbolCheckProgram(RootNode* root);
void printSymbolProgram(RootNode* rootNode);
void typeCheckProgram(RootNode* rootNode);

void yyparse();
int yylex();
RootNode * rootNode;
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
	} else if ( !strcmp(argv[1], "pretty") )
	{
		yyparse();
		printRoot(rootNode);
	} else if ( !strcmp(argv[1], "symbol") )
	{
		yyparse();


		symbolCheckProgram(rootNode);
		puts("Got here");
		printSymbolProgram(rootNode);
		

	} else if ( !strcmp(argv[1], "typecheck") )
	{
		yyparse();
		symbolCheckProgram(rootNode);
		typeCheckProgram(rootNode);
		
		printf("OK.\n");
		//symbolchecknoprint
		//typecheck
	} else {
		printf("sorry, I didn't get that\n");
	}
	return 0;
}
