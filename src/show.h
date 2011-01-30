/* show.h
 * Definitions for the show classes
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 * 4-16-95    Garrick Meeker              Converted to C++
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#ifndef _SHOW_H_
#define _SHOW_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/defs.h>							  // For basic wx defines
#include <wx/string.h>
#include <wx/list.h>

#include "cc_types.h"
#include "cc_winlist.h"
#include "platconf.h"
#include "linmath.h"
#include "cc_show.h"

#include <vector>
#include <deque>

class CC_sheet;
class CC_show;

#define COORD_SHIFT 4
#define COORD_DECIMAL (1<<COORD_SHIFT)
#define INT2COORD(a) ((a) * COORD_DECIMAL)
#define COORD2INT(a) ((a) / COORD_DECIMAL)
#define FLOAT2NUM(a) CLIPFLOAT((a) * (1 << COORD_SHIFT))
#define FLOAT2COORD(a) (Coord)FLOAT2NUM((a))
#define COORD2FLOAT(a) ((a) / ((float)(1 << COORD_SHIFT)))
#define CLIPFLOAT(a) (((a) < 0) ? ((a) - 0.5) : ((a) + 0.5))

#define BASICALLY_ZERO_BASICALLY 0.00001
#define IS_ZERO(a) (ABS((a)) < BASICALLY_ZERO_BASICALLY)
#define DEG2RAD(a) ((a) * PI / 180.0)
#define SQRT2 1.4142136

#define MAX_POINTS 1000
#define NUM_REF_PNTS 3

class wxWindow;
class wxDC;
class CC_show;

struct cc_oldcoord
{
	uint16_t x;
	uint16_t y;
};

/* Format of old calchart stuntsheet files */
#define OLD_FLAG_FLIP 1
struct cc_oldpoint
{
	uint8_t sym;
	uint8_t flags;
	cc_oldcoord pos;
	uint16_t color;
	int8_t code[2];
	uint16_t cont;
	cc_oldcoord ref[3];
};

struct cc_reallyoldpoint
{
	uint8_t sym;
	uint8_t flags;
	cc_oldcoord pos;
	uint16_t color;
	int8_t code[2];
	uint16_t cont;
	int16_t refnum;
	cc_oldcoord ref;
};

extern const wxChar *contnames[];

class CC_textchunk
{
public:
	wxString text;
	enum PSFONT_TYPE font;
};

typedef std::vector<CC_textchunk> CC_textchunk_list;

class CC_textline
{
public:
	CC_textline();
	~CC_textline();

	CC_textchunk_list chunks;
	bool center;
	bool on_main;
	bool on_sheet;
};

class CC_coord
{
public:
	CC_coord(Coord xval = 0, Coord yval = 0) : x(xval), y(yval) {}
	CC_coord(const cc_oldcoord& old) { *this = old; }

	float Magnitude() const;
	float DM_Magnitude() const;				  // check for diagonal military also
	float Direction() const;
	float Direction(const CC_coord& c) const;

	bool Collides(const CC_coord& c) const;

	CC_coord& operator = (const cc_oldcoord& old);
	inline CC_coord& operator += (const CC_coord& c)
	{
		x += c.x; y += c.y;
		return *this;
	}
	inline CC_coord& operator -= (const CC_coord& c)
	{
		x -= c.x; y -= c.y;
		return *this;
	}
	inline CC_coord& operator *= (short s)
	{
		x *= s; y *= s;
		return *this;
	}
	inline CC_coord& operator /= (short s)
	{
		x /= s; y /= s;
		return *this;
	}

	Coord x, y;
};
inline CC_coord operator + (const CC_coord& a, const CC_coord& b)
{
	return CC_coord(a.x + b.x, a.y + b.y);
}


inline CC_coord operator - (const CC_coord& a, const CC_coord& b)
{
	return CC_coord(a.x - b.x, a.y - b.y);
}


inline CC_coord operator * (const CC_coord& a, short s)
{
	return CC_coord(a.x * s, a.y * s);
}


inline CC_coord operator / (const CC_coord& a, short s)
{
	return CC_coord(a.x / s, a.y / s);
}


inline CC_coord operator - (const CC_coord& c)
{
	return CC_coord(-c.x, -c.y);
}


inline int operator == (const CC_coord& a, const CC_coord& b)
{
	return ((a.x == b.x) && (a.y == b.y));
}


inline int operator == (const CC_coord& a, const short b)
{
	return ((a.x == b) && (a.y == b));
}


inline int operator != (const CC_coord& a, const CC_coord& b)
{
	return ((a.x != b.x) || (a.y != b.y));
}


inline int operator != (const CC_coord& a, const short b)
{
	return ((a.x != b) || (a.y != b));
}


float BoundDirection(float f);

float BoundDirectionSigned(float f);

bool IsDiagonalDirection(float f);

void CreateVector(CC_coord& c, float dir, float mag);

void CreateUnitVector(float& a, float& b, float dir);

#define DEF_HASH_W 32
#define DEF_HASH_E 52

class CC_descr
{
public:
	CC_show *show;
	unsigned curr_ss;
	inline CC_sheet *CurrSheet() const { return show->GetNthSheet(curr_ss); }
};

void SetAutoSave(int secs);
#endif
