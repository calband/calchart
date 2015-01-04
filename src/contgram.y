%{
/* contgram.y
 * YACC grammar for continuity
 *
 * Modification history:
 * 10-18-95   Garrick Meeker              Created
 *
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

#include "cont.h"
#include "parse.h"
#include "animate.h"

#include <list>

//#define YYDEBUG 1

int yyerror(const char *s);
extern int yylex();
extern void initscanner();

std::list<std::unique_ptr<ContProcedure>> ParsedContinuity;

%}

%{
	/* Reserved word definitions */
%}
%token rwP rwNP rwR rwREM rwSP rwDOF rwDOH
%token pBLAM pCOUNTERMARCH pDMCM pDMHS pEVEN pEWNS pFOUNTAIN pFM pFMTO
%token pGRID pHSDM pHSCM pMAGIC pMARCH pMT pMTRM pNSEW pROTATE
%token fDIR fDIRFROM fDIST fDISTFROM fEITHER fOPP fSTEP
%token UNKNOWN_TOKEN

%{
	/* Constants and variables */
%}
%token <f> FLOATCONST
%token <v> VARIABLE
%token <d> DEFINECONST

%{
	/* Operator definitions */
%}
%token '+' '-' '*' '/' '(' ')' '='

%{
	/* Semantic stack */
%}
%union {
	char v;
	float f;
	ContDefinedValue d;
	ContProcedure *proc;
	ContPoint *pnt;
	ContValue *value;
	ContValueVar *var;
}

%type <list> proc_list
%type <proc> procedure
%type <pnt> point
%type <value> value
%type <value> function
%type <var> varvalue

%right UNARY    /* a psuedo terminal used only for unary - precedence */
%left '+' '-' '*' '/'

%%

proc_list
	: // Empty
		{}
	| proc_list procedure
		{ ParsedContinuity.emplace_back($2); }
	;

procedure
	: varvalue '=' value
		{ $$ = new ContProcSet($1, $3); }
	| pBLAM
		{ $$ = new ContProcBlam(); }
	| pCOUNTERMARCH point point value value value value
		{ $$ = new ContProcCM($2, $3, $4, $5, $6, $7); }
	| pDMCM point point value
		{ $$ = new ContProcDMCM($2, $3, $4); }
	| pDMHS point
		{ $$ = new ContProcDMHS($2); }
	| pEVEN value point
		{ $$ = new ContProcEven($2, $3); }
	| pEWNS point
		{ $$ = new ContProcEWNS($2); }
	| pFOUNTAIN value value point
		{ $$ = new ContProcFountain($2, $3, NULL, NULL, $4); }
	| pFOUNTAIN value value value value point
		{ $$ = new ContProcFountain($2, $3, $4, $5, $6); }
	| pFM value value
		{ $$ = new ContProcFM($2, $3); }
	| pFMTO point
		{ $$ = new ContProcFMTO($2); }
	| pGRID value
		{ $$ = new ContProcGrid($2); }
	| pHSCM point point value
		{ $$ = new ContProcHSCM($2, $3, $4); }
	| pHSDM point
		{ $$ = new ContProcHSDM($2); }
	| pMAGIC point
		{ $$ = new ContProcMagic($2); }
	| pMARCH value value value
		{ $$ = new ContProcMarch($2, $3, $4, NULL); }
	| pMARCH value value value value
		{ $$ = new ContProcMarch($2, $3, $4, $5); }
	| pMT value value
		{ $$ = new ContProcMT($2, $3); }
	| pMTRM value
		{ $$ = new ContProcMTRM($2); }
	| pNSEW point
		{ $$ = new ContProcNSEW($2); }
	| pROTATE value value point
		{ $$ = new ContProcRotate($2, $3, $4); }
	;

point
	: rwP
		{ $$ = new ContPoint(); }
	| rwSP
		{ $$ = new ContStartPoint(); }
	| rwNP
		{ $$ = new ContNextPoint(); }
	| rwR FLOATCONST
		{ $$ = new ContRefPoint((unsigned)$2 - 1); }
	;

value
	: FLOATCONST
		{ $$ = new ContValueFloat($1); }
	| DEFINECONST
		{ $$ = new ContValueDefined($1); }
	| value '+' value
		{ $$ = new ContValueAdd($1, $3); }
	| value '-' value
		{ $$ = new ContValueSub($1, $3); }
	| value '*' value
		{ $$ = new ContValueMult($1, $3); }
	| value '/' value
		{ $$ = new ContValueDiv($1, $3); }
	| '-' value %prec UNARY
		{ $$ = new ContValueNeg($2); }
	| '(' value ')'
		{ $$ = $2; }
	| rwREM
		{ $$ = new ContValueREM(); }
	| rwDOF
		{ $$ = new ContValueVar(CONTVAR_DOF); }
	| rwDOH
		{ $$ = new ContValueVar(CONTVAR_DOH); }
	| varvalue
		{ $$ = $1; }
	| function
		{ $$ = $1; }
	;

function
	: fDIR '(' point ')'
		{ $$ = new ContFuncDir($3); }
	| fDIRFROM '(' point point ')'
		{ $$ = new ContFuncDirFrom($3, $4); }
	| fDIST '(' point ')'
		{ $$ = new ContFuncDist($3); }
	| fDISTFROM '(' point point ')'
		{ $$ = new ContFuncDistFrom($3, $4); }
	| fEITHER '(' value value point ')'
		{ $$ = new ContFuncEither($3, $4, $5); }
	| fOPP '(' value ')'
		{ $$ = new ContFuncOpp($3); }
	| fSTEP '(' value value point ')'
		{ $$= new ContFuncStep($3, $4, $5); }
	;

varvalue
	: VARIABLE
		{ unsigned i;
		  switch ($1) {
		  case 'A':
		  case 'a':
		    i = 0;
		    break;
		  case 'B':
		  case 'b':
		    i = 1;
		    break;
		  case 'C':
		  case 'c':
		    i = 2;
		    break;
		  case 'D':
		  case 'd':
		    i = 3;
		    break;
		  case 'X':
		  case 'x':
		    i = 4;
		    break;
		  case 'Y':
		  case 'y':
		    i = 5;
		    break;
		  case 'Z':
		  case 'z':
		  default:
		    i = 6;
		    break;
		  }
		  $$ = new ContValueVar(i);
		}
	;

%%

int parsecontinuity()
{
  ParsedContinuity.clear();
  initscanner();
//  yydebug = 1;
  return yyparse();
}

int yyerror(const char *)
{
  // handled outside
  return 0;
}
