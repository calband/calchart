/*
 * show.h
 * Definitions for the show classes
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/defs.h>							  // For basic wx defines
#include <wx/string.h>
#include <wx/list.h>

#include "cc_types.h"
#include "platconf.h"
#include "linmath.h"
#include "cc_show.h"

#include <vector>
#include <deque>

class CC_sheet;
class CC_coord;
class CC_show;

#define BASICALLY_ZERO_BASICALLY 0.00001
#define IS_ZERO(a) (ABS((a)) < BASICALLY_ZERO_BASICALLY)
#define DEG2RAD(a) ((a) * PI / 180.0)
#define SQRT2 1.4142136

#define MAX_POINTS 1000
#define NUM_REF_PNTS 3

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


float BoundDirection(float f);

float BoundDirectionSigned(float f);

bool IsDiagonalDirection(float f);

void CreateVector(CC_coord& c, float dir, float mag);

void CreateUnitVector(float& a, float& b, float dir);

#define DEF_HASH_W 32
#define DEF_HASH_E 52

void SetAutoSave(int secs);
#endif
