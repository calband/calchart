#pragma once
/*
 * CalChartDrawing.h
 * Member functions for drawing stuntsheets
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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
// the "what to draw" -- the CalChart::Draw::DrawCommand object contains the
// primatives of what to draw, but the details are done within the wxWidgets
// part of the code.

#include "CalChartCoord.h"
#include "CalChartShow.h"
#include <map>
#include <ranges>
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
class Configuration;
}
class CalChartDoc;

typedef enum {
    ShowMode_kFieldView,
    ShowMode_kAnimation,
    ShowMode_kPrinting,
    ShowMode_kOmniView
} HowToDraw;

namespace CalChartDraw {

// draw the continuity starting at a specific offset
void Draw(wxDC& dc, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawPoints(wxDC& dc, CalChart::Configuration const& config, CalChart::Coord origin, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary);
void DrawGhostSheet(wxDC& dc, CalChart::Configuration const& config, CalChart::Coord origin, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref);
void DrawCont(wxDC& dc, CalChart::Configuration const& config, CalChart::Textline_list const& print_continuity, wxRect const& bounding, bool landscape);

void DrawForPrinting(wxDC* dc, CalChart::Configuration const& config, CalChartDoc const& show, CalChart::Sheet const& sheet, int ref, bool landscape);
auto GetOmniLinesImage(const CalChart::Configuration& config, CalChart::ShowMode const& mode) -> wxImage;

auto GeneratePointsDrawCommands(CalChart::Configuration const& config, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref, bool primary) -> std::vector<CalChart::Draw::DrawCommand>;
auto GenerateGhostPointsDrawCommands(CalChart::Configuration const& config, CalChart::SelectionList const& selection_list, int numberPoints, std::vector<std::string> const& labels, CalChart::Sheet const& sheet, int ref) -> std::vector<CalChart::Draw::DrawCommand>;
auto GeneratePhatomPointsDrawCommands(const CalChart::Configuration& config, CalChartDoc const& show, CalChart::Sheet const& sheet, std::map<int, CalChart::Coord> const& positions) -> std::vector<CalChart::Draw::DrawCommand>;
auto GenerateModeDrawCommands(CalChart::Configuration const& config, CalChart::ShowMode const& mode, HowToDraw howToDraw) -> std::vector<CalChart::Draw::DrawCommand>;

void PrintStandard(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintSpringshow(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintOverview(std::ostream& buffer, CalChart::Sheet const& sheet);
void PrintCont(std::ostream& buffer, CalChart::Sheet const& sheet);

}

namespace wxCalChart::Draw {

struct DrawSurface {
    wxPoint origin{};
    wxSize size{};
};

namespace details {
    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawItems const& cmd);
    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawManipulators const& cmd);
    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawStack const& cmd);
    void DrawCommand(wxDC& dc, DrawSurface surface, CalChart::Draw::DrawCommand const& cmd);
}

inline void DrawCommandList(wxDC& dc, CalChart::Draw::DrawCommand const& draw_command)
{
    auto drawSurface = DrawSurface{ { 0, 0 }, dc.GetSize() };
    details::DrawCommand(dc, drawSurface, draw_command);
}

// Give an list of commands, apply them to the DeviceContext
template <std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, CalChart::Draw::DrawCommand>
void DrawCommandList(wxDC& dc, DrawSurface surface, R&& draw_commands)
{
    for (auto&& cmd : draw_commands) {
        details::DrawCommand(dc, surface, cmd);
    }
}

// Give an list of commands, apply them to the DeviceContext
template <std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, CalChart::Draw::DrawCommand>
void DrawCommandList(wxDC& dc, R&& draw_commands)
{
    DrawCommandList(dc, { { 0, 0 }, dc.GetSize() }, std::forward<R>(draw_commands));
}

}
