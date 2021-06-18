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
class Textline;
using Textline_list = std::vector<Textline>;
class Point;
struct Coord;
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
void Draw(wxDC& dc, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawPoints(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawGhostSheet(wxDC& dc, CalChartConfiguration const& config, CalChart::Coord origin, SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref);
void DrawCont(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding, bool landscape, int useConstantTabs = 0);
void DrawContForPreview(wxDC& dc, CalChart::Textline_list const& print_continuity, wxRect const& bounding);
void DrawForPrinting(wxDC* dc, CalChartConfiguration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape);
void DrawPhatomPoints(wxDC& dc, const CalChartConfiguration& config, CalChartDoc const& show, CalChart::Sheet const& sheet, std::map<int, CalChart::Coord> const& positions);
void DrawMode(wxDC& dc, CalChartConfiguration const& config, CalChart::ShowMode const& mode, HowToDraw howToDraw);
wxImage GetOmniLinesImage(const CalChartConfiguration& config, CalChart::ShowMode const& mode);

void PrintStandard(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintSpringshow(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintOverview(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintCont(std::ostream& buffer, CalChart::Sheet const& sheet);

// We break this out of the class to make CalChart internals more cross platform
// Draw the point
void DrawPath(wxDC& dc, CalChartConfiguration const& config, std::vector<CalChart::DrawCommand> const& draw_commands, CalChart::Coord const& end);

void DrawCC_DrawCommandList(wxDC& dc, std::vector<CalChart::DrawCommand> const& draw_commands);
