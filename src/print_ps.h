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
#include <set>

class CC_sheet;
class CC_show;
class CalChartDoc;
struct CC_textline;

/**
 * Prints a show to a buffer.
 */
class PrintShowToPS
{
public:
	/**
	 * Creates an object that is prepared to pring a show to a buffer.
	 * @param show The document to print.
	 * @param print_landscape True if the document should be printed
	 * with a landscape orientation; false if it should be printed
	 * with a portrait orientation.
	 * @param print_do_cont True if a sheet's continuities should be
	 * printed with the sheet.
	 * @param print_do_cont_sheet True if a sheet should be printed
	 * having descriptions of all of the continuities in the show.
	 */
	PrintShowToPS(const CalChartDoc& show, bool print_landscape, bool print_do_cont, bool print_do_cont_sheet);

	/**
	 * Prints the show to a buffer.
	 * @param buffer The output buffer to print the show to.
	 * @param eps True if the format of the output should be encapsulated
	 * postscript; false if the format should just be postscript.
	 * @param overview True if an overview of the show should be printed.
	 * @param curr_ss The index of the first sheet to print. The sheet
	 * with this index and all sheets after it will be printed.
	 * @param min_yards The minimum number of yards that should appear in
	 * printout. These will be centered about the 50-yard line.
	 * @param isPicked A set containing the indices of the stunt sheets that
	 * should be printed.
	 */
	int operator()(std::ostream& buffer, bool eps, bool overview, unsigned curr_ss, int min_yards, const std::set<size_t>& isPicked);

private:
	/**
	 * Prints the continuity summary sheets.
	 * These sheets contain a list of all of the continuities in the show.
	 * @param buffer The output buffer to which the pages will be printed.
	 * @param num_pages The number of pages to set asside to print the
	 * continuities to.
	 */
	void PrintSheets(std::ostream& buffer, short& num_pages);
	/**
	 * Prints the continuities for a sheet to the output buffer.
	 * @param buffer The output buffer to which the continuities
	 * will be printed.
	 * @param sheet The sheet whose continuities should be printed.
	 */
	void PrintCont(std::ostream& buffer, const CC_sheet& sheet);
	/**
	 * Prints a sheet of a show with a standard show mode to the output
	 * buffer.
	 * @param buffer The buffer to which the sheet should be printed.
	 * @param sheet The sheet that should be printed.
	 */
	void PrintStandard(std::ostream& buffer, const CC_sheet& sheet);
	/**
	 * Prints a sheet of a show with a springshow show mode to the output
	 * buffer.
	 * @param buffer The buffer to which the sheet should be printed.
	 * @param sheet The sheet that should be printed.
	 */
	void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet);
	/**
	 * Prints an the a sheet of the show into the overview.
	 * @param buffer The buffer to print the sheet to.
	 * @param sheet The sheet to print to the overview.
	 */
	void PrintOverview(std::ostream& buffer, const CC_sheet& sheet);
	/**
	 * Formats and prints a line of a continuity to the output buffer.
	 * @param buffer The buffer to which the continuity should be printed.
	 * @param line A line of continuity text.
	 * @param currfontnum The font to print the continuity text in.
	 * @param fontsize The size at which to print the continuity text.
	 */
	void gen_cont_line(std::ostream& buffer, const CC_textline& line, PSFONT_TYPE *currfontnum, float fontsize);
	/**
	 * Formats a new page in the output buffer.
	 * @param buffer The output buffer to which the new page will be printed.
	 * @param landscape True if the new page will be printed in landscape
	 * orientation, false otherwise.
	 */
	void print_start_page(std::ostream& buffer, bool landscape);

	/**
	 * The document to print.
	 */
	const CalChartDoc& mShow;
	/**
	 * True if the document should be printed in a landscape orientation,
	 * false if the document should be printed in a portrait orientation.
	 */
	bool mPrintLandscape;
	/**
	 * True if the stuntsheets should be printed with their continuities
	 * below; false otherwise.
	 */
	bool mPrintDoCont;
	/**
	 * True if a page should be printed that will describe all of the
	 * continuities in the show; false otherwise.
	 */
	bool mPrintDoContSheet;
	/**
	 * The width of the region where the field is drawn.
	 */
	float width;
	/**
	 * The height of the region where the field is drawn.
	 */
	float height;
	/**
	 * The width of the page, in dots.
	 */
	float real_width;
	/**
	 * The height of the page, in dots.
	 */
	float real_height;
	/**
	 * The x offset of the field.
	 */
	float field_x;
	/**
	 * The y offset of the field.
	 */
	float field_y;
	/**
	 * The width of the field on the page.
	 */
	float field_w;
	/**
	 * The height of the field on the page.
	 */
	float field_h;
	/**
	 * The x offset of a springshow stage.
	 */
	float stage_field_x;
	/**
	 * The y offset of a springshow stage.
	 */
	float stage_field_y;
	/**
	 * The width of a springshow stage.
	 */
	float stage_field_w;
	/**
	 * The height of a springshow stage.
	 */
	float stage_field_h;
	/**
	 * The size of a step on the printed page, in dots.
	 */
	float step_size;
	/**
	 * The size of a step on the printed page for a springshow show, in dots.
	 */
	float spr_step_size;
	/**
	 * The width of the printed field, in steps.
	 */
	short step_width;
	/**
	 * The offset in x coordinate (in steps) of a page, if a sheet is being
	 * split across multiple pages.
	 */
	short step_offset;
	/**
	 * The lower x bound of the field that will be printed to the page.
	 */
	Coord max_s;
	/**
	 * The upper x bound of the field that will be printed to the page.
	 */
	Coord max_n;
	/**
	 * When a sheet is split across multiple pages, this is the minimum
	 * x position that will be displayed on the current page.
	 */
	Coord clip_s;
	/**
	 * When a sheet is split across multiple pages, this is the maximum
	 * x position that will be dispayed on the current page.
	 */
	Coord clip_n;
	/**
	 * True if the next sheet that is printed should be split over
	 * multiple pages; false otherwise.
	 */
	short split_sheet;
};

#endif // __PRINT_PS_H__

