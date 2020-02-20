%{
    #include <stdio.h>
    #include <stdlib.h>
	#include "globalEnum.h"
    #include "parser.tab.h"
    
    #define RETURN(token) if ( printTokens ) puts(#token); lastToken = token; return token;
    #define RETURN_NO_PRINT(token) lastToken = token; return token;
	
    int lastToken = 0;
    
    int needsSemiColon() {
		return lastToken == tIDENTIFIER ||
		lastToken == tINTLIT ||
		lastToken == tFLOATLIT ||
		lastToken == tRUNELIT ||
		lastToken == tINTERPRETEDSTRINGLIT ||
		lastToken == tRAWSTRINGLIT ||
		lastToken == tBreak ||
		lastToken == tContinue ||
		lastToken == tFallthrough ||
		lastToken == tReturn ||
		lastToken == tIncrement ||
		lastToken == tDecrement ||
		lastToken == ']' ||
		lastToken == '}' ||
		lastToken == ')';
	}
%}

%option yylineno

%%

[ \t\r]+

\n {
	if (needsSemiColon()) {
		puts(";"); RETURN( ';' )
	}
}


"break"			RETURN( tBreak )
"default"			RETURN( tDefault )
"func"				RETURN( tFunc )
"interface"			RETURN( tInterface )
"select"			RETURN( tSelect )
"case"				RETURN( tCase )
"defer"			RETURN( tDefer )
"go"				RETURN( tGo )
"map"				RETURN( tMap )
"struct"			RETURN( tStruct )
"chan"				RETURN( tChan )
"else"				RETURN( tElse )
"goto"				RETURN( tGoto )
"package"			RETURN( tPackage )
"switch"			RETURN( tSwitch )
"const"			RETURN( tConst )
"fallthrough"			RETURN( tFallthrough )
"if"				RETURN( tIf )
"range"			RETURN( tRange )
"type"				RETURN( tType )
"continue"			RETURN( tContinue )
"for"				RETURN( tFor )
"import"			RETURN( tImport )
"return"			RETURN( tReturn )
"var"				RETURN( tVar )


"print"			RETURN( tPrint )
"println"			RETURN( tPrintln )
"append"			RETURN( tAPPEND )
"len"				RETURN( tLENGTH )
"cap"				RETURN( tCAP )


"+="				RETURN( tPlusEq )
"&="				RETURN( tAndEq )
"&&"				RETURN( tLOGICAND )
"=="				RETURN( tEQ )
"!="				RETURN( tNEQ )
"("  				RETURN( '(' )
")"				RETURN( ')' )
"-"				RETURN( '-' )
"|"				RETURN( '|' )
"-="				RETURN( tMinusEquals )
"|="				RETURN( tOrEquals )
"||"				RETURN( tLOGICOR )
"<="				RETURN( tLEQ )
"*="				RETURN( tTimesEquals )
"^="				RETURN( tHatEquals )
"<-"				RETURN( tLessMinus )
">="				RETURN( tGEQ )
"<<"				RETURN( tBShiftLeft )
"/="				RETURN( tDivideEquals )
"<<="				RETURN( tLShiftEquals )
"++"				RETURN( tIncrement )
":="				RETURN( tDefined )
">>"				RETURN( tBShiftRight )
"%="				RETURN( tModEquals )
">>="				RETURN( tRShiftEquals )
"--"				RETURN( tDecrement )
"..."				RETURN( tElipses )
"&^"				RETURN( tAndNot )
"&^="				RETURN( tAndHatEquals )
"+"  				RETURN( '+' )
"&"				RETURN( '&' )
"."				RETURN( '.' )
":"				RETURN( ':' )
"!"				RETURN( '!' )
","				RETURN( ',' )
";"				RETURN( ';' )
"%"				RETURN( '%' )
"="				RETURN( '=' )
"{"				RETURN( '{' )
"}"				RETURN( '}' )
"/"				RETURN( '/' )
">"				RETURN( '>' )
"["				RETURN( '[' )
"]"				RETURN( ']' )
"*"				RETURN( '*' )
"^"				RETURN( '^' )
"<"				RETURN( '<' )

"//"[^\n]*				{if ( printTokens ) printf("sing-comm\n"); }
"/*"([^\*]|\*[^/]|\n)*"*/"		{if ( printTokens ) printf("multi-comm\n" ); }


"/*" {
	fprintf(stderr, "Error: comment never closed on line %d\n", yylineno);
	exit(1);
}


"'\\"[abfnrtv\\\']"'" {
	yylval.runeval = strdup(yytext);
	if ( printTokens ) printf("tRuneLit(%s)\n", yytext);
	RETURN_NO_PRINT ( tRUNELIT )
}


"'"[^\'\\]"'" {
 	yylval.runeval = strdup(yytext); 
	if ( printTokens ) printf("tRuneLit(%s)\n", yytext);
	RETURN_NO_PRINT( tRUNELIT )
}



\"([^\"]|(\\\"))*\" {
	yylval.stringval = strdup(yytext); 
	if ( printTokens ) printf("tInterpretedStringLit(%s)\n", yytext);
	RETURN_NO_PRINT( tINTERPRETEDSTRINGLIT )
}

\" {
	fprintf(stderr, "Error: unmatched quote found on line %d\n", yylineno);
	exit(1);
}

"`"[^\`]*"`" {
	yylval.stringval = strdup(yytext); 
	if ( printTokens ) printf("tRawStringLit(%s)\n", yytext);
	RETURN_NO_PRINT( tRAWSTRINGLIT)
}


0|([1-9][0-9]*) {
	yylval.intval = atoi(yytext);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN_NO_PRINT( tINTLIT )
}

0[oO]([1-7][0-7]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN_NO_PRINT( tINTLIT )
}

0([1-7][0-7])* {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN_NO_PRINT( tINTLIT )
}

0[xX]([1-9a-fA-F][0-9a-fA-F]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 16);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN_NO_PRINT( tINTLIT )
}

([0-9]+"."[0-9]*)|"."[0-9]+ {
	yylval.floatval = atof(yytext); 
	if ( printTokens ) printf("tFloatLit(%s)\n", yytext);
	RETURN_NO_PRINT( tFLOATLIT )
}


[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.identifier = strdup(yytext);  
	if ( printTokens ) printf("tIdentifier(%s)\n", yytext);
	RETURN_NO_PRINT( tIDENTIFIER )
    
}

              

. {
	fprintf(stderr, "Error: bad character found: %s, line: %d\n", yytext, yylineno); 
	exit(1);
}

%%

