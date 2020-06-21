#pragma once
/*
 * basic_ui.h
 * Header for basic wxWindows classes
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

#include "CalChartSizes.h"

#include <vector>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

class CalChartView;

// Set icon to band's insignia
void SetBandIcon(wxFrame* frame);

// Set icon to band's insignia
wxStaticBitmap* BitmapWithBandIcon(wxWindow* parent, wxSize const& size = wxDefaultSize);
wxStaticText* TextStringWithSize(wxWindow* parent, std::string const& label, int pointSize);
wxHyperlinkCtrl* LinkStringWithSize(wxWindow* parent, std::string const& label, std::string const& url, int pointSize);
wxStaticLine* LineWithLength(wxWindow* parent, int length, long style = wxLI_HORIZONTAL);

// class for saving and restoring
class SaveAndRestoreBrushAndPen {
    wxDC& mDC;
    wxBrush const& mBrush;
    wxPen const& mPen;

public:
    SaveAndRestoreBrushAndPen(wxDC& dc)
        : mDC(dc)
        , mBrush(dc.GetBrush())
        , mPen(dc.GetPen())
    {
    }
    ~SaveAndRestoreBrushAndPen()
    {
        mDC.SetBrush(mBrush);
        mDC.SetPen(mPen);
    }
};

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
    MouseMoveScrollCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
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
};

/**
 * A canvas that scrolls when the ctrl button is pressed and the user clicks and
 * drags the mouse.
 */
class ClickDragCtrlScrollCanvas : public MouseMoveScrollCanvas {
    using super = MouseMoveScrollCanvas;

public:
    ClickDragCtrlScrollCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ClickDragCtrlScrollCanvas() override;

protected:
    virtual bool ShouldScrollOnMouseEvent(const wxMouseEvent& event) const override;
};

wxSizerFlags BasicSizerFlags();
wxSizerFlags LeftBasicSizerFlags();
wxSizerFlags RightBasicSizerFlags();
wxSizerFlags ExpandSizerFlags();

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

template <typename T, typename Int, typename Brush>
void CreateAndSetItemBitmap(T* target, Int which, Brush const& brush)
{
    wxBitmap temp_bitmap(GetColorBoxSize());
    wxMemoryDC temp_dc;
    temp_dc.SelectObject(temp_bitmap);
    temp_dc.SetBackground(brush);
    temp_dc.Clear();
    target->SetItemBitmap(which, temp_bitmap);
}

wxFont CreateFont(int pixelSize, wxFontFamily family = wxFONTFAMILY_SWISS, wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL);
wxFont ResizeFont(wxFont const& font, int pixelSize);

template <typename Strings>
auto BestSizeX(wxControl* controller, Strings const& strings)
{
    return controller->GetTextExtent(*std::max_element(std::begin(strings), std::end(strings), [controller](auto a, auto b) {
                         return controller->GetTextExtent(a).x < controller->GetTextExtent(b).x;
                     }))
        .x;
}
