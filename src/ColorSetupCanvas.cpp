/*
 * ColorSetupCanvas.cpp
 * Canvas for setting up colors
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

#include "ColorSetupCanvas.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"

#include <wx/dcbuffer.h>

using namespace CalChart;

//////// Draw setup ////////
// handling Drawing colors
////////

BEGIN_EVENT_TABLE(ColorSetupCanvas, ColorSetupCanvas::super)
EVT_PAINT(ColorSetupCanvas::OnPaint)
EVT_ERASE_BACKGROUND(ColorSetupCanvas::OnEraseBackground)
END_EVENT_TABLE()

namespace {
auto GenerateScriptCal(CalChart::Coord offset)
{
    auto curve = CalChart::Curve(offset + Coord(Int2CoordUnits(4), Int2CoordUnits(2)));
    curve.Append(offset + Coord(Int2CoordUnits(5), Int2CoordUnits(1)));
    curve.Append(offset + Coord(Int2CoordUnits(3), Int2CoordUnits(0)));
    curve.Append(offset + Coord(Int2CoordUnits(0), Int2CoordUnits(4)));
    curve.Append(offset + Coord(Int2CoordUnits(3), Int2CoordUnits(8)));
    curve.Append(offset + Coord(Int2CoordUnits(6), Int2CoordUnits(6)));
    curve.Append(offset + Coord(Int2CoordUnits(8), Int2CoordUnits(4)));
    curve.Append(offset + Coord(Int2CoordUnits(10), Int2CoordUnits(5)));
    curve.Append(offset + Coord(Int2CoordUnits(8), Int2CoordUnits(4)));
    curve.Append(offset + Coord(Int2CoordUnits(6), Int2CoordUnits(6)));
    curve.Append(offset + Coord(Int2CoordUnits(7), Int2CoordUnits(8)));
    curve.Append(offset + Coord(Int2CoordUnits(9), Int2CoordUnits(8)));
    curve.Append(offset + Coord(Int2CoordUnits(10), Int2CoordUnits(6)));
    curve.Append(offset + Coord(Int2CoordUnits(10), Int2CoordUnits(8)));
    curve.Append(offset + Coord(Int2CoordUnits(13), Int2CoordUnits(7)));
    curve.Append(offset + Coord(Int2CoordUnits(16), Int2CoordUnits(1)));
    curve.Append(offset + Coord(Int2CoordUnits(15), Int2CoordUnits(0)));
    curve.Append(offset + Coord(Int2CoordUnits(14), Int2CoordUnits(1)));
    curve.Append(offset + Coord(Int2CoordUnits(12), Int2CoordUnits(8)));
    curve.Append(offset + Coord(Int2CoordUnits(13), Int2CoordUnits(10)));
    curve.Append(offset + Coord(Int2CoordUnits(2), Int2CoordUnits(9)));
    curve.Append(offset + Coord(Int2CoordUnits(1), Int2CoordUnits(10)));
    return curve;
}
}

ColorSetupCanvas::ColorSetupCanvas(CalChart::Configuration& config, wxWindow* parent)
    : super(config, parent, wxID_ANY, wxDefaultPosition, GetColorSetupCanvas())
    , mShow(Show::Create(ShowMode::GetDefaultShowMode()))
    , mMode(ShowMode::CreateShowMode(
          Coord(Int2CoordUnits(160), Int2CoordUnits(84)),
          Coord(Int2CoordUnits(80), Int2CoordUnits(42)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)), Int2CoordUnits(32), Int2CoordUnits(52),
          kDefaultYardLines))
    , mConfig(config)
{
    auto field_offset = mMode.FieldOffset();
    SetCanvasSize(wxSize{ mMode.Size().x, mMode.Size().y });
    SetZoom(3);

    // Create a fake show with some points and selections to draw an example for
    // the user
    auto labels = std::vector<std::pair<std::string, std::string>>{
        { "unsel", "" },
        { "unsel", "" },
        { "sel", "" },
        { "sel", "" },
    };
    mShow->Create_SetupMarchersCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetSelectionListCommand(SelectionList{ 0, 2 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_X).first(*mShow);
    mShow->Create_SetSelectionListCommand(SelectionList{ 1, 3 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_SOLX).first(*mShow);
    mShow->Create_SetSelectionListCommand(SelectionList{}).first(*mShow);

    auto script_offset = field_offset + Coord(Int2CoordUnits(18), Int2CoordUnits(6));

    mShow->Create_AddSheetCurveCommand(GenerateScriptCal(script_offset)).first(*mShow);

    // we want to show the difference between ref highlight and normal highlight, so we draw ref 1.
    // As we are doing that it means the reference points will show up in the normal color and the
    // non-ref point (0) will show up in the reference point color.
    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(6)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 1).first(*mShow);
    }

    mShow->Create_AddSheetsCommand(Show::Sheet_container_t{ *static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet() }, 1).first(*mShow);
    mShow->Create_SetCurrentSheetCommand(1).first(*mShow);
    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 0).first(*mShow);
    }
    mShow->Create_SetCurrentSheetCommand(0).first(*mShow);

    SelectionList list;
    list.insert(2);
    list.insert(3);

    mShow->Create_SetSelectionListCommand(list).first(*mShow);
}

// Because we're not a real show, we have to make the path manually.
auto GenerateFakePathDrawCommands(CalChart::ShowMode const& mode, CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto field_offset = mode.FieldOffset();
    auto point_start = field_offset + Coord(Int2CoordUnits(4), Int2CoordUnits(2));
    auto path = std::vector<CalChart::Draw::DrawCommand>{};
    auto pathEnd = point_start + Coord(Int2CoordUnits(0), Int2CoordUnits(2));
    path.emplace_back(Draw::Line{ point_start, pathEnd });
    point_start = pathEnd;
    pathEnd += Coord(Int2CoordUnits(18), Int2CoordUnits(0));
    path.emplace_back(Draw::Line{ point_start, pathEnd });
    path.emplace_back(Draw::Circle{ pathEnd, static_cast<Coord::units>(CalChart::Float2CoordUnits(config.Get_DotRatio()) / 2) });
    return path;
}

// Because we're not a real interaction, we have to make the select shape manually.
auto GenerateFakeSelectShapeDrawCommands(CalChart::ShowMode const& mode) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto field_offset = mode.FieldOffset();
    auto shape_start = field_offset + Coord(Int2CoordUnits(18), Int2CoordUnits(-2));
    auto shape_end = shape_start + Coord(Int2CoordUnits(4), Int2CoordUnits(4));
    Shape_rect rect(shape_start, shape_end);
    return rect.GetCC_DrawCommand();
}

auto GenerateCurvePoints(std::vector<CalChart::Coord> const& points, CalChart::Coord::units boxSize) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(points | std::views::transform([boxSize](auto&& point) {
        return CalChart::Draw::Rectangle(point - Coord(boxSize, boxSize) / 2, Coord(boxSize, boxSize));
    }));
}

auto GenerateCurves(CalChart::ShowMode const& mode, CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto field_offset = mode.FieldOffset();
    auto boxSize = CalChart::Float2CoordUnits(config.Get_ControlPointRatio());
    auto curve = CalChart::Curve(Coord(Int2CoordUnits(2), Int2CoordUnits(8)));
    curve.Append(Coord(Int2CoordUnits(8), Int2CoordUnits(12)));
    curve.Append(Coord(Int2CoordUnits(6), Int2CoordUnits(7)));
    curve.Append(Coord(Int2CoordUnits(12), Int2CoordUnits(12)));
    curve.Append(Coord(Int2CoordUnits(16), Int2CoordUnits(8)));
    auto selectedPoint = std::vector{ curve.GetControlPoints().at(2) };
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE),
            curve.GetCC_DrawCommand()),
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE_CONTROL_POINT),
            GenerateCurvePoints(curve.GetControlPoints(), boxSize)),
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE_CONTROL_POINT),
            GenerateCurvePoints(selectedPoint, 2 * boxSize)),
    }
    + field_offset;
}

// Define the repainting behaviour
void ColorSetupCanvas::OnPaint(wxPaintEvent&)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    wxCalChart::setBackground(dc, mConfig.Get_CalChartBrushAndPen(CalChart::Colors::FIELD));
    dc.Clear();

    auto sheet = static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet();
    auto nextSheet = sheet + 1;

    // Draw the field
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    auto offset = mMode.Offset();
    CalChart::append(drawCmds,
        CalChart::CreateModeDrawCommandsWithBorderOffset(
            mConfig,
            mMode,
            CalChart::HowToDraw::FieldView));

    // draw the ghost sheet
    CalChart::append(drawCmds,
        mShow->GenerateGhostPointsDrawCommands(
            mConfig,
            mShow->GetSelectionList(),
            *nextSheet));

    // Draw the points
    CalChart::append(drawCmds, mShow->GenerateSheetElements(mConfig, 1));

    // draw the path, but because we're not a real show, we have to make the path manually.
    CalChart::append(drawCmds,
        CalChart::Draw::withBrushAndPen(
            mConfig.Get_CalChartBrushAndPen(CalChart::Colors::PATHS),
            GenerateFakePathDrawCommands(mMode, mConfig)));

    // draw the shape
    CalChart::append(drawCmds,
        CalChart::Draw::withBrush(
            CalChart::Brush::TransparentBrush(),
            CalChart::Draw::withPen(
                toPen(mConfig.Get_CalChartBrushAndPen(CalChart::Colors::SHAPES)),
                GenerateFakeSelectShapeDrawCommands(mMode))));
    // draw the curves
    CalChart::append(drawCmds, GenerateCurves(mMode, mConfig));
    wxCalChart::Draw::DrawCommandList(dc, drawCmds + offset);
}

// We have a empty erase background to improve redraw performance.
void ColorSetupCanvas::OnEraseBackground(wxEraseEvent&) { }
