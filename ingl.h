/* ingl.h
 * Definitions for classes for the INGL file format (loosely based on IFF)
 *
 * Modification history:
 * 3-21-96    Garrick Meeker              Created
 *
 */

#ifndef _INGL_H_
#define _INGL_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx_defs.h> // For basic wx defines

#include "platconf.h"

typedef unsigned long INGLid;

#define MakeINGLid(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

class INGLchunk {
public:
  INGLchunk();
  ~INGLchunk();

  INGLchunk *prev;
  INGLid name;
  INGLid size;
  void *data;
  void *userdata;
};

struct INGLhandler {
  INGLid id;
  INGLid previd;
  char* (*func) (INGLchunk* chunk);
};

class INGLread {
public:
  INGLread(FILE *file);
  INGLread(const char *filename);
  ~INGLread();

  Bool Okay();

  void *ParseFile(INGLhandler hndlr[], unsigned num, char **error,
		  void *topdata = NULL);

private:
  Bool ReadLong(INGLid *d);
  void *PopChunk();

  FILE *fp;
  INGLchunk* chunks;
};

class INGLwrite {
public:
  INGLwrite(FILE *file);
  INGLwrite(const char *filename);
  ~INGLwrite();

  inline Bool Okay() { return (fp != NULL); }

  Bool WriteHeader();
  Bool WriteGurk(INGLid name);
  Bool WriteChunkHeader(INGLid name, INGLid size);
  Bool WriteChunk(INGLid name, INGLid size, const void *data);
  Bool WriteChunkStr(INGLid name, const char *str);
  Bool WriteEnd(INGLid name);

  // Write raw data
  Bool WriteStr(const char *str);
  Bool Write(const void *data, INGLid size);
private:
  Bool WriteLong(INGLid d);

  FILE *fp;
};

#endif
