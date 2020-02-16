%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "parser.tab.h"
    
    #define RETURN(token) lastToken = token; return token;
    
    int lastToken = 0;
    
    int needsSemiColon() {
		return lastToken == tIDENTIFIER ||
		lastToken == tINTLIT ||
		lastToken == tFLOATLIT ||
		lastToken == tRUNELIT ||
		lastToken == tINTERPRETEDSTRINGLIT ||
		lastToken == tRAWSTRINGLIT ||
		lastToken == ']' ||
		lastToken == tIncrement ||
		lastToken == tDecrement ||
		lastToken == '}' ||
		lastToken == ')';
	}
%}

%option yylineno

%%

[ \t\r]+

\n {
	if (needsSemiColon()) {
		RETURN( ';' )
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


"//"[^\n]*				printf("sing-comm\n ");
"/*"([^\*]|\*[^/]|\n)*"*/"		printf("multi-comm\n" );


"/*" {
	fprintf(stderr, "Error: comment never closed on line %d\n", yylineno);
	exit(1);
}


"'\\"[abfnrtv\\\']"'" {
	yylval.runeval = strdup(yytext);
	RETURN( tRUNELIT )
}


"'"[^\'\\]"'" {
 	yylval.runeval = strdup(yytext); 
	RETURN( tRUNELIT )
}



\"([^\"]|(\\\"))*\" {
	yylval.stringval = strdup(yytext); 
	RETURN( tINTERPRETEDSTRINGLIT )
}

\" {
	fprintf(stderr, "Error: unmatched quote found on line %d\n", yylineno);
	exit(1);
}

"`"[^\`]*"`" {
	yylval.stringval = strdup(yytext); 
	RETURN( tRAWSTRINGLIT)
}


0|([1-9][0-9]*) {
	yylval.intval = atoi(yytext);  
	RETURN( tINTLIT )
}

0[oO]([1-7][0-7]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	RETURN( tINTLIT )
}

0([1-7][0-7])* {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	RETURN( tINTLIT )
}

0[xX]([1-9a-fA-F][0-9a-fA-F]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 16);  
	RETURN( tINTLIT )
}

([0-9]+"."[0-9]*)|"."[0-9]+ {
	yylval.floatval = atof(yytext); 
	RETURN( tFLOATLIT )
}


[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.identifier = strdup(yytext);  
	RETURN( tIDENTIFIER )
    
}

              

. {
	fprintf(stderr, "Error: bad character found: %s, line: %d\n", yytext, yylineno); 
	exit(1);
}

%%

int main(){
	yylex();
	return 1;
}
