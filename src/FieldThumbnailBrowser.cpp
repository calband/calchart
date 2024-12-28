/*
 * FieldThumbnailBrowser
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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1
#include "FieldThumbnailBrowser.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartMeasure.h"
#include "CalChartShow.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "CalChartView.h"
#include "basic_ui.h"

#include <ranges>
#include <wx/dcbuffer.h>

namespace {
constexpr auto kXLeftPadding = 4;
constexpr auto kXRightPadding = 4;
constexpr auto kYUpperPadding = 4;
constexpr auto kYNamePadding = 4;
constexpr auto kYBottomPadding = 4;
constexpr auto kHighlightWidth = 5;
}

BEGIN_EVENT_TABLE(FieldThumbnailBrowser, wxScrolledWindow)
EVT_PAINT(FieldThumbnailBrowser::OnPaint)
EVT_CHAR(FieldThumbnailBrowser::HandleKey)
EVT_LEFT_DOWN(FieldThumbnailBrowser::HandleMouseDown)
EVT_SIZE(FieldThumbnailBrowser::HandleSizeEvent)
END_EVENT_TABLE()

FieldThumbnailBrowser::FieldThumbnailBrowser(CalChart::Configuration const& config, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
    , mXScrollPadding(wxSystemSettings::GetMetric(wxSYS_VSCROLL_X))
    , mYNameSize(GetThumbnailFontSize())
    , mYScrollPadding(wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y))
    , mLayoutHorizontal{ true }
    , mConfig(config)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // now update the current screen
    OnUpdate();
}

// calculate the size of the panel depending on orientation
wxSize FieldThumbnailBrowser::SizeOfOneCell(bool horizontal) const
{
    if (!mView) {
        return { 1, 1 };
    }

    auto mode_size = fDIP(mView->GetShowFullSize());
    if (horizontal) {
        auto current_size_y = GetSize().y - kYUpperPadding - mYNameSize - kYNamePadding - kYBottomPadding - mYScrollPadding;
        auto box_size_x = mode_size.x * (current_size_y / double(mode_size.y));
        return { int(box_size_x) + kXLeftPadding + kXRightPadding, GetSize().y };
    }
    auto current_size_x = GetSize().x - kXLeftPadding - kXRightPadding - mXScrollPadding;
    auto box_size_y = mode_size.y * (current_size_x / double(mode_size.x));
    return { GetSize().x, int(box_size_y) + kYUpperPadding + mYNameSize + kYNamePadding };
}

// calculate which sheet the user clicked in
int FieldThumbnailBrowser::WhichCell(wxPoint const& p) const
{
    auto size_of_one = SizeOfOneCell(mLayoutHorizontal);
    return (mLayoutHorizontal) ? p.x / size_of_one.x : p.y / size_of_one.y;
}

namespace {

auto CalcUserScale(wxSize const& box_size, wxSize const& mode_size)
{
    auto newX = static_cast<float>(box_size.x);
    auto newY = static_cast<float>(box_size.y);
    auto showSizeX = static_cast<float>(mode_size.x);
    auto showSizeY = static_cast<float>(mode_size.y);

    auto showAspectRatio = showSizeX / showSizeY;
    auto newSizeRatio = newX / newY;
    auto newvalue = 1.0;
    // always choose x when the new aspect ratio is smaller than the show.
    // This will keep the whole field on in the canvas
    if (newSizeRatio < showAspectRatio) {
        newvalue = newX / (float)CalChart::CoordUnits2Int(showSizeX);
    } else {
        newvalue = newY / (float)CalChart::CoordUnits2Int(showSizeY);
    }
    auto userScale = newvalue * (CalChart::CoordUnits2Int(1 << 16) / 65536.0);

    return userScale;
}

auto LayoutSheetThumbnails(CalChartView const& view, CalChart::Configuration const& config, int YNameSize, CalChart::Coord thumbnail_offset, CalChart::Coord box_size, CalChart::Coord box_offset)
{
    auto highlight_offset = box_offset + view.GetCurrentSheetNum() * thumbnail_offset;
    return std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::withFont(
            CalChart::Font{ YNameSize },
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD),
                CalChart::Ranges::enumerate_view(view.GetSheetsName())
                    | std::views::transform([thumbnail_offset, box_size, box_offset](auto&& whichAndSheet) {
                          auto [which, name] = whichAndSheet;
                          return std::vector<CalChart::Draw::DrawCommand>{
                              CalChart::Draw::Text(name),
                              CalChart::Draw::Rectangle(box_offset, box_size),
                          }
                          + (which * thumbnail_offset);
                      })
                    | std::views::join)),
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD),
            CalChart::Draw::withPen(CalChart::toPen(config.Get_CalChartBrushAndPen(CalChart::Colors::POINT_HILIT_TEXT)).withWidth(kHighlightWidth),
                CalChart::Draw::Rectangle(highlight_offset, box_size))),
    }
    + CalChart::Coord(kXLeftPadding, kYUpperPadding);
}

}

// Define the repainting behaviour
// we draw things as a series of mini-fields, with number than the field.
// the current field is outlined in yellow
// The regions are going to be the size of the current frame and preserve the aspect ratio
// with an offset of 16 point font of the current sheet
// with a boundary of 4 above and below.

// auto gFieldThumbnailMeasure = CalChart::MeasureDuration{ "FieldThumbnail" };
void FieldThumbnailBrowser::OnPaint(wxPaintEvent&)
{
    // for profiling purposes
    // std::cout << gFieldThumbnailMeasure << "\n";
    // auto snapshot = gFieldThumbnailMeasure.doMeasurement();
    if (!mView) {
        return;
    }

    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.Clear();

    // let's draw the boxes
    auto mode_size = fDIP(mView->GetShowFullSize());
    auto current_size = GetSize() - wxSize(kXLeftPadding + kXRightPadding + mXScrollPadding, mYNameSize + kYNamePadding + kYUpperPadding + kYBottomPadding + mYScrollPadding);
    auto box_size = mLayoutHorizontal
        ? wxSize(mode_size.x * (current_size.y / static_cast<double>(mode_size.y)), current_size.y)
        : wxSize(current_size.x, mode_size.y * (current_size.x / static_cast<double>(mode_size.x)));

    auto userScale = CalcUserScale(box_size, mode_size);

    auto thumbnail_offset = mLayoutHorizontal
        ? CalChart::Coord(box_size.x + kXLeftPadding + kXRightPadding, 0)
        : CalChart::Coord(0, box_size.y + kYUpperPadding + mYNameSize + kYNamePadding);

    auto field_offset = CalChart::Coord(0, mYNameSize + kYNamePadding);

    wxCalChart::Draw::DrawCommandList(dc, LayoutSheetThumbnails(*mView, mConfig, mYNameSize, thumbnail_offset, CalChart::Coord(box_size.x, box_size.y), field_offset));

    // we manipulate the scale and origin to create repeating copies of the field
    auto origin = dc.GetDeviceOrigin();
    dc.SetUserScale(userScale, userScale);
    for (auto [which, sheet] : CalChart::Ranges::enumerate_view(mView->GenerateFieldWithMarchersDrawCommands())) {
        auto newOrigin = which * thumbnail_offset;
        dc.SetDeviceOrigin(origin.x + newOrigin.x + kXLeftPadding, origin.y + newOrigin.y + kYUpperPadding + mYNameSize + kYNamePadding);
        wxCalChart::Draw::DrawCommandList(dc, sheet);
    }
}

void FieldThumbnailBrowser::OnUpdate()
{
    if (!mView) {
        return;
    }

    auto size_of_one = SizeOfOneCell(mLayoutHorizontal);
    SetVirtualSize(size_of_one.x * (mLayoutHorizontal ? mView->GetNumSheets() : 1.0), size_of_one.y * (mLayoutHorizontal ? 1.0 : mView->GetNumSheets()));

    SetScrollRate(mLayoutHorizontal ? size_of_one.x : 0, mLayoutHorizontal ? 0 : size_of_one.y);

    auto get_size = GetSize();
    auto scrolled_top = CalcUnscrolledPosition({ 0, 0 });
    auto scrolled_bottom = CalcUnscrolledPosition({ get_size.x, get_size.y });
    // so how many are visible:
    auto how_many_visible = mLayoutHorizontal ? get_size.x / size_of_one.x : get_size.y / size_of_one.y;
    if (how_many_visible == 0) {
        Scroll(mLayoutHorizontal ? mView->GetCurrentSheetNum() : 0, mLayoutHorizontal ? 0 : mView->GetCurrentSheetNum());
    } else {
        // if the upper part is above the view, move the view to contain it.
        if (mLayoutHorizontal) {
            if (size_of_one.x * mView->GetCurrentSheetNum() < scrolled_top.x) {
                Scroll(mView->GetCurrentSheetNum(), 0);
            }
            // if the lower part is below the view, move the view to contain it.
            if ((size_of_one.x * (mView->GetCurrentSheetNum() + 1)) > scrolled_bottom.x) {
                Scroll(mView->GetCurrentSheetNum() - how_many_visible + 1, 0);
            }
        } else {
            if (size_of_one.y * mView->GetCurrentSheetNum() < scrolled_top.y) {
                Scroll(0, mView->GetCurrentSheetNum());
            }
            // if the lower part is below the view, move the view to contain it.
            if ((size_of_one.y * (mView->GetCurrentSheetNum() + 1)) > scrolled_bottom.y) {
                Scroll(0, mView->GetCurrentSheetNum() - how_many_visible + 1);
            }
        }
    }

    Refresh();
}

void FieldThumbnailBrowser::HandleKey(wxKeyEvent& event)
{
    if (((event.GetKeyCode() == WXK_LEFT) || (event.GetKeyCode() == WXK_UP)) && mView->GetCurrentSheetNum() > 0) {
        mView->GoToSheet(mView->GetCurrentSheetNum() - 1);
    }
    if (((event.GetKeyCode() == WXK_RIGHT) || (event.GetKeyCode() == WXK_DOWN)) && mView->GetCurrentSheetNum() < (mView->GetNumSheets() - 1)) {
        mView->GoToSheet(mView->GetCurrentSheetNum() + 1);
    }
}

void FieldThumbnailBrowser::HandleMouseDown(wxMouseEvent& event)
{
    auto which = WhichCell(CalcUnscrolledPosition(event.GetPosition()));
    mView->GoToSheet(std::min(which, mView->GetNumSheets() - 1));
}

void FieldThumbnailBrowser::HandleSizeEvent(wxSizeEvent& event)
{
    auto mode_size = mView->GetShowFullSize();
    auto ratioMode = mode_size.y ? mode_size.x / float(mode_size.y) : 0;
    auto ratioSize = event.m_size.y ? event.m_size.x / float(event.m_size.y) : 0;

    mLayoutHorizontal = ratioSize > ratioMode;
    OnUpdate();
}
