/* ingl.cc
 * Member functions for INGL file format classes
 *
 * Modification history:
 * 3-21-95    Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "ingl.h"

#include <string.h>

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
  if (fp) fclose(fp);
}

Bool INGLread::Okay() {
  INGLid id;

  if (fp == NULL) return FALSE;
  if (!ReadLong(&id)) return FALSE;
  if (id != MakeINGLid('I','N','G','L')) return FALSE;
  return TRUE;
}

void *INGLread::ParseFile(INGLhandler hndlrs[], unsigned num, char **error) {
  void *data = NULL;
  INGLchunk *cnk;
  INGLhandler *hndl;
  unsigned i;

  do {
    cnk = new INGLchunk;
    cnk->prev = chunks;
    chunks = cnk;

    ReadLong(&cnk->name);
    ReadLong(&cnk->size);

    if (cnk->name == MakeINGLid('E','N','D',' ')) {
      PopChunk();
      data = PopChunk();
    } else {
      Bool gurkchunk;

      if (cnk->name == MakeINGLid('G','U','R','K')) {
	cnk->name = cnk->size;
	gurkchunk = TRUE;
      } else {
	gurkchunk = FALSE;
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
	    fread(cnk->data, cnk->size, 1, fp);
	  }
	  *error = hndl->func(cnk);
	  if (*error) return data;
	} else {
	  fseek(fp, cnk->size, SEEK_CUR);
	}
	data = PopChunk();
      }
    }
  } while (chunks != NULL);

  *error = NULL;
  return data;
}

Bool INGLread::ReadLong(INGLid *d) {
  INGLid rawd;

  if (fread(&rawd, 4, 1, fp) != 1) return FALSE;
  *d = get_big_long(&rawd);
  return TRUE;
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

Bool INGLwrite::WriteHeader() {
  return WriteLong(MakeINGLid('I','N','G','L'));
}

Bool INGLwrite::WriteGurk(INGLid name) {
  if (!WriteLong(MakeINGLid('G','U','R','K'))) return FALSE;
  return WriteLong(name);
}

Bool INGLwrite::WriteChunkHeader(INGLid name, INGLid size) {
  if (!WriteLong(name)) return FALSE;
  if (!WriteLong(size)) return FALSE;
  return TRUE;
}

Bool INGLwrite::WriteChunk(INGLid name, INGLid size, const void *data) {
  if (!WriteLong(name)) return FALSE;
  if (!WriteLong(size)) return FALSE;
  if (size > 0)
    if (fwrite(data, size, 1, fp) != 1) return FALSE;
  return TRUE;
}

Bool INGLwrite::WriteChunkStr(INGLid name, const char *str) {
  return WriteChunk(name, strlen(str)+1, (unsigned char *)str);
}

Bool INGLwrite::WriteEnd(INGLid name) {
  if (!WriteLong(MakeINGLid('E','N','D',' '))) return FALSE;
  return WriteLong(name);
}

Bool INGLwrite::WriteStr(const char *str) {
  if (fwrite(str, strlen(str)+1, 1, fp) != 1) return FALSE;
  return TRUE;
}

Bool INGLwrite::Write(const void *data, INGLid size) {
  if (fwrite(data, size, 1, fp) != 1) return FALSE;
  return TRUE;
}

Bool INGLwrite::WriteLong(INGLid d) {
  INGLid rawd;

  put_big_long(&rawd, d);
  if (fwrite(&rawd, 4, 1, fp) != 1) return FALSE;
  return TRUE;
}
