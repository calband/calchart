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

#pragma once

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

class CC_textline
{
private:
	friend class CC_print_continuity;
	CC_textline(std::string line, PSFONT_TYPE& currfontnum);
public:

	CC_textchunk_list GetChunks() const { return chunks; }
	bool GetCenter() const { return center; }
	bool GetOnMain() const { return on_main; }
	bool GetOnSheet() const { return on_sheet; }
private:
	CC_textchunk_list chunks;
	bool center;
	bool on_main;
	bool on_sheet;
};

typedef std::vector<CC_textline> CC_textline_list;

class CC_print_continuity
{
public:
	CC_print_continuity();
	CC_print_continuity(const std::string& data);
	CC_print_continuity(const std::string& number, const std::string& data);
	CC_textline_list GetChunks() const;
	std::string GetOriginalLine() const;
	std::string GetPrintNumber() const;
private:
	CC_textline_list mPrintChunks;
	std::string mOriginalLine;
	std::string mNumber;
};
