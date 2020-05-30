/*
 * basic_ui.cpp
 * Handle basic wxWindows classes
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

#include "basic_ui.h"
#include "confgr.h"
#include "platconf.h"

#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/stdpaths.h>

#include "calchart.xbm"
#include "calchart.xpm"

// Set icon to band's insignia
void SetBandIcon(wxFrame* frame)
{
    wxIcon icon = wxICON(calchart);
    frame->SetIcon(icon);
}

wxStaticBitmap* BitmapWithBandIcon(wxWindow* parent, wxSize const& size)
{
    wxBitmap bitmap(BITMAP_NAME(calchart));
    wxImage image;
#if defined(__APPLE__) && (__APPLE__)
    const static wxString kImageDir = wxT("CalChart.app/Contents/Resources/calchart.png");
#else
    const static wxString kImageDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR wxT("calchart.png"));
#endif
    if (image.LoadFile(kImageDir)) {
        if (size != wxDefaultSize) {
            image = image.Scale(size.GetX(), size.GetY(), wxIMAGE_QUALITY_HIGH);
        }
        bitmap = wxBitmap{ image };
    }
    return new wxStaticBitmap(parent, wxID_STATIC, bitmap, wxDefaultPosition, size);
}

wxStaticText* TextStringWithSize(wxWindow* parent, std::string const& label, int pointSize)
{
    auto result = new wxStaticText(parent, wxID_STATIC, label, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    result->SetFont(CreateFont(pointSize));
    return result;
}

wxStaticLine* LineWithLength(wxWindow* parent, int length, long style)
{
    return new wxStaticLine(parent, wxID_STATIC, wxDefaultPosition, wxSize(length, -1), style);
}

class FancyTextWinDropTarget : public wxFileDropTarget {
public:
    FancyTextWinDropTarget(FancyTextWin* w)
        : win(w)
    {
    }
    virtual bool OnDropFiles(wxCoord x, wxCoord y,
        const wxArrayString& filenames)
    {
        if (filenames.Count() > 0) {
            win->LoadFile(filenames[0]);
        }
        return true;
    }

private:
    FancyTextWin* win;
};

// Text subwindow that can respond to drag-and-drop
FancyTextWin::FancyTextWin(wxWindow* parent, wxWindowID id,
    const wxString& value, const wxPoint& pos,
    const wxSize& size, long style)
    : wxTextCtrl(parent, id, value, pos, size, style)
{
    SetDropTarget(new FancyTextWinDropTarget(this));
}

wxString FancyTextWin::GetValue(void) const
{
#ifdef TEXT_DOS_STYLE
    wxString result;
    unsigned i;

    // Remove carriage returns
    result = wxTextCtrl::GetValue();
    for (i = 0; i < result.length();) {
        if (result[i] != '\r')
            i++;
        else
            result.erase(i, 1);
    }

    return result;
#else
    return super::GetValue();
#endif
}

ScrollZoomWindow::ScrollZoomWindow(wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style)
    : wxScrolledWindow(parent, id, pos, size, wxHSCROLL | wxVSCROLL | style)
    , mOffset(0, 0)
    , mZoomFactor(1.0)
    , mCanvasSize(GetSize())
{
}

ScrollZoomWindow::~ScrollZoomWindow() {}

void ScrollZoomWindow::PrepareDC(wxDC& dc)
{
    super::PrepareDC(dc);
    auto screenRatio = static_cast<float>(GetSize().x)/static_cast<float>(mCanvasSize.x);
    dc.SetUserScale(mZoomFactor*screenRatio, mZoomFactor*screenRatio);
}

void ScrollZoomWindow::SetCanvasSize(wxSize s)
{
    mCanvasSize = s;
    auto virtualSizeX = GetSize().x * mZoomFactor;
    auto virtualSizeY = (virtualSizeX * mCanvasSize.y) / mCanvasSize.x;
    SetVirtualSize(wxSize( virtualSizeX, virtualSizeY ));
}

void ScrollZoomWindow::SetZoom(float z)
{
    mZoomFactor = z;
    auto virtualSizeX = GetSize().x * mZoomFactor;
    auto virtualSizeY = (virtualSizeX * mCanvasSize.y) / mCanvasSize.x;
    SetVirtualSize(wxSize( virtualSizeX, virtualSizeY ));
}

float ScrollZoomWindow::GetZoom() const
{
    return mZoomFactor;
}

void ScrollZoomWindow::ChangeOffset(wxPoint deltaOffset)
{
    auto offset = GetViewStart();
    offset += deltaOffset;
    Scroll(offset);
}

MouseMoveScrollCanvas::MouseMoveScrollCanvas(wxWindow* parent, wxWindowID id,
    const wxPoint& pos,
    const wxSize& size, long style)
    : super(parent, id, pos, size, wxHSCROLL | wxVSCROLL | style)
    , mLastPos(0, 0)
    , mScrolledLastMove(false)
{
}

MouseMoveScrollCanvas::~MouseMoveScrollCanvas() {}

void MouseMoveScrollCanvas::OnMouseMove(wxMouseEvent& event)
{
    wxPoint thisPos = event.GetPosition();
    mScrolledLastMove = false;
    if (ShouldScrollOnMouseEvent(event)) {
        wxPoint changeInOffset = mLastPos - thisPos;
        ChangeOffset(changeInOffset);
        mScrolledLastMove = true;
    }
    mLastPos = thisPos;
}

void MouseMoveScrollCanvas::OnMousePinchToZoom(wxMouseEvent& event)
{
    // calculate where the point will be before and after, then adjust.
    auto point_before = [this](auto&& event) {
        wxClientDC dc(this);
        PrepareDC(dc);
        auto mouse_point = mLastPos;
        mouse_point.x = dc.DeviceToLogicalX(mouse_point.x);
        mouse_point.y = dc.DeviceToLogicalY(mouse_point.y);
        return mouse_point;
    }(event);
    SetZoom(GetZoom() * (1.0 + event.GetMagnification()));
    auto point_after = [this](auto&& event) {
        wxClientDC dc(this);
        PrepareDC(dc);
        auto mouse_point = mLastPos;
        mouse_point.x = dc.DeviceToLogicalX(mouse_point.x);
        mouse_point.y = dc.DeviceToLogicalY(mouse_point.y);
        return mouse_point;
    }(event);
    ChangeOffset((point_before - point_after) * GetZoom());
}

void MouseMoveScrollCanvas::OnMouseWheel(wxMouseEvent& event)
{
    auto pt = wxPoint(event.GetWheelAxis() == 1 ? event.GetWheelRotation() : 0, event.GetWheelAxis() == 0 ? (CalChartConfiguration::GetGlobalConfig().Get_ScrollDirectionNatural() ? -1 : 1) * event.GetWheelRotation() : 0);
    ChangeOffset(pt);
}

bool MouseMoveScrollCanvas::IsScrolling() const { return mScrolledLastMove; }

ClickDragCtrlScrollCanvas::ClickDragCtrlScrollCanvas(wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style)
    : super(parent, id, pos, size, wxHSCROLL | wxVSCROLL | style)
{
}

ClickDragCtrlScrollCanvas::~ClickDragCtrlScrollCanvas() {}

bool ClickDragCtrlScrollCanvas::ShouldScrollOnMouseEvent(
    const wxMouseEvent& event) const
{
    return event.Dragging() && event.ControlDown();
}

wxSizerFlags BasicSizerFlags()
{
    static const auto sizerFlags = wxSizerFlags{}.Border(wxALL, 2).Center().Proportion(0);
    return sizerFlags;
}

wxSizerFlags LeftBasicSizerFlags()
{
    static const auto sizerFlags = wxSizerFlags{}.Border(wxALL, 2).Left().Proportion(0);
    return sizerFlags;
}

wxSizerFlags RightBasicSizerFlags()
{
    static const auto sizerFlags = wxSizerFlags{}.Border(wxALL, 2).Right().Proportion(0);
    return sizerFlags;
}

wxSizerFlags ExpandSizerFlags()
{
    static const auto sizerFlags = wxSizerFlags{}.Border(wxALL, 2).Proportion(1).Expand();
    return sizerFlags;
}

wxFont CreateFont(int pixelSize, wxFontFamily family, wxFontStyle style, wxFontWeight weight)
{
    auto size = wxSize{ 0, fDIP(pixelSize) };
    return wxFont(size, family, style, weight);
}

wxFont ResizeFont(wxFont const& font, int pixelSize)
{
    auto newFont = font;
    newFont.SetPixelSize(wxSize{ 0, fDIP(pixelSize) });
    return newFont;
}
