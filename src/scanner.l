%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.tab.h"
#include "main.h"
int characterOnLine = 0;
char curLine[500];
void incrementCharacter() { 
	characterOnLine += strlen(yytext); 
	strcat(curLine, yytext);	
}
void clearLine() {
	characterOnLine = 0;
	strcpy(curLine, "");
}
void error(char *code)
{
	fprintf(stderr, "Error: %s (line %d, %d-%zu)\n", code, yylineno, characterOnLine, characterOnLine + strlen(yytext));
	fprintf(stderr, "%s%s\n", curLine, yytext);
	for ( int i = 0; i < characterOnLine - 1; i++ )
	{
		fputc(' ', stderr);
	}
	for ( int i = characterOnLine - 1; i < characterOnLine + strlen(yytext); i++ )
	{
		fputc('^', stderr);
	}
	fputc('\n', stderr);
	exit(1);
}
%}

%option yylineno

%%

[ \t\r]+		 incrementCharacter();
#.*\n|\n		 clearLine();
"var"			{incrementCharacter();  if (printTokens) printf("tVAR\n");		return tVAR;		}
"float"			{incrementCharacter();  if (printTokens) printf("tFLOAT\n");	return tFLOAT;		}
"int"			{incrementCharacter();  if (printTokens) printf("tINT\n");		return tINT;		}
"string"		{incrementCharacter();  if (printTokens) printf("tSTRING\n");	return tSTRING;		}
"bool"			{incrementCharacter();  if (printTokens) printf("tBOOL\n");		return tBOOL;		}
"if"			{incrementCharacter();  if (printTokens) printf("tIF\n");		return tIF;			}
"else"			{incrementCharacter();  if (printTokens) printf("tELSE\n");		return tELSE;		}
"while"			{incrementCharacter();  if (printTokens) printf("tWHILE\n");	return tWHILE;		}
"read"			{incrementCharacter();  if (printTokens) printf("tREAD\n");		return tREAD;		}
"print"			{incrementCharacter();  if (printTokens) printf("tPRINT\n");	return tPRINT;		}
"True"			{
	incrementCharacter();  
	yylval.boolval = 1;
	if (printTokens) printf("tBOOL(True)\n"); 
	return tBOOLVAL;   
}
"False"			{
	incrementCharacter();  
	yylval.boolval = 0;
	if (printTokens) printf("tBOOL(False)\n"); 
	return tBOOLVAL;  
}
0|[1-9][0-9]*	{
	incrementCharacter();
	yylval.intval = atoi(yytext);
	if (printTokens) printf("tINTLIT(%s)\n", yytext); 
	return tINTLIT;
}
0\.[0-9]+|[1-9][0-9]*\.[0-9]+	{
	incrementCharacter();
	yylval.floatval = atof(yytext);
	if (printTokens) printf("tFLOATLIT(%s)\n", yytext); 
	return tFLOATLIT;
}
[0-9]*\.|\.[0-9]*  {
	error("Both sides of decimal must have digits.");
}
[0-9]*\.[0-9]*|[0-9]* {
	error("Leading zeroes not permitted.");
}
\"[^"]*\"		{
	incrementCharacter();
	char * stringVal = (char *) malloc(sizeof(char) * strlen(yytext));
	memcpy(stringVal, (yytext + 1), strlen(yytext) - 2);
	yylval.stringval = stringVal;
	if (printTokens) printf("tSTRINGLIT(%s)\n", yytext); 
	return tSTRINGLIT;
}
"+"  | 
"-"	 |
"/"  |
"*"	 |
"="	 |
"!"  |
"("  |
")"  |
"{"  |
"}"  |
"<"  |
">"  |
"#"  |
":"  |
";"		{incrementCharacter(); if (printTokens) printf("t%c\n", yytext[0]);	return yytext[0];	}
"=="	{incrementCharacter(); if (printTokens) printf("tEQ\n");			return tEQ;			}
"!="	{incrementCharacter(); if (printTokens) printf("tNEQ\n");			return tNEQ;		}	
">="	{incrementCharacter(); if (printTokens) printf("tGEQ\n");			return tGEQ;		}
"<="	{incrementCharacter(); if (printTokens) printf("tLEQ\n");			return tLEQ;		}	
"&&"	{incrementCharacter(); if (printTokens) printf("tLOGICAND\n");		return tLOGICAND;	}
"||"	{incrementCharacter(); if (printTokens) printf("tLOGICOR\n");		return tLOGICOR;	}
[a-zA-Z_][a-zA-Z0-9_]*	{
	incrementCharacter();
	yylval.identifier = strdup(yytext);
	if (printTokens) printf("tIDENTIFIER(%s)\n", yytext); 
	return tIDENTIFIER;
}
. {
	error("Unexpected character.");
}
