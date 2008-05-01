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

#include <wx/defs.h> // For basic wx defines

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

  bool Okay();

  void *ParseFile(INGLhandler hndlr[], unsigned num, char **error,
		  void *topdata = NULL);

private:
  bool ReadLong(INGLid *d);
  void *PopChunk();

  FILE *fp;
  INGLchunk* chunks;
};

class INGLwrite {
public:
  INGLwrite(FILE *file);
  INGLwrite(const char *filename);
  ~INGLwrite();

  inline bool Okay() { return (fp != NULL); }

  bool WriteHeader();
  bool WriteGurk(INGLid name);
  bool WriteChunkHeader(INGLid name, INGLid size);
  bool WriteChunk(INGLid name, INGLid size, const void *data);
  bool WriteChunkStr(INGLid name, const char *str);
  bool WriteEnd(INGLid name);

  // Write raw data
  bool WriteStr(const char *str);
  bool Write(const void *data, INGLid size);
private:
  bool WriteLong(INGLid d);

  FILE *fp;
};

#endif
