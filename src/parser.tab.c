/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_TAB_C_INCLUDED
# define YY_YY_PARSER_TAB_C_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 77 "parserWithConstructors.y" /* yacc.c:1909  */

	#include "AST.h"
	extern Stmt* root;

#line 49 "parser.tab.c" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    tLOGICOR = 258,
    tLOGICAND = 259,
    tEQ = 260,
    tNEQ = 261,
    tGEQ = 262,
    tLEQ = 263,
    tBShiftLeft = 264,
    tBShiftRight = 265,
    tAndNot = 266,
    tLENGTH = 267,
    tCAP = 268,
    tAPPEND = 269,
    tBreak = 270,
    tDefault = 271,
    tFunc = 272,
    tInterface = 273,
    tSelect = 274,
    tCase = 275,
    tDefer = 276,
    tGo = 277,
    tMap = 278,
    tStruct = 279,
    tChan = 280,
    tElse = 281,
    tGoto = 282,
    tPackage = 283,
    tSwitch = 284,
    tConst = 285,
    tFallthrough = 286,
    tIf = 287,
    tRange = 288,
    tType = 289,
    tContinue = 290,
    tFor = 291,
    tImport = 292,
    tReturn = 293,
    tVar = 294,
    tPrint = 295,
    tPrintln = 296,
    tPlusEq = 297,
    tAndEq = 298,
    tMinusEquals = 299,
    tOrEquals = 300,
    tTimesEquals = 301,
    tHatEquals = 302,
    tLessMinus = 303,
    tDivideEquals = 304,
    tLShiftEquals = 305,
    tIncrement = 306,
    tDefined = 307,
    tModEquals = 308,
    tRShiftEquals = 309,
    tDecrement = 310,
    tElipses = 311,
    tAndHatEquals = 312,
    tINTLIT = 313,
    tFLOATLIT = 314,
    tRUNELIT = 315,
    tRAWSTRINGLIT = 316,
    tINTERPRETEDSTRINGLIT = 317,
    tBOOLVAL = 318,
    tIDENTIFIER = 319,
    UNARY = 320
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 83 "parserWithConstructors.y" /* yacc.c:1909  */

	int intval;
	double floatval;
	char *runeval;
	char *stringval;
	char *identifier;
	struct Exp *exp;
	struct Stmt* stmt;
	struct switchCaseClause* clause;
	struct ExpList *explist;

#line 139 "parser.tab.c" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_C_INCLUDED  */
