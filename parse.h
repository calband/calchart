/* parse.h
 * Classes for parsing continuity
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created
 *
 */

#ifndef _PARSE_H_
#define _PARSE_H_

struct YYLTYPE {
  int first_line;
  int first_column;
};

extern YYLTYPE yylloc;

struct proclist {
  ContProcedure* list;
  ContProcedure* last;
};

#endif
