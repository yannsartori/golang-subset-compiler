
%{
   #include "ast.h"
    extern int yylineno;
    #include <stdio.h>
    #include <stdlib.h>
	#include "globalEnum.h"
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
		puts(";"); RETURN( ';' )
	}
}

"break"				{if ( printTokens ) puts("tBreak");			RETURN( tBreak )		}
"default"			{if ( printTokens ) puts("tDefault");		RETURN( tDefault )		}
"func"				{if ( printTokens ) puts("tFunc");			RETURN( tFunc )			}
"interface"			{if ( printTokens ) puts("tInterface");		RETURN( tInterface )	}
"select"			{if ( printTokens ) puts("tSelect");		RETURN( tSelect )		}
"case"				{if ( printTokens ) puts("tCase");			RETURN( tCase )			}
"defer"				{if ( printTokens ) puts("tDefer");			RETURN( tDefer )		}
"go"				{if ( printTokens ) puts("tGo");			RETURN( tGo )			}
"map"				{if ( printTokens ) puts("tMap");			RETURN( tMap )			}
"struct"			{if ( printTokens ) puts("tStruct");		RETURN( tStruct )		}
"chan"				{if ( printTokens ) puts("tChan");			RETURN( tChan )			}
"else"				{if ( printTokens ) puts("tElse");			RETURN( tElse )			}
"goto"				{if ( printTokens ) puts("tGoto");			RETURN( tGoto )			}
"package"			{if ( printTokens ) puts("tPackage");		RETURN( tPackage )		}
"switch"			{if ( printTokens ) puts("tSwitch");		RETURN( tSwitch )		}
"const"				{if ( printTokens ) puts("tConst");			RETURN( tConst )		}
"if"				{if ( printTokens ) puts("tIf");			RETURN( tIf )			}
"range"				{if ( printTokens ) puts("tRange");			RETURN( tRange )		}
"type"				{if ( printTokens ) puts("tType");			RETURN( tType )			}
"continue"			{if ( printTokens ) puts("tContinue");		RETURN( tContinue )		}
"for"				{if ( printTokens ) puts("tFor");			RETURN( tFor )			}
"import"			{if ( printTokens ) puts("tImport");		RETURN( tImport )		}
"return"			{if ( printTokens ) puts("tReturn");		RETURN( tReturn )		}
"var"				{if ( printTokens ) puts("tVar");			RETURN( tVar )			}

"print"				{if ( printTokens ) puts("tPrint");			RETURN( tPrint )		}
"println"			{if ( printTokens ) puts("tPrintln");		RETURN( tPrintln )		}
"append"			{if ( printTokens ) puts("tAppend");		RETURN( tAPPEND )		}
"len"				{if ( printTokens ) puts("tLength");		RETURN( tLENGTH )		}
"cap"				{if ( printTokens ) puts("tCap");			RETURN( tCAP )			}


