/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     rwP = 258,
     rwNP = 259,
     rwR = 260,
     rwREM = 261,
     rwSP = 262,
     rwDOF = 263,
     rwDOH = 264,
     pBLAM = 265,
     pCOUNTERMARCH = 266,
     pDMCM = 267,
     pDMHS = 268,
     pEVEN = 269,
     pEWNS = 270,
     pFOUNTAIN = 271,
     pFM = 272,
     pFMTO = 273,
     pGRID = 274,
     pHSDM = 275,
     pHSCM = 276,
     pMAGIC = 277,
     pMARCH = 278,
     pMT = 279,
     pMTRM = 280,
     pNSEW = 281,
     pROTATE = 282,
     fDIR = 283,
     fDIRFROM = 284,
     fDIST = 285,
     fDISTFROM = 286,
     fEITHER = 287,
     fOPP = 288,
     fSTEP = 289,
     UNKNOWN_TOKEN = 290,
     FLOATCONST = 291,
     VARIABLE = 292,
     DEFINECONST = 293,
     UNARY = 294
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 65 "src/contgram.y"

	char v;
	float f;
	ContDefinedValue d;
	proclist list;
	ContProcedure *proc;
	ContPoint *pnt;
	ContValue *value;
	ContValueVar *var;



/* Line 1685 of yacc.c  */
#line 103 "contgram.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


