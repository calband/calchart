/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
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

#ifndef YY_JSON_GENERATED_JSON_IMPORT_GRAM_HPP_INCLUDED
# define YY_JSON_GENERATED_JSON_IMPORT_GRAM_HPP_INCLUDED
/* Enabling traces.  */
#ifndef JSON_DEBUG
# if defined YYDEBUG
#  if YYDEBUG
#   define JSON_DEBUG 1
#  else
#   define JSON_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define JSON_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined JSON_DEBUG */
#if JSON_DEBUG
extern int json_debug;
#endif

/* Tokens.  */
#ifndef JSON_TOKENTYPE
# define JSON_TOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum json_tokentype {
     OBJECT_OPEN = 258,
     OBJECT_CLOSE = 259,
     ARRAY_OPEN = 260,
     ARRAY_CLOSE = 261,
     SEPARATOR = 262,
     ASSIGNMENT = 263,
     UNKNOWN_TOKEN = 264,
     JNULL = 265,
     STRING = 266,
     INT = 267,
     FLOAT = 268,
     BOOL = 269
   };
#endif


#if ! defined JSON_STYPE && ! defined JSON_STYPE_IS_DECLARED
typedef union JSON_STYPE
{
/* Line 2053 of yacc.c  */
#line 43 "src/json_import_gram.y"

	JSONValue* value;
	std::string* stringVal;
	int intVal;
	float floatVal;
	bool boolVal;


/* Line 2053 of yacc.c  */
#line 88 "generated/json_import_gram.hpp"
} JSON_STYPE;
# define JSON_STYPE_IS_TRIVIAL 1
# define json_stype JSON_STYPE /* obsolescent; will be withdrawn */
# define JSON_STYPE_IS_DECLARED 1
#endif

extern JSON_STYPE json_lval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int json_parse (void *YYPARSE_PARAM);
#else
int json_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int json_parse (void);
#else
int json_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_JSON_GENERATED_JSON_IMPORT_GRAM_HPP_INCLUDED  */
