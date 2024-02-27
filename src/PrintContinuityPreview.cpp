/*
 * PrintContinuityPreview.cpp
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

#include "PrintContinuityPreview.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartMeasure.h"
#include "CalChartPrintContinuityLayout.h"
#include "DCSaveRestore.h"

BEGIN_EVENT_TABLE(PrintContinuityPreview, PrintContinuityPreview::super)
EVT_SIZE(PrintContinuityPreview::OnSizeEvent)
END_EVENT_TABLE()

static constexpr auto kPrintContinuityPreviewMinX = 256;
static constexpr auto kPrintContinuityPreviewMinY = 734 - 606;
PrintContinuityPreview::PrintContinuityPreview(wxWindow* parent, CalChart::Configuration const& config)
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

// Divide the draw space into lines with a padding in between each.
auto CalculatePointsPerLine(
    CalChart::PrintContinuityLayout::VStack const& print_continuity,
    wxRect const& bounding,
    int linePad)
{
    auto numLines = std::count_if(print_continuity.lines.begin(), print_continuity.lines.end(), [](auto&& i) { return i.on_sheet; });
    return ((bounding.GetBottom() - bounding.GetTop()) - (numLines - 1) * linePad) / (numLines ? numLines : 1);
}

static auto GenerateDrawCommands(wxDC& dc,
    CalChart::Configuration const& config,
    CalChart::PrintContinuityLayout::VStack const& printLayout,
    wxRect const& bounding,
    bool landscape) -> CalChart::Draw::DrawCommand
{
    // font size, we scale to be no more than 256 pixels.
    // 256 can be config
    auto linePad = static_cast<int>(config.Get_PrintContLinePad());
    auto factor = config.Get_PrintContDotRatio();
    auto fontSize = std::min<int>(CalculatePointsPerLine(printLayout, bounding, linePad), 256);
    auto [symbolSize, symbolMiddle] = [](auto& dc, int fontSize, double factor) -> std::pair<int, CalChart::Coord> {
        auto restore = SaveAndRestore::Font{ dc };
        wxCalChart::setFont(dc, { fontSize, CalChart::Font::Family::Modern });
        wxCoord textw{};
        wxCoord texth{};
        wxCoord textd{};
        dc.GetTextExtent("O", &textw, &texth, &textd);
        auto middle = texth - textw - textd;
        return { textw * factor, CalChart::Coord(middle, middle) };
    }(dc, fontSize, factor);

    auto context = CalChart::PrintContinuityLayout::Context{
        fontSize,
        landscape,
        linePad,
        symbolSize,
        symbolMiddle,
        config.Get_PrintContPLineRatio(),
        config.Get_PrintContSLineRatio(),
    };

    try {
        return CalChart::Draw::withBrushAndPen(
            wxCalChart::toBrushAndPen(*wxBLACK_PEN),
            CalChart::Draw::withFont(
                context.plain,
                CalChart::PrintContinuityLayout::ToDrawCommand(printLayout, context)));
    } catch (std::exception const& e) {
        std::cout << "hit the excpetion " << e.what() << "\n";
        return CalChart::Draw::Ignore{};
    }
}

void PrintContinuityPreview::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
    auto virtSize = GetVirtualSize();

    dc.Clear();
    dc.DrawRectangle(wxRect(wxPoint(0, 0), virtSize));
    auto useNew = mConfig.Get_PrintContUseNewDraw();
    if (useNew) {
        // a possible future optimization would be to generate this ahead of time.
        auto printLayout = CalChart::PrintContinuityLayout::Parse(mPrintContinuity.GetOriginalLine());
        auto drawCommand = GenerateDrawCommands(dc, mConfig, printLayout, wxRect(wxPoint(0, 0), virtSize), m_landscape);
        wxCalChart::Draw::DrawCommandList(dc, drawCommand);
    } else {
        CalChartDraw::DrawCont(dc, mConfig, mPrintContinuity.GetChunks(), wxRect(wxPoint(0, 0), virtSize), m_landscape);
    }
}

void PrintContinuityPreview::OnSizeEvent(wxSizeEvent& event)
{
    auto x = std::max(event.m_size.x, kPrintContinuityPreviewMinX);
    auto y = std::max(event.m_size.y, kPrintContinuityPreviewMinY);
    SetVirtualSize(wxSize(x, y));
    SetScrollRate(10, 10);
}
