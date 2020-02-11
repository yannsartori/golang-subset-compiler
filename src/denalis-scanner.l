%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"
    #include "minil.tab.h"
%}

%option yylineno

%%

[ \t\r\n]+

var                 return tVar;
string              return tString;
else                return tElse;
print               return tPrint;
float               return tFloat;
bool                return tBool;
while               return tWhile;
True                return tTrue;
int                 return tInt;
if                  return tIf;
read                return tRead;
False               return tFalse;

"+"                 return '+';
"-"                 return '-';
"*"                 return '*';
"/"                 return '/';
"="                 return '=';
"=="                return '~';
"!="                return tNeq;
"<"                 return '<';
">"                 return '>';
"<="                return tLeq;
">="                return tGeq;
"&&"                return '&';
"||"                return '|';
"("                 return '(';
")"                 return ')';
"!"                 return '!';
"{"                 return '{';
"}"                 return '}';
":"                 return ':';
";"                 return ';';

\"[^"]*\" {
    yylval.quoted = strdup(yytext);
    return tQuote;
}

\" {
    fprintf(stderr, "Error: unmatched quote found on line %d\n", yylineno);
    exit(1);
}

0|([1-9][0-9]*) {
    yylval.intval = atoi(yytext); 
    return tInt;
}

0[0-9]+ {
    fprintf(stderr, "Error: Leading zeros not allowed in: %s, line: %d\n", yytext, yylineno); 
    exit(1);
}

(0|([1-9][0-9]*))"."[0-9]+ {
    yylval.floatval = atof(yytext);
    return tFloat;
}

("."[0-9]+)|(0|([1-9][0-9]*))"."  {
    fprintf(stderr, "Error: bad string found: %s, line: %d\nIf you were trying to declare a float, you need digits on both sides of the decimal\n", yytext, yylineno); 
    exit(1);
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.identifier = strdup(yytext); 
    return tId;
}

"#"[^\n]*               

. {
    fprintf(stderr, "Error: bad character found: %s, line: %d\n", yytext, yylineno); 
    exit(1);
}

%%
