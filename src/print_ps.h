/*
 * print.cpp
 * Handles all postscript printing to a file stream
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

#ifndef __PRINT_PS_H__
#define __PRINT_PS_H__

#include "cc_types.h"

#include <iostream>

//#include <time.h>
//#include <string>
//#include <sstream>
//
//// from http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
//// there is no snprint on visual c++ 2010
//#if _MSC_VER
//#define snprintf _snprintf
//#endif
//
//static float width, height, real_width, real_height;
//static float field_x, field_y, field_w, field_h;
//static float stage_field_x, stage_field_y, stage_field_w, stage_field_h;
//static float step_size;
//static float spr_step_size;
//static short num_pages;
//static short step_width, step_offset;
//static Coord max_s, max_n, clip_s, clip_n;
//static short split_sheet;

class CC_sheet;
class CC_show;
struct CC_textline;

class PrintShowToPS
{
public:
	PrintShowToPS(const CC_show&, bool print_landscape, bool print_do_cont, bool print_do_cont_sheet);

	int operator()(std::ostream& buffer, bool eps, bool overview, unsigned curr_ss, int min_yards);

private:
	void PrintSheets(std::ostream& buffer, short& num_pages);
	void PrintCont(std::ostream& buffer, const CC_sheet& sheet);
	void PrintStandard(std::ostream& buffer, const CC_sheet& sheet);
	void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet);
	void PrintOverview(std::ostream& buffer, const CC_sheet& sheet);
	void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE *currfontnum, float fontsize);
	void print_start_page(std::ostream& buffer, bool landscape);

	const CC_show& mShow;
	bool mPrintLandscape;
	bool mPrintDoCont;
	bool mPrintDoContSheet;
	float width, height, real_width, real_height;
	float field_x, field_y, field_w, field_h;
	float stage_field_x, stage_field_y, stage_field_w, stage_field_h;
	float step_size;
	float spr_step_size;
	short step_width, step_offset;
	Coord max_s, max_n, clip_s, clip_n;
	short split_sheet;
};

#endif // __PRINT_PS_H__

