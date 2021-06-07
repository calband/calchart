/*
 * mode_dialog_canvas.cpp
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

#include "ShowModeSetupCanvas.h"
#include "basic_ui.h"
#include "draw.h"
#include "modes.h"

#include <wx/dcbuffer.h>

//////// Show Mode setup ////////
// setup drawing characteristics
////////

BEGIN_EVENT_TABLE(ShowModeSetupCanvas, ClickDragCtrlScrollCanvas)
EVT_PAINT(ShowModeSetupCanvas::OnPaint)
EVT_MOTION(ShowModeSetupCanvas::OnMouseMove)
EVT_MAGNIFY(ShowModeSetupCanvas::OnMousePinchToZoom)
EVT_MOUSEWHEEL(ShowModeSetupCanvas::OnMouseWheel)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowModeSetupCanvas, ClickDragCtrlScrollCanvas)

ShowModeSetupCanvas::ShowModeSetupCanvas(CalChartConfiguration& config, wxWindow* parent, wxWindowID id)
    : super(parent, id, wxDefaultPosition, wxSize(640, 240))
    , mConfig(config)
    , mMode(CalChart::ShowMode::GetDefaultShowMode())
{
}

// Define the repainting behaviour
void ShowModeSetupCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(mConfig.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();

    // Draw the field
    DrawMode(dc, mConfig, mMode, ShowMode_kFieldView);
}

void ShowModeSetupCanvas::SetMode(CalChart::ShowMode const& mode)
{
    mMode = mode;
    SetCanvasSize(wxSize{ mMode.Size().x, mMode.Size().y });
    Refresh();
}

void ShowModeSetupCanvas::SetZoom(float factor)
{
    super::SetZoom(factor);
    Refresh();
}
