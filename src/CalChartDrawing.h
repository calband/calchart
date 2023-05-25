#pragma once
/*
 * CalChartDrawing.h
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

// Drawing is a huge part of CalChart.  These drawing primatives are for
// the UI, the Print buffer, and the Animation windows.
// Reuse is done where possible.
//
// The CalChart module attempts to keep the "how to draw" separated from
// the "what to draw" -- the CalChart::DrawCommand object contains the
// primatives of what to draw, but the details are done within the wxWidgets
// part of the code.

#include "CalChartConfiguration.h"
#include "CalChartShow.h"
#include <map>
#include <set>
#include <vector>
#include <wx/dc.h>

class wxBrush;
class wxString;
namespace CalChart {
class Sheet;
struct Textline;
using Textline_list = std::vector<Textline>;
class Point;
struct Coord;
}
class CalChartDoc;
class CalChartConfiguration;

typedef enum {
    ShowMode_kFieldView,
    ShowMode_kAnimation,
    ShowMode_kPrinting,
    ShowMode_kOmniView
} HowToDraw;

namespace CalChartDraw {

// draw the continuity starting at a specific offset
void Draw(wxDC& dc, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawPoints(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawGhostSheet(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref);
void DrawCont(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding, bool landscape, int useConstantTabs = 0);
void DrawContForPreview(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding);
void DrawForPrinting(wxDC* dc, CalChartConfiguration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape);
auto CreatePhatomPoints(const CalChartConfiguration& config, CalChartDoc const& show, CalChart::Sheet const& sheet, std::map<int, CalChart::Coord> const& positions) -> std::vector<CalChart::DrawCommand>;
auto DrawMode(CalChartConfiguration const& config, CalChart::ShowMode const& mode, HowToDraw howToDraw) -> std::vector<CalChart::DrawCommand>;
wxImage GetOmniLinesImage(const CalChartConfiguration& config, CalChart::ShowMode const& mode);

void PrintStandard(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintSpringshow(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintOverview(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintCont(std::ostream& buffer, CalChart::Sheet const& sheet);

// Draw the point
void DrawPath(wxDC& dc, CalChartConfiguration const& config, std::vector<CalChart::DrawCommand> const& draw_commands);

// Give an list of commands, apply them to the DeviceContext
void DrawCC_DrawCommandList(wxDC& dc, std::vector<CalChart::DrawCommand> const& draw_commands);

}