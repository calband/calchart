%{
/* contgram.y
 * YACC grammar for continuity
 *
 * Modification history:
 * 10-18-95   Garrick Meeker              Created
 *
 */

#include <wx.h>
#include "cont.h"
#include "parse.h"

//#define YYDEBUG 1

int yyerror(char *s);
extern int yylex();
extern void initscanner();

ContProcedure *ParsedContinuity = NULL;

%}

%{
	/* Reserved word definitions */
%}
%token rwP rwNP rwR rwREM
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
	proclist list;
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
		{ $$.list = NULL; ParsedContinuity = NULL;}
	| proc_list procedure
		{ if ($1.list)
			{$$.list = $1.list; $1.last->next = $2; $$.last = $2;}
		  else {ParsedContinuity = $2; $$.list = $2; $$.last = $2;}}
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
		{ $$ = new ContProcMarch($2, $3, $4); }
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
		    i = 0;
		    break;
		  case 'B':
		    i = 1;
		    break;
		  case 'C':
		    i = 2;
		    break;
		  case 'D':
		    i = 3;
		    break;
		  case 'X':
		    i = 4;
		    break;
		  case 'Y':
		    i = 5;
		    break;
		  case 'Z':
		    i = 6;
		    break;
		  default:
		    i = 7;
		    break;
		  }
		  $$ = new ContValueVar(i);
		}
	;

%%

int parsecontinuity()
{
  initscanner();
//  yydebug = 1;
  return yyparse();
}

int yyerror(char *s)
{
  char tempbuf[256];
  sprintf(tempbuf, "%s near line %d, column %d.\n", 
          s, yylloc.first_line, yylloc.first_column);
  (void)wxMessageBox(tempbuf, "Animate");
  return 0;
}
