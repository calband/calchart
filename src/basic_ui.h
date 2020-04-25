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

#pragma once

#include <wx/statline.h>
#include <wx/toolbar.h>
#include <wx/wx.h>

#include <vector>

// Set icon to band's insignia
void SetBandIcon(wxFrame* frame);

// Set icon to band's insignia
wxStaticBitmap* BitmapWithBandIcon(wxWindow* parent, wxSize const& size = wxDefaultSize);
wxStaticText* TextStringWithSize(wxWindow* parent, std::string const& label, int pointSize);
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
public:
    FancyTextWin(wxWindow* parent, wxWindowID id,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTE_MULTILINE | wxHSCROLL | wxTE_PROCESS_TAB);
#ifdef TEXT_DOS_STYLE
    wxString GetValue(void) const;
#endif
};

/**
 * A window that can be scrolled and zoomed in on.
 * To use, call PrepareDC on your OnPaint routines with the dc.
 */
class ScrollZoomWindow : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    ScrollZoomWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~ScrollZoomWindow() override;

protected:
    virtual void PrepareDC(wxDC&) override;

    // inform the scrolling window of the size of the underlying canvas.
    void SetCanvasSize(wxSize z);

    void SetZoom(float z);
    float GetZoom() const;

public:
    void ChangeOffset(wxPoint deltaOffset);

private:
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
