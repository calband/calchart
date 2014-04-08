%{





#include "json_file_formatter.h"
#include <string>

//#define YYDEBUG 1

extern int json_lex();
extern void json_initscanner();
int json_error(const char *);

JSONObjectValue *OutputMainObject = nullptr;

%}

%{
	/* Reserved word definitions */
%}

%token OBJECT_OPEN OBJECT_CLOSE
%token ARRAY_OPEN ARRAY_CLOSE
%token SEPARATOR
%token ASSIGNMENT
%token UNKNOWN_TOKEN
%token JNULL


%{
	/* Constants and variables */
%}
%token <stringVal> STRING
%token <intVal> INT
%token <floatVal> FLOAT
%token <boolVal> BOOL

%{
	/* Semantic stack */
%}
%union {
	JSONValue* value;
	std::string* stringVal;
	int intVal;
	float floatVal;
	bool boolVal;
}

%type <value> json_file
%type <value> content
%type <value> object
%type <value> object_content
%type <value> nonempty_object_content
%type <stringVal> binding
%type <value> array
%type <value> array_content
%type <value> nonempty_array_content
%type <value> array_entry
%type <value> boolean
%type <value> number
%type <value> float
%type <value> integer
%type <value> string
%type <value> null

%define api.prefix json_


%%

json_file
	: //Empty
		{OutputMainObject = nullptr;}
	| object 
		{OutputMainObject = (JSONObjectValue*)($1);}
	;

content
	: object
		{$$ = $1;}
	| array
		{$$ = $1;}
	| string
		{$$ = $1;}
	| boolean
		{$$ = $1;}
	| number
		{$$ = $1;}
	| null
		{$$ = $1;}
	;

object
	: OBJECT_OPEN object_content OBJECT_CLOSE
		{$$ = $2;}
	;

object_content
	: //empty
		{$$ = new JSONObjectValue();}
	| nonempty_object_content
		{$$ = $1;}
	;

nonempty_object_content
	: binding content //First Entry, or only entry
		{JSONObjectValue* object = new JSONObjectValue(); $$ = object; object->addValue(*$1, $2); delete $1; }
	| nonempty_object_content SEPARATOR binding content
		{$$ = $1; ((JSONObjectValue*)($$))->addValue(*$3, $4); delete $3;}

binding
	: STRING ASSIGNMENT
		{$$ = $1;}
	;


array
	: ARRAY_OPEN array_content ARRAY_CLOSE
		{$$ = $2;}
	;

array_content
	: //Empty
		{$$ = new JSONArrayValue();}
	| nonempty_array_content
		{$$ = $1;}
	;

nonempty_array_content
	: array_entry
		{JSONArrayValue* array = new JSONArrayValue(); $$ = array; array->addValue($1);}
	| nonempty_array_content SEPARATOR array_entry
		{$$ = $1; ((JSONArrayValue*)($$))->addValue($3);}

array_entry
	: content
		{$$ = $1;}


boolean
	: BOOL
		{$$ = new JSONBooleanValue($1);}
	;

number
	: float
		{$$ = $1;}
	| integer
		{$$ = $1;}
	;

float
	: FLOAT
		{$$ = new JSONFloatValue($1);}
	;

integer
	: INT
		{$$ = new JSONIntValue($1);}
	;

string
	: STRING
		{$$ = new JSONStringValue(*$1); delete $1;}
	;

null
	: JNULL
		{$$ = new JSONNullValue();}
	;

%%

int importJSON()
{
  OutputMainObject = nullptr;
  json_initscanner();
  return json_parse();
}

int json_error(const char *)
{
  return 0;
}