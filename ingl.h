/* ingl.h
 * Definitions for classes for the INGL file format (loosely based on IFF)
 * "I Never Get Lost" - Gurk
 *
 * Modification history:
 * 3-21-96    Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1996-2008  Garrick Brian Meeker

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

#ifndef _INGL_H_
#define _INGL_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <wx/defs.h>							  // For basic wx defines

#include "platconf.h"

typedef uint32_t INGLid;

#define MakeINGLid(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

class INGLchunk
{
public:
	INGLchunk();
	~INGLchunk();

	INGLchunk *prev;
	INGLid name;
	INGLid size;
	std::vector<uint8_t> data;
	void *userdata;
};

struct INGLhandler
{
	INGLid id;
	INGLid previd;
	const char* (*func) (INGLchunk* chunk);
};

class INGLread
{
public:
	INGLread(FILE *file);
	INGLread(const char *filename);
	~INGLread();

	bool Okay();

	typedef const char* StringPtr;
	void *ParseFile(INGLhandler hndlr[], unsigned num, StringPtr *error,
		void *topdata = NULL);

private:
	bool ReadLong(INGLid *d);
	void *PopChunk();

	FILE *fp;
	INGLchunk* chunks;
};

class INGLwrite
{
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
