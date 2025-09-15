/*
 * PrintingPreview.cpp
 * Continuity editors
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

#include "PrintingPreview.hpp"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartMeasure.h"
#include "CalChartPrintContinuityLayout.h"
#include "CalChartShow.h"
#include "DCSaveRestore.h"

BEGIN_EVENT_TABLE(PrintingPreview, PrintingPreview::super)
EVT_SIZE(PrintingPreview::OnSizeEvent)
END_EVENT_TABLE()

static constexpr double kSizeX = 576, kSizeY = 734;
static constexpr double kSizeXLandscape = 917, kSizeYLandscape = 720;

static constexpr auto kAspectRatio = kSizeX / kSizeY;
static constexpr auto kAspectRatioLandscape = kSizeXLandscape / kSizeYLandscape;

static constexpr auto DefaultText
    = "~This is a centered line of text\n"
      "Normal \\bsBold \\isBold+Italics \\beItalics \\ieNormal\n"
      "All the symbols with two tabs\n"
      "\t\t\\po:\tplainman\n"
      "\t\t\\sx:\tsolidxman\n"
      "";

PrintingPreview::PrintingPreview(wxWindow* parent, CalChart::Configuration const& config)
    : super(config, parent, wxID_ANY, wxDefaultPosition, wxSize(kSizeX, kSizeY / 2))
    , mShow(CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode()))
    , mLandscape(false)
    , mConfig(config)
{
    using namespace CalChart;
    mShow->Create_SetupMarchersCommand({ { "A", "A" }, { "B", "B" }, { "C", "C" }, { "D", "D" } }, 1, 0).first(*mShow);
    SetVirtualSize(wxSize(kSizeX, kSizeY));
    SetScrollRate(10, 10);
    SetBackgroundColour(*wxWHITE);
    Connect(wxEVT_PAINT, wxPaintEventHandler(PrintingPreview::OnPaint));
}

void PrintingPreview::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    auto virtSize = GetVirtualSize();

    dc.Clear();
    dc.SetTextForeground(*wxBLACK);
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));

    auto sheet = CalChart::Sheet(4, "hello");
    sheet.SetPrintableContinuity("Test1", DefaultText);
    sheet.SetPosition({ 0, 0 }, 0);
    sheet.SetPosition({ 16, 16 }, 1);
    sheet.SetPosition({ 32, 32 }, 2);
    sheet.SetPosition({ 48, 48 }, 3);
    CalChartDraw::DrawForPrintingContinuity(dc, virtSize, mConfig, sheet, mLandscape);
    CalChartDraw::DrawForPrintingElements(dc, virtSize, sheet, mLandscape);
    CalChartDraw::DrawForPrintingField(
        dc,
        virtSize,
        mConfig,
        CalChart::ShowMode::GetDefaultShowMode(),
        mShow->GetPointsLabel(),
        sheet,
        0,
        mLandscape);
}

void PrintingPreview::OnSizeEvent(wxSizeEvent& event)
{
    // width is always going to be
    auto x = std::max<int>(event.m_size.x, kSizeX);
    auto y = x / (mLandscape ? kAspectRatioLandscape : kAspectRatio);
    SetVirtualSize(wxSize(x, y));
    SetScrollRate(10, 10);
}
