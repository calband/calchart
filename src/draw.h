/*
 * draw.h
 * Member functions for drawing stuntsheets
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

#ifndef __DRAW_H__
#define __DRAW_H__

#include <wx/dc.h>
#include <vector>
#include "confgr.h"

class wxBrush;
class wxString;
class CC_show;
class CC_sheet;
class CC_point;
class CC_coord;
struct CC_DrawCommand;
class CalChartDoc;
class CC_textline;
typedef std::vector<CC_textline> CC_textline_list;

// draw the continuity starting at a specific offset
void DrawSheetPoints(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, CalChartColors unselectedColor, CalChartColors selectedColor, CalChartColors unselectedTextColor, CalChartColors selectedTextColor);
void DrawGhostSheet(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref);
void Draw(wxDC& dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, bool primary);
void DrawCont(wxDC& dc, const CC_textline_list& print_continuity, const wxRect& bounding, bool landscape);
void DrawForPrinting(wxDC *dc, const CalChartDoc& show, const CC_sheet& sheet, unsigned ref, bool landscape);

void PrintStandard(std::ostream& buffer, const CC_sheet& sheet);
void PrintSpringshow(std::ostream& buffer, const CC_sheet& sheet);
void PrintOverview(std::ostream& buffer, const CC_sheet& sheet);
void PrintCont(std::ostream& buffer, const CC_sheet& sheet);


// We break this out of the class to make CalChart internals more cross platform
// Draw the point
void DrawPoint(const CC_point& point, wxDC& dc, unsigned reference, const CC_coord& origin, const wxBrush& fillBrush, const wxString& label);

void DrawPath(wxDC& dc, const std::vector<CC_DrawCommand>& draw_commands, const CC_coord& end);

void DrawCC_DrawCommandList(wxDC& dc, const std::vector<CC_DrawCommand>& draw_commands);
//void DrawShape(wxDC& dc, const CC_shape& shape, float x, float y);

#endif // __DRAW_H__
