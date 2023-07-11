/*
 * PrintContinuityPreview.cpp
 * Continuity editors
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

#include "PrintContinuityPreview.h"
#include "CalChartDrawing.h"

BEGIN_EVENT_TABLE(PrintContinuityPreview, PrintContinuityPreview::super)
EVT_SIZE(PrintContinuityPreview::OnSizeEvent)
END_EVENT_TABLE()

static constexpr auto kPrintContinuityPreviewMinX = 256;
static constexpr auto kPrintContinuityPreviewMinY = 734 - 606;
PrintContinuityPreview::PrintContinuityPreview(wxWindow* parent, CalChartConfiguration const& config)
    : wxScrolled<wxWindow>(parent, wxID_ANY)
    , m_landscape(false)
    , mConfig(config)
{
    static const double kSizeX = 576, kSizeY = 734 - 606;
    SetVirtualSize(wxSize(kSizeX, kSizeY));
    SetScrollRate(10, 10);
    SetBackgroundColour(*wxWHITE);
    Connect(wxEVT_PAINT, wxPaintEventHandler(PrintContinuityPreview::OnPaint));
}

void PrintContinuityPreview::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    wxSize virtSize = GetVirtualSize();

    dc.Clear();
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));
    CalChartDraw::DrawCont(dc, mConfig, mPrintContinuity, wxRect(wxPoint(0, 0), virtSize), m_landscape);
}

void PrintContinuityPreview::OnSizeEvent(wxSizeEvent& event)
{
    auto x = std::max(event.m_size.x, kPrintContinuityPreviewMinX);
    auto y = std::max(event.m_size.y, kPrintContinuityPreviewMinY);
    SetVirtualSize(wxSize(x, y));
    SetScrollRate(10, 10);
}
