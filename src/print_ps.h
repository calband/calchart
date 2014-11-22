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
//#include "confgr.h"

#include <iostream>
#include <set>
#include <functional>

class CC_sheet;
class CC_show;
class CalChartDoc;
class CC_textline;
class CalChartConfiguration;
class ShowMode;

// intented to print a show to a buffer.
class PrintShowToPS
{
public:
//	PrintShowToPS(const CC_show&, const CalChartConfiguration& config_, bool print_landscape, bool print_do_cont, bool print_do_cont_sheet);
	PrintShowToPS(const CC_show&, bool PrintLandscape, bool PrintDoCont, bool PrintDoContSheet, std::string const& head_font_str_, std::string const& main_font_str_, std::string const& number_font_str_, std::string const& cont_font_str_, std::string const& bold_font_str_, std::string const& ital_font_str_, std::string const& bold_ital_font_str_, double PageWidth, double PageHeight, double PageOffsetX, double PageOffsetY, double PaperLength, double HeaderSize, double YardsSize, double TextSize, double DotRatio, double NumRatio, double PLineRatio, double SLineRatio, double ContRatio, std::function<std::string(size_t)> Get_yard_text, std::function<std::string(size_t)> Get_spr_line_text);
//	PrintShowToPS(const CC_show&);//, bool PrintLandscape, bool PrintDoCont, bool PrintDoContSheet, std::string const& head_font_str_, std::string const& main_font_str_, std::string const& number_font_str_, std::string const& cont_font_str_, std::string const& bold_font_str_, std::string const& ital_font_str_, std::string const& bold_ital_font_str_, double PageWidth, double PageHeight, double PageOffsetX, double PageOffsetY, double PaperLength, double HeaderSize, double YardsSize, double TextSize, double DotRatio, double NumRatio, double PLineRatio, double SLineRatio, double ContRatio);//, std::function<std::string(size_t)> Get_yard_text, std::function<std::string(size_t)> Get_spr_line_text);

	int operator()(std::ostream& buffer, bool eps, bool overview, unsigned curr_ss, int min_yards, const std::set<size_t>& isPicked, ShowMode const& mode, std::string const& title);

private:
	void PrintSheets(std::ostream& buffer, short& num_pages);
	void PrintCont(std::ostream& buffer, const CC_sheet& sheet);
	void PrintStandard(std::ostream& buffer, const CC_sheet& sheet, ShowMode const& mode);
	void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet, ShowMode const& mode);
	void PrintOverview(std::ostream& buffer, const CC_sheet& sheet, ShowMode const& mode);
	void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE *currfontnum, float fontsize);
	void print_start_page(std::ostream& buffer, bool landscape);

	const CC_show& mShow;
	bool mPrintLandscape;
	bool mPrintDoCont;
	bool mPrintDoContSheet;

	std::string head_font_str;
	std::string main_font_str;
	std::string number_font_str;
	std::string cont_font_str;
	std::string bold_font_str;
	std::string ital_font_str;
	std::string bold_ital_font_str;

	double mPageWidth;
	double mPageHeight;
	double mPageOffsetX;
	double mPageOffsetY;
	double mPaperLength;

	double mHeaderSize;
	double mYardsSize;
	double mTextSize;
	double mDotRatio;
	double mNumRatio;
	double mPLineRatio;
	double mSLineRatio;
	double mContRatio;

	std::function<std::string(size_t)> mGet_yard_text;
	std::function<std::string(size_t)> mGet_spr_line_text;

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

