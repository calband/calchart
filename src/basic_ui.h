#pragma once
/*
 * basic_ui.h
 * Header for basic wxWindows classes
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

#include "CalChartSizes.h"

#include <vector>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/toolbar.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

class CalChartView;
namespace CalChart {
class Configuration;
}

static constexpr auto kMultiple = "[multiple]";
static constexpr auto kDefaultInstrument = "default";
static constexpr auto kCustom = "custom...";

// Set icon to band's insignia
void SetBandIcon(wxFrame* frame);

// Set icon to band's insignia
wxStaticBitmap* BitmapWithBandIcon(wxWindow* parent, wxSize const& size = wxDefaultSize);
wxStaticText* TextStringWithSize(wxWindow* parent, std::string const& label, int pointSize);
wxHyperlinkCtrl* LinkStringWithSize(wxWindow* parent, std::string const& label, std::string const& url, int pointSize);
wxStaticLine* LineWithLength(wxWindow* parent, int length, long style = wxLI_HORIZONTAL);

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextCtrl {
    using super = wxTextCtrl;

public:
    FancyTextWin(wxWindow* parent, wxWindowID id,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTE_MULTILINE | wxHSCROLL | wxTE_PROCESS_TAB);
    wxString GetValue(void) const override;
};

/**
 * A window that can be scrolled and zoomed in on.
 * To use, call PrepareDC on your OnPaint routines with the dc.
 */
class ScrollZoomWindow : public wxScrolledWindow {
    using super = wxScrolledWindow;
    DECLARE_EVENT_TABLE()

public:
    ScrollZoomWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ScrollZoomWindow() override;

protected:
    virtual void PrepareDC(wxDC&) override;

public:
    // inform the scrolling window of the size of the underlying canvas.
    void SetCanvasSize(wxSize z);

    void SetZoom(float z);
    void SetZoom(float z, wxPoint center);
    float GetZoom() const;

    void ChangeOffset(wxPoint deltaOffset);

private:
    void HandleSizeEvent(wxSizeEvent& event);

    void SetupSize();
    wxPoint mOffset;
    float mZoomFactor;
    wxSize mCanvasSize;
};

/**
 * A canvas that scrolls according to the movement of the mouse. The amount that
 * the canvas
 * scrolls is directly proportional to the mouse movement.
 * To make this work, OnMouseMove(...) should be called in response to mouse
 * move events.
 */
class MouseMoveScrollCanvas : public ScrollZoomWindow {
    using super = ScrollZoomWindow;

public:
    MouseMoveScrollCanvas(CalChart::Configuration const& config, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~MouseMoveScrollCanvas() override;

protected:
    // Inform the ctrl scrolling when the mouse moves
    virtual void OnMouseMove(wxMouseEvent& event);
    virtual void OnMousePinchToZoom(wxMouseEvent& event);
    virtual void OnMouseWheel(wxMouseEvent& event);

    virtual bool IsScrolling() const;

    virtual bool ShouldScrollOnMouseEvent(const wxMouseEvent& event) const = 0;

private:
    wxPoint mLastPos;
    bool mScrolledLastMove;
    CalChart::Configuration const& mConfig;
};

/**
 * A canvas that scrolls when the ctrl button is pressed and the user clicks and
 * drags the mouse.
 */
class ClickDragCtrlScrollCanvas : public MouseMoveScrollCanvas {
    using super = MouseMoveScrollCanvas;

public:
    ClickDragCtrlScrollCanvas(CalChart::Configuration const& config, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ClickDragCtrlScrollCanvas() override;

protected:
    virtual bool ShouldScrollOnMouseEvent(const wxMouseEvent& event) const override;
};

// common sizers all in one places
wxSizerFlags BasicSizerFlags();
wxSizerFlags LeftBasicSizerFlags();
wxSizerFlags RightBasicSizerFlags();
wxSizerFlags ExpandSizerFlags();
wxSizerFlags HorizontalSizerFlags();
wxSizerFlags VerticalSizerFlags();

template <typename T>
void AddToSizerBasic(wxSizer* sizer, T window)
{
    sizer->Add(window, BasicSizerFlags());
}

template <typename T>
void AddToSizerExpand(wxSizer* sizer, T window)
{
    sizer->Add(window, ExpandSizerFlags());
}

template <typename T>
void AddToSizerRight(wxSizer* sizer, T window)
{
    sizer->Add(window, RightBasicSizerFlags());
}

template <typename Brush>
auto CreateItemBitmap(Brush const& brush)
{
    wxBitmap temp_bitmap(GetColorBoxSize());
    wxMemoryDC temp_dc;
    temp_dc.SelectObject(temp_bitmap);
    temp_dc.SetBackground(brush);
    temp_dc.Clear();
    return temp_bitmap;
}

template <typename T, typename Int, typename Brush>
void CreateAndSetItemBitmap(T* target, Int which, Brush const& brush)
{
    target->SetItemBitmap(which, CreateItemBitmap(brush));
}

wxFont CreateFont(int pixelSize, wxFontFamily family = wxFONTFAMILY_SWISS, wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL);
wxFont ResizeFont(wxFont const& font, int pixelSize);

template <typename String>
inline auto StringSizeX(wxControl* controller, String const& string)
{
    return controller->GetTextExtent(string).x;
}

template <typename Strings>
inline auto BestSizeX(wxControl* controller, Strings const& strings)
{
    return controller->GetTextExtent(*std::max_element(std::begin(strings), std::end(strings), [controller](auto a, auto b) {
                         return controller->GetTextExtent(a).x < controller->GetTextExtent(b).x;
                     }))
        .x;
}

// Give a wxUI::Widgets a vertical label
template <wxUI::details::SizerItem W>
auto VLabelWidget(std::string caption, W&& widget)
{
    return wxUI::VSizer{
        wxUI::Text{ caption },
        std::forward<W>(widget),
    };
}

#pragma GCC diagnostic pop
