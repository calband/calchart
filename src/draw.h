#pragma once
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

#include "cc_show.h"
#include <wx/dc.h>
#include <vector>
#include <set>
#include <map>
#include "confgr.h"

class wxBrush;
class wxString;
namespace CalChart {
class Sheet;
class Textline;
using Textline_list = std::vector<Textline>;
class Point;
class Coord;
struct DrawCommand;
}
class CalChartDoc;
class CalChartConfiguration;

typedef enum {
    ShowMode_kFieldView,
    ShowMode_kAnimation,
    ShowMode_kPrinting,
    ShowMode_kOmniView
} HowToDraw;

// draw the continuity starting at a specific offset
void DrawPoints(wxDC& dc, const CalChartConfiguration& config, CalChart::Coord origin,
    const SelectionList& selection_list,
    unsigned short numberPoints,
    const std::vector<std::string>& labels, const CalChart::Sheet& sheet,
    unsigned ref, bool primary);
void DrawGhostSheet(wxDC& dc, const CalChartConfiguration& config,
    CalChart::Coord origin, const SelectionList& selection_list,
    unsigned short numberPoints,
    const std::vector<std::string>& labels,
    const CalChart::Sheet& sheet, unsigned ref);
void Draw(wxDC& dc, const CalChartDoc& show, const CalChart::Sheet& sheet,
    unsigned ref, bool primary);
void DrawCont(wxDC& dc, const CalChart::Textline_list& print_continuity,
    const wxRect& bounding, bool landscape);
void DrawForPrinting(wxDC* dc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    unsigned ref, bool landscape);
void DrawPhatomPoints(wxDC& dc, const CalChartConfiguration& config,
    const CalChartDoc& show, const CalChart::Sheet& sheet,
    const std::map<int, CalChart::Coord>& positions);
void DrawMode(wxDC& dc, const CalChartConfiguration& config,
    const CalChart::ShowMode& mode, HowToDraw howToDraw);
wxImage GetOmniLinesImage(const CalChartConfiguration& config,
    const CalChart::ShowMode& mode);

void PrintStandard(std::ostream& buffer, const CalChart::Sheet& sheet);
void PrintSpringshow(std::ostream& buffer, const CalChart::Sheet& sheet);
void PrintOverview(std::ostream& buffer, const CalChart::Sheet& sheet);
void PrintCont(std::ostream& buffer, const CalChart::Sheet& sheet);

// We break this out of the class to make CalChart internals more cross platform
// Draw the point
void DrawPath(wxDC& dc, const CalChartConfiguration& config,
    const std::vector<CalChart::DrawCommand>& draw_commands,
    const CalChart::Coord& end);

void DrawCC_DrawCommandList(wxDC& dc,
    const std::vector<CalChart::DrawCommand>& draw_commands);
// void DrawShape(wxDC& dc, const CC_shape& shape, float x, float y);