"+="				{if ( printTokens ) puts("tPlusEq");		RETURN( tPlusEq )		}
"&="				{if ( printTokens ) puts("tAndEq");			RETURN( tAndEq )		}
"&&"				{if ( printTokens ) puts("tLogicAnd");		RETURN( tLOGICAND )		}
"=="				{if ( printTokens ) puts("tEQ");			RETURN( tEQ )			}
"!="				{if ( printTokens ) puts("tNEQ");			RETURN( tNEQ )			}
"("  				{if ( printTokens ) puts("t(");				RETURN( '(' )			}
")"					{if ( printTokens ) puts("t)");				RETURN( ')' )			}
"-"					{if ( printTokens ) puts("t-");				RETURN( '-' )			}	
"|"					{if ( printTokens ) puts("t|");				RETURN( '|' )			}
"-="				{if ( printTokens ) puts("tMinusEquals");	RETURN( tMinusEquals )	}
"|="				{if ( printTokens ) puts("tOrEquals");		RETURN( tOrEquals )		}
"||"				{if ( printTokens ) puts("tLogicOr");		RETURN( tLOGICOR )		}
"<="				{if ( printTokens ) puts("tLEQ");			RETURN( tLEQ )			}
"*="				{if ( printTokens ) puts("tTimesEquals");	RETURN( tTimesEquals )	}
"^="				{if ( printTokens ) puts("tHatEquals");		RETURN( tHatEquals )	}
"<-"				{if ( printTokens ) puts("tLessMinus");		RETURN( tLessMinus )	}
">="				{if ( printTokens ) puts("tGEQ");			RETURN( tGEQ )			}
"<<"				{if ( printTokens ) puts("tBShiftLeft");	RETURN( tBShiftLeft )	}
"/="				{if ( printTokens ) puts("tDivideEquals");	RETURN( tDivideEquals ) }
"<<="				{if ( printTokens ) puts("tLShiftEquals");	RETURN( tLShiftEquals ) }
"++"				{if ( printTokens ) puts("tIncrement");		RETURN( tIncrement )	}
":="				{if ( printTokens ) puts("tDefined");		RETURN( tDefined )		}
">>"				{if ( printTokens ) puts("tBShiftRight");	RETURN( tBShiftRight )	}
"%="				{if ( printTokens ) puts("tModEquals");		RETURN( tModEquals )	}
">>="				{if ( printTokens ) puts("tRShiftEquals");	RETURN( tRShiftEquals ) }
"--"				{if ( printTokens ) puts("tDecrement");		RETURN( tDecrement )	}
"..."				{if ( printTokens ) puts("tElipses");		RETURN( tElipses )		}
"&^"				{if ( printTokens ) puts("tAndNot");		RETURN( tAndNot )		}
"&^="				{if ( printTokens ) puts("tAndHatEquals");	RETURN( tAndHatEquals ) }
"+"  				{if ( printTokens ) puts("t+"); RETURN( '+' ) }
"&"					{if ( printTokens ) puts("t&"); RETURN( '&' ) }
"."					{if ( printTokens ) puts("t."); RETURN( '.' ) }
":"					{if ( printTokens ) puts("t:"); RETURN( ':' ) }
"!"					{if ( printTokens ) puts("t!"); RETURN( '!' ) }
","					{if ( printTokens ) puts("t,"); RETURN( ',' ) }
";"					{if ( printTokens ) puts("t;"); RETURN( ';' ) }
"%"					{if ( printTokens ) puts("t%"); RETURN( '%' ) }
"="					{if ( printTokens ) puts("t="); RETURN( '=' ) }
"{"					{if ( printTokens ) puts("t{"); RETURN( '{' ) }
"}"					{if ( printTokens ) puts("t}"); RETURN( '}' ) }
"/"					{if ( printTokens ) puts("t/"); RETURN( '/' ) }
">"					{if ( printTokens ) puts("t>"); RETURN( '>' ) }
"["					{if ( printTokens ) puts("t["); RETURN( '[' ) }
"]"					{if ( printTokens ) puts("t]"); RETURN( ']' ) }
"*"					{if ( printTokens ) puts("t*"); RETURN( '*' ) }
"^"					{if ( printTokens ) puts("t^"); RETURN( '^' ) }
"<"					{if ( printTokens ) puts("t<"); RETURN( '<' ) }


"//"[^\n]*						{if ( printTokens ) printf("sing-comm\n "); }
"/*"([^\*]|\*[^/]|\n)*"*/"		{if ( printTokens ) printf("multi-comm\n" ); }


"/*" {
	fprintf(stderr, "Error: comment never closed on line %d\n", yylineno);
	exit(1);
}


"'\\"[abfnrtv\\\']"'" {
	yylval.runeval = strdup(yytext);
	if ( printTokens ) printf("tRuneLit(%s)\n", yytext);
	RETURN ( tRUNELIT )
}


"'"[^\'\\]"'" {
 	yylval.runeval = strdup(yytext); 
	if ( printTokens ) printf("tRuneLit(%s)\n", yytext);
	RETURN( tRUNELIT )
}



\"([^\"]|(\\\"))*\" {
	yylval.stringval = strdup(yytext); 
	if ( printTokens ) printf("tInterpretedStringLit(%s)\n", yytext);
	RETURN( tINTERPRETEDSTRINGLIT )
}

\" {
	fprintf(stderr, "Error: unmatched quote found on line %d\n", yylineno);
	exit(1);
}

"`"[^\`]*"`" {
	yylval.stringval = strdup(yytext); 
	if ( printTokens ) printf("tRawStringLit(%s)\n", yytext);
	RETURN( tRAWSTRINGLIT)
}


0|([1-9][0-9]*) {
	yylval.intval = atoi(yytext);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN( tINTLIT )
}

0[oO]([1-7][0-7]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN( tINTLIT )
}

0([1-7][0-7])* {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 8);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN( tINTLIT )
}

0[xX]([1-9a-fA-F][0-9a-fA-F]*) {
	char* ptr;
	yylval.intval = strtol(yytext + 2, &ptr, 16);  
	if ( printTokens ) printf("tIntLit(%s)\n", yytext);
	RETURN( tINTLIT )
}

([0-9]+"."[0-9]*)|"."[0-9]+ {
	yylval.floatval = atof(yytext); 
	if ( printTokens ) printf("tFloatLit(%s)\n", yytext);
	RETURN( tFLOATLIT )
}


[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.identifier = strdup(yytext);  
	if ( printTokens ) printf("tIdentifier(%s)\n", yytext);
	RETURN( tIDENTIFIER )
    
}

              

. {
	fprintf(stderr, "Error: bad character found: %s, line: %d\n", yytext, yylineno); 
	exit(1);
}

%%