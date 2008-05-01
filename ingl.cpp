/* ingl.cpp
 * Member functions for INGL file format classes
 *
 * Modification history:
 * 3-21-95    Garrick Meeker              Created
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "ingl.h"

#include <string.h>

static char readerr_str[] = "Read error: check disk media";
static char seekerr_str[] = "Seek error: file is corrupted";

INGLchunk::INGLchunk()
  : prev(NULL), data(NULL), userdata(NULL) {}

INGLchunk::~INGLchunk() {
  if (data) delete data;
}

INGLread::INGLread(FILE *file)
  : fp(file), chunks(NULL) {}

INGLread::INGLread(const char *filename) {
  fp = fopen(filename, "rb");
  chunks = NULL;
}

INGLread::~INGLread() {
  INGLchunk *tmpc;

  if (fp) fclose(fp);
  while (chunks != NULL) {
    tmpc = chunks->prev;
    delete chunks;
    chunks = tmpc;
  }
}

bool INGLread::Okay() {
  INGLid id;

  if (fp == NULL) return false;
  if (!ReadLong(&id)) return false;
  if (id != MakeINGLid('I','N','G','L')) return false;
  return true;
}

void *INGLread::ParseFile(INGLhandler hndlrs[], unsigned num, char **error,
			  void *topdata) {
  void *data = NULL;
  INGLchunk *cnk;
  INGLhandler *hndl;
  unsigned i;

  do {
    cnk = new INGLchunk;
    cnk->prev = chunks;
    chunks = cnk;

    if (!ReadLong(&cnk->name)) {
      *error = readerr_str;
      return NULL;
    }
    if (!ReadLong(&cnk->size)) {
      *error = readerr_str;
      return NULL;
    }

    if (cnk->name == MakeINGLid('E','N','D',' ')) {
      PopChunk();
      data = PopChunk();
    } else {
      bool gurkchunk;

      if (cnk->name == MakeINGLid('G','U','R','K')) {
	cnk->name = cnk->size;
	gurkchunk = true;
	// Set first chunk to top level data
	if (cnk->prev == NULL) {
	  cnk->userdata = topdata;
	}
      } else {
	gurkchunk = false;
      }

      hndl = NULL;
      for (i = 0; i < num; i++) {
	if (hndlrs[i].id == cnk->name) {
	  if (chunks->prev == NULL) {
	    hndl = &hndlrs[i];
	    break;
	  } else {
	    if (hndlrs[i].previd == chunks->prev->name) {
	      hndl = &hndlrs[i];
	      break;
	    }
	  }
	}
      }
      
      if (gurkchunk) {
	if (hndl) {
	  *error = hndl->func(cnk);
	  if (*error) return data;
	}
      } else {
	if (hndl) {
	  if (cnk->size > 0) {
	    cnk->data = new unsigned char[cnk->size];
	    if (fread(cnk->data, cnk->size, 1, fp) != 1) {
	      *error = readerr_str;
	      return NULL;
	    }
	  }
	  *error = hndl->func(cnk);
	  if (*error) return data;
	} else {
	  if (fseek(fp, cnk->size, SEEK_CUR) != 0) {
	    *error = seekerr_str;
	    return NULL;
	  }
	}
	data = PopChunk();
      }
    }
  } while (chunks != NULL);

  *error = NULL;
  return data;
}

bool INGLread::ReadLong(INGLid *d) {
  INGLid rawd;

  if (fread(&rawd, 4, 1, fp) != 1) return false;
  *d = get_big_long(&rawd);
  return true;
}

void *INGLread::PopChunk() {
  void *d;
  INGLchunk *cnk;

  if (chunks) {
    cnk = chunks;
    chunks = cnk->prev;
    d = cnk->userdata;
    delete cnk;
    return d;
  } else {
    return NULL;
  }
}

INGLwrite::INGLwrite(FILE *file)
  : fp(file) {}

INGLwrite::INGLwrite(const char *filename) {
  fp = fopen(filename, "wb");
}

INGLwrite::~INGLwrite() {
  if (fp) fclose(fp);
}

bool INGLwrite::WriteHeader() {
  return WriteLong(MakeINGLid('I','N','G','L'));
}

bool INGLwrite::WriteGurk(INGLid name) {
  if (!WriteLong(MakeINGLid('G','U','R','K'))) return false;
  return WriteLong(name);
}

bool INGLwrite::WriteChunkHeader(INGLid name, INGLid size) {
  if (!WriteLong(name)) return false;
  if (!WriteLong(size)) return false;
  return true;
}

bool INGLwrite::WriteChunk(INGLid name, INGLid size, const void *data) {
  if (!WriteLong(name)) return false;
  if (!WriteLong(size)) return false;
  if (size > 0)
    if (fwrite(data, size, 1, fp) != 1) return false;
  return true;
}

bool INGLwrite::WriteChunkStr(INGLid name, const char *str) {
  return WriteChunk(name, strlen(str)+1, (unsigned char *)str);
}

bool INGLwrite::WriteEnd(INGLid name) {
  if (!WriteLong(MakeINGLid('E','N','D',' '))) return false;
  return WriteLong(name);
}

bool INGLwrite::WriteStr(const char *str) {
  if (fwrite(str, strlen(str)+1, 1, fp) != 1) return false;
  return true;
}

bool INGLwrite::Write(const void *data, INGLid size) {
  if (fwrite(data, size, 1, fp) != 1) return false;
  return true;
}

bool INGLwrite::WriteLong(INGLid d) {
  INGLid rawd;

  put_big_long(&rawd, d);
  if (fwrite(&rawd, 4, 1, fp) != 1) return false;
  return true;
}
