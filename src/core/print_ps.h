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

#pragma once

#include "cc_types.h"

#include <iostream>
#include <set>
#include <functional>
#include <array>

class CC_sheet;
class CC_show;
class CC_textline;
class ShowMode;

// intented to print a show to a buffer.
// fonts are the list of fonts in this order:
//std::string head_font_str;
//std::string main_font_str;
//std::string number_font_str;
//std::string cont_font_str;
//std::string bold_font_str;
//std::string ital_font_str;
//std::string bold_ital_font_str;

class PrintShowToPS
{
public:
	PrintShowToPS(const CC_show&, bool PrintLandscape, bool PrintDoCont, bool PrintDoContSheet, bool PrintOverview, int min_yards, ShowMode const& mode, std::array<std::string, 7> const& fonts_, double PageWidth, double PageHeight, double PageOffsetX, double PageOffsetY, double PaperLength, double HeaderSize, double YardsSize, double TextSize, double DotRatio, double NumRatio, double PLineRatio, double SLineRatio, double ContRatio, std::function<std::string(size_t)> Get_yard_text, std::function<std::string(size_t)> Get_spr_line_text);

	int operator()(std::ostream& buffer, bool eps, unsigned curr_ss, const std::set<size_t>& isPicked, std::string const& title) const;

private:
	short PrintContinuitySheets(std::ostream& buffer, short num_pages) const;
	void PrintContSections(std::ostream& buffer, const CC_sheet& sheet) const;
	void PrintStandard(std::ostream& buffer, const CC_sheet& sheet, bool split_sheet) const;
	void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet) const;
	void PrintOverview(std::ostream& buffer, const CC_sheet& sheet) const;
	void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE currfontnum, float fontsize) const;
	void print_start_page(std::ostream& buffer, bool landscape, double translate_x, double translate_y) const;
	bool SplitSheet(const CC_sheet& sheet) const;
	void PrintHeader(std::ostream& buffer, bool eps, const std::string& title) const;
	void PrintFieldDefinition(std::ostream& buffer) const;
	void PrintTrailer(std::ostream& buffer, short num_pages) const;
	short PrintSheets(std::ostream& buffer, bool eps, unsigned curr_ss, const std::set<size_t>& isPicked, short num_pages) const;

	const CC_show& mShow;
	bool mPrintLandscape;
	bool mPrintDoCont;
	bool mPrintDoContSheet;
	bool mOverview;
	ShowMode const& mMode;

	std::array<std::string, 7> fonts;

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
	short step_width;
};