/*
 * cc_preferences_ui.cpp
 * Dialox box for preferences
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

#include "ColorSetupCanvas.h"
#include "CalChartDoc.h"
#include "ContinuityBrowserPanel.h"
#include "ContinuityComposerDialog.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "cont.h"
#include "draw.h"
#include "mode_dialog.h"
#include "mode_dialog_canvas.h"
#include "modes.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

using namespace CalChart;

//////// Draw setup ////////
// handling Drawing colors
////////

BEGIN_EVENT_TABLE(ColorSetupCanvas, ClickDragCtrlScrollCanvas)
EVT_PAINT(ColorSetupCanvas::OnPaint)
EVT_ERASE_BACKGROUND(ColorSetupCanvas::OnEraseBackground)
END_EVENT_TABLE()

ColorSetupCanvas::ColorSetupCanvas(CalChartConfiguration& config, wxWindow* parent)
    : super(parent, wxID_ANY, wxDefaultPosition, wxSize(640, 240))
    , mMode(ShowMode::CreateShowMode(
          Coord(Int2CoordUnits(160), Int2CoordUnits(84)),
          Coord(Int2CoordUnits(80), Int2CoordUnits(42)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)),
          Coord(Int2CoordUnits(4), Int2CoordUnits(4)), Int2CoordUnits(32), Int2CoordUnits(52),
          ShowMode::GetDefaultYardLines()))
    , mConfig(config)
{
    auto field_offset = mMode.FieldOffset();
    auto offset = mMode.Offset();
    SetCanvasSize(wxSize{ mMode.Size().x, mMode.Size().y });

    // Create a fake show with some points and selections to draw an example for
    // the user
    mShow = Show::Create_CC_show(ShowMode::GetDefaultShowMode());
    auto labels = std::vector<std::string>{
        "unsel",
        "unsel",
        "sel",
        "sel",
    };
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetShowInfoCommand(labels, 4, field_offset).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 0, 2 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_X).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{ 1, 3 }).first(*mShow);
    mShow->Create_SetSymbolCommand(SYMBOL_SOLX).first(*mShow);
    mShow->Create_SetSelectionCommand(SelectionList{}).first(*mShow);

    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(i * 4), Int2CoordUnits(6)) } }, 1).first(*mShow);
    }

    mShow->Create_AddSheetsCommand(Show::Sheet_container_t{ *static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet() }, 1).first(*mShow);
    mShow->Create_SetCurrentSheetCommand(1).first(*mShow);
    for (auto i = 0; i < 4; ++i) {
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 0).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 1).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 2).first(*mShow);
        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 2)) } }, 3).first(*mShow);

        mShow->Create_MovePointsCommand({ { i, field_offset + Coord(Int2CoordUnits(18 + i * 4), Int2CoordUnits(2 + 6)) } }, 1).first(*mShow);
    }
    mShow->Create_SetCurrentSheetCommand(0).first(*mShow);

    auto point_start = offset + field_offset + Coord(Int2CoordUnits(4), Int2CoordUnits(2));
    mPathEnd = point_start + Coord(Int2CoordUnits(0), Int2CoordUnits(2));
    mPath.emplace_back(point_start, mPathEnd);
    point_start = mPathEnd;
    mPathEnd += Coord(Int2CoordUnits(18), Int2CoordUnits(0));
    mPath.emplace_back(point_start, mPathEnd);

    auto shape_start = field_offset + Coord(Int2CoordUnits(18), Int2CoordUnits(-2));
    auto shape_end = shape_start + Coord(Int2CoordUnits(4), Int2CoordUnits(4));
    Shape_rect rect(shape_start, shape_end);
    mShape = rect.GetCC_DrawCommand(offset.x, offset.y);
}

// Define the repainting behaviour
void ColorSetupCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();

    // Draw the field
    DrawMode(dc, mConfig, mMode, ShowMode_kFieldView);

    auto sheet = static_cast<CalChart::Show const&>(*mShow).GetCurrentSheet();
    auto nextSheet = sheet;
    ++nextSheet;

    SelectionList list;
    list.insert(2);
    list.insert(3);

    // draw the ghost sheet
    DrawGhostSheet(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *nextSheet, 0);

    // Draw the points
    DrawPoints(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *sheet, 0, true);
    DrawPoints(dc, mConfig, mMode.Offset(), list, mShow->GetNumPoints(),
        mShow->GetPointLabels(), *sheet, 1, false);

    // draw the path
    DrawPath(dc, mConfig, mPath, mPathEnd);

    // draw the shap
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(mConfig.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
    DrawCC_DrawCommandList(dc, mShape);
}

// We have a empty erase background to improve redraw performance.
void ColorSetupCanvas::OnEraseBackground(wxEraseEvent& event) {}
