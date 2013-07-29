/*
 * cc_text.h
 * textchunk and textline
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#ifndef __CC_TEXT_H__
#define __CC_TEXT_H__

#include "cc_types.h"

#include <string>
#include <vector>

struct CC_textchunk
{
	CC_textchunk() : text(), font(PSFONT_NORM) {}
	std::string text;
	enum PSFONT_TYPE font;
};

typedef std::vector<CC_textchunk> CC_textchunk_list;

struct CC_textline
{
	CC_textline() : center(false), on_main(true), on_sheet(true) {}

	CC_textchunk_list chunks;
	bool center;
	bool on_main;
	bool on_sheet;
};

#endif
