%{
/* contscan.l
 * Lex scanner for continuity
 *
 * Modification history:
 * 10-18-95   Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#include "cont.h"
#include "parse.h"
#include "contgram.h"

/* Globals to communicate with parser */
extern YYSTYPE yylval;	/* lexical value of current token */
YYLTYPE yylloc;		/* line and column of token */
int yylineno;
int yycolno;

const char *yyinputbuffer;
int yyinputbufferlen;

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

%}

%option noyywrap

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
"N"		yylval.d = CC_N; ReturnToken(DEFINECONST);
"NW"		yylval.d = CC_NW; ReturnToken(DEFINECONST);
"W"		yylval.d = CC_W; ReturnToken(DEFINECONST);
"SW"		yylval.d = CC_SW; ReturnToken(DEFINECONST);
"S"		yylval.d = CC_S; ReturnToken(DEFINECONST);
"SE"		yylval.d = CC_SE; ReturnToken(DEFINECONST);
"E"		yylval.d = CC_E; ReturnToken(DEFINECONST);
"NE"		yylval.d = CC_NE; ReturnToken(DEFINECONST);
"DOF"		ReturnToken(rwDOF);
"DOH"		ReturnToken(rwDOH);
"+"		ReturnToken('+');
"-"		ReturnToken('-');
"*"		ReturnToken('*');
"/"		ReturnToken('/');
"("		ReturnToken('(');
")"		ReturnToken(')');
"="		ReturnToken('=');
"HS"		yylval.d = CC_HS; ReturnToken(DEFINECONST);
"MM"		yylval.d = CC_MM; ReturnToken(DEFINECONST);
"SH"		yylval.d = CC_SH; ReturnToken(DEFINECONST);
"JS"		yylval.d = CC_JS; ReturnToken(DEFINECONST);
"GV"		yylval.d = CC_GV; ReturnToken(DEFINECONST);
"M"		yylval.d = CC_M; ReturnToken(DEFINECONST);
"DM"		yylval.d = CC_DM; ReturnToken(DEFINECONST);
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

