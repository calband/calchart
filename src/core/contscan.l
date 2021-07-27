%{
/*
 * contscan.l
 * Lex scanner for continuity
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CalChartContinuityToken.h"
#include "parse.h"
#include "contgram.hpp"

#if __cplusplus > 199711L
#define register      // Deprecated in C++11.
#endif  // #if __cplusplus > 199711L

/* Globals to communicate with parser */
extern YYSTYPE yylval;	/* lexical value of current token */
YYLTYPE yylloc;		/* line and column of token */
//int yylineno;
int yycolno;

const char *yyinputbuffer;
size_t yyinputbufferlen;

#define YY_NO_UNPUT

#define YY_INPUT(buf,result,max_size) { \
	if (yyinputbufferlen == 0) result = YY_NULL; \
	else if (yyinputbufferlen < max_size) { \
		strcpy(buf, yyinputbuffer); \
		result = yyinputbufferlen; \
		yyinputbufferlen = 0; \
	} else { \
		strncpy(buf, yyinputbuffer, max_size); \
		buf[max_size] = 0; \
		result = max_size; \
		yyinputbuffer += max_size; \
		yyinputbufferlen -= max_size; \
	} \
}

#define ReturnToken(x) \
	yycolno += yyleng; \
	return (x)

void initscanner() {
  yylineno = 1;
  yycolno = 1;
  yyinputbufferlen = strlen(yyinputbuffer);
  if (YY_CURRENT_BUFFER != NULL) yy_flush_buffer(YY_CURRENT_BUFFER);
}

//Supress warnings in Flex's generated code
#ifdef _MSC_VER
#pragma warning (disable : 4018) //Disable the "'<': signed/unsigned mismatch" warning
#pragma warning (disable : 4003) //Disable the "not enough actual parameters for macro 'yywrap'" warning
#endif // _MSC_VER
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wunused-function"
#endif // __clang__
%}

%option noyywrap
%option never-interactive

DIGIT		[0-9]
NUMBER		{DIGIT}+
FLOAT		({DIGIT}+"."{DIGIT}*)|("."{DIGIT}+)
VAR		[ABCDXYZ]
SPACE		[ \t\r]
NEWLINE		[\n\f]
COMMENT		"#"[^\n\f]*

%%
	yylloc.first_line = yylineno;
	yylloc.first_column = yycolno;

{NEWLINE}	{yylineno++; yycolno = 1; return(yylex());}
{SPACE}+	{yycolno += yyleng; return(yylex());}
{COMMENT}	{return(yylex());}

"P"		ReturnToken(rwP);
"NP"		ReturnToken(rwNP);
"R"		ReturnToken(rwR);
"REM"		ReturnToken(rwREM);
"SP"		ReturnToken(rwSP);
"N"		yylval.d = CalChart::CC_N; ReturnToken(DEFINECONST);
"NW"		yylval.d = CalChart::CC_NW; ReturnToken(DEFINECONST);
"W"		yylval.d = CalChart::CC_W; ReturnToken(DEFINECONST);
"SW"		yylval.d = CalChart::CC_SW; ReturnToken(DEFINECONST);
"S"		yylval.d = CalChart::CC_S; ReturnToken(DEFINECONST);
"SE"		yylval.d = CalChart::CC_SE; ReturnToken(DEFINECONST);
"E"		yylval.d = CalChart::CC_E; ReturnToken(DEFINECONST);
"NE"		yylval.d = CalChart::CC_NE; ReturnToken(DEFINECONST);
"DOF"		ReturnToken(rwDOF);
"DOH"		ReturnToken(rwDOH);
"+"		ReturnToken('+');
"-"		ReturnToken('-');
"*"		ReturnToken('*');
"/"		ReturnToken('/');
"("		ReturnToken('(');
")"		ReturnToken(')');
"="		ReturnToken('=');
"HS"		yylval.d = CalChart::CC_HS; ReturnToken(DEFINECONST);
"MM"		yylval.d = CalChart::CC_MM; ReturnToken(DEFINECONST);
"SH"		yylval.d = CalChart::CC_SH; ReturnToken(DEFINECONST);
"JS"		yylval.d = CalChart::CC_JS; ReturnToken(DEFINECONST);
"GV"		yylval.d = CalChart::CC_GV; ReturnToken(DEFINECONST);
"M"		yylval.d = CalChart::CC_M; ReturnToken(DEFINECONST);
"DM"		yylval.d = CalChart::CC_DM; ReturnToken(DEFINECONST);
"BLAM"		ReturnToken(pBLAM);
"CLOSE"		ReturnToken(pMT);
"COUNTERMARCH"	ReturnToken(pCOUNTERMARCH);
"DMCM"		ReturnToken(pDMCM);
"DMHS"		ReturnToken(pDMHS);
"EVEN"		ReturnToken(pEVEN);
"EWNS"		ReturnToken(pEWNS);
"FOUNTAIN"	ReturnToken(pFOUNTAIN);
"FM"		ReturnToken(pFM);
"FMTO"		ReturnToken(pFMTO);
"GRID"		ReturnToken(pGRID);
"HSCM"		ReturnToken(pHSCM);
"HSDM"		ReturnToken(pHSDM);
"MAGIC"		ReturnToken(pMAGIC);
"MARCH"		ReturnToken(pMARCH);
"MT"		ReturnToken(pMT);
"MTRM"		ReturnToken(pMTRM);
"NSEW"		ReturnToken(pNSEW);
"ROTATE"	ReturnToken(pROTATE);
"DIR"		ReturnToken(fDIR);
"DIRFROM"	ReturnToken(fDIRFROM);
"DIST"		ReturnToken(fDIST);
"DISTFROM"	ReturnToken(fDISTFROM);
"EITHER"	ReturnToken(fEITHER);
"OPP"		ReturnToken(fOPP);
"STEP"		ReturnToken(fSTEP);
{NUMBER}	{ yylval.f = (float)atoi(yytext); ReturnToken(FLOATCONST); }
{FLOAT}		{ yylval.f = atof(yytext); ReturnToken(FLOATCONST); }
{VAR}		{ yylval.v = yytext[0]; ReturnToken(VARIABLE); }
.		ReturnToken(UNKNOWN_TOKEN);

%%

//Stop supressing warnings after Flex's generated code
#ifdef _MSC_VER
#pragma warning (default : 4018)
#endif // _MSC_VER
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
