%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"
    #include "minil.tab.h"
%}

%option yylineno

%%

[ \t\r\n]+

"break"				return tBreak;
"default"			return tDefault;
"func"				return tFunc;
"interface"			return tInterface;
"select"			return tSelect;
"case"				return tCase;
"defer"				return tDefer;
"go"				return tGo;
"map"				return tMap;
"struct"			return tStruct;
"chan"				return tChan;
"else"				return tElse;
"goto"				return tGoto;
"package"			return tPackage;
"switch"			return tSwitch;
"const"				return tConst;
"fallthrough"		return tFallthrough;
"if"				return tIf;
"range"				return tRange;
"type"				return tType;
"continue"			return tContinue;
"for"				return tFor;
"import"			return tImport;
"return"			return tReturn;
"var"				return tVar;

"print"				return tPrint;
"println"			return tPrintln;
"append"			return tAPPEND;
"len"				return tLENGTH;
"cap"				return tCAP;

"+"  				return '+';
"&"					return '&';
"+="				return tPlusEq;
"&="				return tAndEq;
"&&"				return tLOGICAND;
"=="				return tEQ;
"!="				return tNEQ;
"("  				return '(';
")"					return ')';
"-"					return '-';
"|"					return '|';
"-="				return tMinusEquals;
"|="				return tOrEquals;
"||"				return tLOGICOR;
"<"					return '<';
"<="				return tLEQ;
"["					return '[';
"]"					return ']';
"*"					return '*';
"ˆ"					return '^';
"*="				return tTimesEquals;
"ˆ="				return tHatEquals;
"<-"				return tLessMinus;
">"					return '>';
">="				return tGEQ;
"{"					return '{';
"}"					return '}';
"/"					return '/';
"<<"				return tBShiftLeft;
"/="				return tDivideEquals;
"<<="				return tLShiftEquals;
"++"				return tIncrement;
"="					return '=';
":="				return tDefined;
","					return ',';
";"					return ';';
"%"					return '%';
">>"				return tBShiftRight;
"%="				return tModEquals;
">>="				return tRShiftEquals;
"--"				return tDecrement;
"!"					return '!';
"..."				return tElipses;
"."					return '.';
":"					return ':';
"&ˆ"				return tAndNot;
"&ˆ="				return tAndHatEquals;

"//"[^\n]*				/* Comments */
"/*"([^*]|*[^/])*"*/"	/* Comments */

"/*" {
	fprintf(stderr, "Error: comment never closed on line %d\n", yylineno);
	exit(1);
}


"'"("\\"[abfnrtv\\\'])"'" {
	yylval.runeval = strdup(yytext);
	return tRUNELIT;
}


"'"[^\'\\]"'" {
	yylval.runeval = strdup(yytext);
	return tRUNELIT;
}



\"([^"]|(\\\"))*\" {
    yylval.stringval = strdup(yytext);
    return tINTERPRETEDSTRINGLIT;
}

\" {
    fprintf(stderr, "Error: unmatched quote found on line %d\n", yylineno);
    exit(1);
}

`[^`]` {
	yylval.stringval = strdup(yytext);
	return tRAWSTRINGLIT;
}


0|([1-9][0-9]*) {
    yylval.intval = atoi(yytext); 
    return tINTLIT;
}

0[oO]([1-7][0-7]*) {
	char* ptr;
    yylval.intval = strtol(yytext + 2, &ptr, 8); 
    return tINTLIT;
}

0[xX]([1-9a-fA-F][0-9a-fA-F]*) {
	char* ptr;
    yylval.intval = strtol(yytext + 2, &ptr, 16); 
    return tINTLIT;
}

([0-9]+"."[0-9]*)|"."[0-9]+ {
    yylval.floatval = atof(yytext);
    return tFLOATLIT;
}


[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.identifier = strdup(yytext); 
    return tIDENTIFIER;
}

              

. {
    fprintf(stderr, "Error: bad character found: %s, line: %d\n", yytext, yylineno); 
    exit(1);
}

%%