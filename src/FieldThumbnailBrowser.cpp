/*
 * FieldThumbnailBrowser
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

#include "FieldThumbnailBrowser.h"
#include "CalChartView.h"
#include "CalChartSizes.h"
#include "cc_show.h"
#include "confgr.h"
#include "draw.h"
#include "modes.h"

#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(FieldThumbnailBrowser, wxScrolledWindow)
EVT_PAINT(FieldThumbnailBrowser::OnPaint)
EVT_CHAR(FieldThumbnailBrowser::HandleKey)
EVT_LEFT_DOWN(FieldThumbnailBrowser::HandleMouseDown)
EVT_SIZE(FieldThumbnailBrowser::HandleSizeEvent)
END_EVENT_TABLE()

FieldThumbnailBrowser::FieldThumbnailBrowser(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
    , mXScrollPadding(wxSystemSettings::GetMetric(wxSYS_VSCROLL_X))
    , mYScrollPadding(wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y))
    , mLayoutHorizontal{ true }
{
    // now update the current screen
    OnUpdate();
}

// calculate the size of the panel depending on orientation
wxSize FieldThumbnailBrowser::SizeOfOneCell(bool horizontal) const
{
    if (!mView) {
        return {};
    }

    auto mode_size = fDIP(wxSize{mView->GetShowMode().Size().x, mView->GetShowMode().Size().y});
    if (horizontal) {
        auto current_size_y = GetSize().y - mYUpperPadding - mYNameSize - mYNamePadding - mYBottomPadding - mYScrollPadding;
        auto box_size_x = mode_size.x * (current_size_y / double(mode_size.y));
        return { int(box_size_x) + mXLeftPadding + mXRightPadding, GetSize().y };
    }
    auto current_size_x = GetSize().x - mXLeftPadding - mXRightPadding - mXScrollPadding;
    auto box_size_y = mode_size.y * (current_size_x / double(mode_size.x));
    return { GetSize().x, int(box_size_y) + mYUpperPadding + mYNameSize + mYNamePadding };
}

// calculate which sheet the user clicked in
int FieldThumbnailBrowser::WhichCell(wxPoint const& p) const
{
    auto size_of_one = SizeOfOneCell(mLayoutHorizontal);
    return (mLayoutHorizontal) ? p.x / size_of_one.x : p.y / size_of_one.y;
}

static auto CalcUserScale(wxSize const& box_size, wxSize const& mode_size)
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
        newvalue = newX / (float)CoordUnits2Int(showSizeX);
    } else {
        newvalue = newY / (float)CoordUnits2Int(showSizeY);
    }
    auto userScale = newvalue * (CoordUnits2Int(1 << 16) / 65536.0);

    return userScale;
}

// Define the repainting behaviour
// we draw things as a series of mini-fields, with number than the field.
// the current field is outlined in yellow
// The regions are going to be the size of the current frame and preserve the aspect ratio
// with an offset of 16 point font of the current sheet
// with a boundary of 4 above and below.

void FieldThumbnailBrowser::OnPaint(wxPaintEvent& event)
{
    if (!mView) {
        return;
    }

    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    auto& config = CalChartConfiguration::GetGlobalConfig();
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.Clear();

    // let's draw the boxes
    auto offset_x = 0;
    auto offset_y = 0;

    for (auto sheet = mView->GetSheetBegin(); sheet != mView->GetSheetEnd(); ++sheet) {

        dc.SetUserScale(1, 1);
        dc.SetBrush((config.Get_CalChartBrushAndPen(COLOR_FIELD).first));
        dc.DrawText(sheet->GetName(), offset_x + mXLeftPadding, offset_y + mYUpperPadding);
        auto newOffsetX = offset_x + mXLeftPadding;
        auto newOffsetY = offset_y + mYUpperPadding + mYNameSize + mYNamePadding;

        auto mode_size = fDIP(wxSize{mView->GetShowMode().Size().x, mView->GetShowMode().Size().y});
        auto current_size_x = GetSize().x - mXLeftPadding - mXRightPadding - mXScrollPadding;
        auto current_size_y = GetSize().y - mYNameSize - mYNamePadding - mYUpperPadding - mYBottomPadding - mYScrollPadding;
        auto box_size_x = (mLayoutHorizontal) ? mode_size.x * (current_size_y / double(mode_size.y)) : current_size_x;
        auto box_size_y = (mLayoutHorizontal) ? current_size_y : mode_size.y * (current_size_x / double(mode_size.x));

        dc.SetPen(*wxBLACK_PEN);
        if (mView->GetCurrentSheet() == sheet) {
            auto copy_of_pen = *wxYELLOW_PEN;
            copy_of_pen.SetWidth(5);
            dc.SetPen(copy_of_pen);
        }

        dc.DrawRectangle(newOffsetX, newOffsetY, box_size_x, box_size_y);
        dc.SetPen(*wxBLACK_PEN);

        auto origin = dc.GetDeviceOrigin();
        auto userScale = CalcUserScale({ int(box_size_x), int(box_size_y) }, mode_size);
        dc.SetUserScale(userScale, userScale);
        dc.SetDeviceOrigin(origin.x + newOffsetX, origin.y + newOffsetY);

        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
        DrawMode(dc, config, mView->GetShowMode(), ShowMode_kAnimation);
        for (auto i = 0; i < mView->GetNumPoints(); ++i) {
            auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_FRONT);
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            auto position = sheet->GetPoint(i).GetPos();
            auto x = position.x + mView->GetShowMode().Offset().x;
            auto y = position.y + mView->GetShowMode().Offset().y;
            dc.DrawRectangle(x - Int2CoordUnits(1) / 2, y - Int2CoordUnits(1) / 2, Int2CoordUnits(1), Int2CoordUnits(1));
        }
        dc.SetDeviceOrigin(origin.x, origin.y);

        offset_x += (mLayoutHorizontal) ? box_size_x + mXLeftPadding + mXRightPadding : 0;
        offset_y += (mLayoutHorizontal) ? 0 : box_size_y + mYUpperPadding + mYNameSize + mYNamePadding;
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
    mLayoutHorizontal = float(event.m_size.x) / float(event.m_size.y) > 1;
    OnUpdate();
}
