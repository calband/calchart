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
#include "calchartdoc.h"
#include "cc_show.h"
#include "confgr.h"
#include "draw.h"
#include "modes.h"

#include <wx/dcbuffer.h>

// View for linking CalChartDoc with FieldThumbnailBrowser
class FieldThumbnailBrowserView : public wxView {
public:
    FieldThumbnailBrowserView() = default;
    virtual ~FieldThumbnailBrowserView() = default;
    virtual void OnDraw(wxDC* dc) {}
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
};

void FieldThumbnailBrowserView::OnUpdate(wxView* sender, wxObject* hint)
{
    dynamic_cast<FieldThumbnailBrowser*>(GetFrame())->OnUpdate();
}

BEGIN_EVENT_TABLE(FieldThumbnailBrowser, wxScrolledWindow)
EVT_PAINT(FieldThumbnailBrowser::OnPaint)
EVT_CHAR(FieldThumbnailBrowser::HandleKey)
EVT_LEFT_DOWN(FieldThumbnailBrowser::HandleMouseDown)
END_EVENT_TABLE()

FieldThumbnailBrowser::FieldThumbnailBrowser(CalChartDoc* doc, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
    , mDoc(doc)
    , x_left_padding(4)
    , x_right_padding(4 + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X))
    , y_upper_padding(4)
    , y_name_size(16)
    , y_name_padding(4)
{
    mView = std::make_unique<FieldThumbnailBrowserView>();
    mView->SetDocument(doc);
    mView->SetFrame(this);

    // now update the current screen
    OnUpdate();
}

// calculate the size of the panel
wxSize FieldThumbnailBrowser::SizeOfOneCell() const
{
    auto mode_size = mDoc->GetMode().Size();
    auto current_size_x = GetSize().x - x_left_padding - x_right_padding;
    auto box_size_y = mode_size.y * (current_size_x / double(mode_size.x));
    return { GetSize().x - x_right_padding - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X), y_upper_padding + y_name_size + y_name_padding + int(box_size_y) };
}

// calculate which sheet the user clicked in
int FieldThumbnailBrowser::WhichCell(wxPoint const& p) const
{
    auto size_of_one = SizeOfOneCell();
    return p.y / size_of_one.y;
}

static auto CalcUserScale(wxSize const& box_size, CalChart::Coord const& mode_size)
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
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    auto& config = CalChartConfiguration::GetGlobalConfig();
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.Clear();

    // let's draw the boxes
    auto offset_x = 0;
    auto offset_y = 0;

    for (auto sheet = mDoc->GetSheetBegin(); sheet != mDoc->GetSheetEnd(); ++sheet) {

        dc.SetUserScale(1, 1);
        dc.SetBrush((config.Get_CalChartBrushAndPen(COLOR_FIELD).first));
        dc.DrawText(sheet->GetName(), offset_x + x_left_padding, offset_y + y_upper_padding);
        offset_y += y_upper_padding + y_name_size + y_name_padding;
        auto mode_size = mDoc->GetMode().Size();
        auto current_size_x = GetSize().x - x_left_padding - x_right_padding;
        auto box_size_y = mode_size.y * (current_size_x / double(mode_size.x));

        dc.SetPen(*wxBLACK_PEN);
        if (mDoc->GetCurrentSheet() == sheet) {
            auto copy_of_pen = *wxYELLOW_PEN;
            copy_of_pen.SetWidth(5);
            dc.SetPen(copy_of_pen);
        }

        dc.DrawRectangle(offset_x + x_left_padding, offset_y, current_size_x, box_size_y);
        dc.SetPen(*wxBLACK_PEN);

        auto origin = dc.GetDeviceOrigin();
        auto userScale = CalcUserScale({ current_size_x, int(box_size_y) }, mode_size);
        dc.SetUserScale(userScale, userScale);
        dc.SetDeviceOrigin(origin.x + offset_x + x_left_padding, origin.y + offset_y);

        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_FIELD_DETAIL).second);
        DrawMode(dc, config, mDoc->GetMode(), ShowMode_kAnimation);
        for (auto i = 0; i < mDoc->GetNumPoints(); ++i) {
            auto brushAndPen = config.Get_CalChartBrushAndPen(COLOR_POINT_ANIM_FRONT);
            dc.SetBrush(brushAndPen.first);
            dc.SetPen(brushAndPen.second);
            auto position = sheet->GetPoint(i).GetPos();
            auto x = position.x + mDoc->GetMode().Offset().x;
            auto y = position.y + mDoc->GetMode().Offset().y;
            dc.DrawRectangle(x - Int2CoordUnits(1) / 2, y - Int2CoordUnits(1) / 2, Int2CoordUnits(1), Int2CoordUnits(1));
        }
        dc.SetDeviceOrigin(origin.x, origin.y);

        offset_y += box_size_y;
    }
}

void FieldThumbnailBrowser::OnUpdate()
{
    auto size_of_one = SizeOfOneCell();
    SetVirtualSize(size_of_one.x, size_of_one.y * mDoc->GetNumSheets());

    SetScrollRate(0, size_of_one.y);

    auto get_size = GetSize();
    auto scrolled_top = CalcUnscrolledPosition({ 0, 0 });
    auto scrolled_bottom = CalcUnscrolledPosition({ 0, get_size.y });
    // so how many are visible:
    auto how_many_visible = get_size.y / size_of_one.y;
    if (how_many_visible == 0) {
        Scroll(0, mDoc->GetCurrentSheetNum());
    }
    else {
        // if the upper part is above the view, move the view to contain it.
        if (size_of_one.y * mDoc->GetCurrentSheetNum() < scrolled_top.y) {
            Scroll(0, mDoc->GetCurrentSheetNum());
        }
        // if the lower part is below the view, move the view to contain it.
        if ((size_of_one.y * (mDoc->GetCurrentSheetNum() + 1)) > scrolled_bottom.y) {
            Scroll(0, mDoc->GetCurrentSheetNum() - how_many_visible + 1);
        }
    }

    Refresh();
}

void FieldThumbnailBrowser::HandleKey(wxKeyEvent& event)
{
    if (((event.GetKeyCode() == WXK_LEFT) || (event.GetKeyCode() == WXK_UP)) && mDoc->GetCurrentSheetNum() > 0) {
        mDoc->SetCurrentSheet(mDoc->GetCurrentSheetNum() - 1);
    }
    if (((event.GetKeyCode() == WXK_RIGHT) || (event.GetKeyCode() == WXK_DOWN)) && mDoc->GetCurrentSheetNum() < (mDoc->GetNumSheets() - 1)) {
        mDoc->SetCurrentSheet(mDoc->GetCurrentSheetNum() + 1);
    }
}

void FieldThumbnailBrowser::HandleMouseDown(wxMouseEvent& event)
{
    auto which = WhichCell(CalcUnscrolledPosition(event.GetPosition()));
    mDoc->SetCurrentSheet(std::min(which, mDoc->GetNumSheets() - 1));
}
