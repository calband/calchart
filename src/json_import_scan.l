%{


#include <string>
#include "json_file_formatter.h"
#include "json_import_gram.hpp"

/* Globals to communicate with parser */
extern JSON_STYPE json_lval;	/* lexical value of current token */



const char *json_inputbuffer;
int json_inputbufferlen;
bool json_isInQuotes = false;

std::string* json_building_string = nullptr;

#define YY_NO_UNPUT

#define YY_INPUT(buf,result,max_size) { \
	if (json_inputbufferlen == 0) result = YY_NULL; \
	else if (json_inputbufferlen < max_size) { \
		strcpy(buf, json_inputbuffer); \
		result = json_inputbufferlen; \
		json_inputbufferlen = 0; \
	} else { \
		strncpy(buf, json_inputbuffer, max_size); \
		buf[max_size] = 0; \
		result = max_size; \
		json_inputbuffer += max_size; \
		json_inputbufferlen -= max_size; \
	} \
}

void json_initscanner() {
  json_lineno = 1;
  json_inputbufferlen = strlen(json_inputbuffer);
  if (YY_CURRENT_BUFFER != NULL) json__flush_buffer(YY_CURRENT_BUFFER);
}

%}

%option prefix="json_"
%option noyywrap
%option never-interactive

DIGIT		[0-9]
NUMBER		{DIGIT}+
FLOAT		({DIGIT}+"."{DIGIT}*)|("."{DIGIT}+)
SPACE	[ \t\r]
NEWLINE		[\n\f]
TEXT		[^"]


%%
{NEWLINE}	{json_lineno++; return json_lex();}
{SPACE}+	{return json_lex();}

"null"			return JNULL;
"True"			{json_lval.boolVal = true; return BOOL; };
"False"			{json_lval.boolVal = false; return BOOL; };
"{"				return OBJECT_OPEN;
"}"				return OBJECT_CLOSE;
"["				return ARRAY_OPEN;
"]"				return ARRAY_CLOSE;
":"				return ASSIGNMENT;
","				return SEPARATOR;
{NUMBER}		{ json_lval.intVal = atoi(json_text); return INT; }
{FLOAT}			{ json_lval.floatVal = atof(json_text); return FLOAT; }
{TEXT}+			{if (!json_isInQuotes) REJECT; (*json_building_string) += json_text; return json_lex(); };
"\\."			{
					if (json_isInQuotes) {
						(*json_building_string) += json_text;
						return json_lex();
					} else {
						REJECT;
					}
				}
"\""			{
					if (json_isInQuotes) {
						json_isInQuotes = false;
						json_lval.stringVal = json_building_string;
						return STRING;
					} else {
						json_isInQuotes = true;
						json_building_string = new std::string();
						return json_lex();
					}
				}
.				return UNKNOWN_TOKEN;

%%

