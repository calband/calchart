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

#ifndef _BASIC_UI_H_
#define _BASIC_UI_H_

#include <wx/wx.h>
#include <wx/toolbar.h>

#include <vector>

// Set icon to band's insignia
void SetBandIcon(wxFrame *frame);

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextCtrl
{
public:
	FancyTextWin(wxWindow* parent, wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTE_MULTILINE|wxHSCROLL);
#ifdef TEXT_DOS_STYLE
	wxString GetValue(void) const;
#endif
};


/**
 * A canvas that can be scrolled and zoomed in on.
 * To use, call PrepareDC on your OnPaint routines with the dc.
 */
class ScrollZoomCanvas : public wxScrolledWindow
{
private:
	typedef wxScrolledWindow super;
public:
	ScrollZoomCanvas(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	virtual ~ScrollZoomCanvas();

protected:
	void PrepareDC(wxDC&);

public:
	virtual void SetZoom(float z);
	virtual float GetZoom() const;

	virtual void SetOffset(wxPoint newOffset);
	virtual void ChangeOffset(wxPoint deltaOffset);
	virtual wxPoint GetOffset() const;
	virtual void ResetScrollToOrigin();

	virtual void SetOffsetOrigin(wxPoint newOrigin);
	virtual void ChangeOffsetOrigin(wxPoint deltaOrigin);
	virtual wxPoint GetOffsetOrigin() const;
private:
	wxPoint mOrigin;
	wxPoint mOffset;
	float mZoomFactor;
};

/**
 * A canvas that scrolls according to the movement of the mouse. The amount that the canvas
 * scrolls is directly proportional to the mouse movement.
 * To make this work, OnMouseMove(...) should be called in response to mouse move events.
 */
class MouseMoveScrollCanvas : public ScrollZoomCanvas
{
public:
	MouseMoveScrollCanvas(wxWindow *parent,
					 wxWindowID id = wxID_ANY,
					 const wxPoint& pos = wxDefaultPosition,
					 const wxSize& size = wxDefaultSize,
					 long style = 0);
	virtual ~MouseMoveScrollCanvas();
public:
	// Inform the ctrl scrolling when the mouse moves
    virtual void OnMouseMove(wxMouseEvent &event);
	virtual bool IsScrolling();
protected:
	virtual bool ShouldScrollOnMouseEvent(wxMouseEvent &event) = 0;
	virtual float GetScrollFactor() = 0;
private:
	wxPoint mLastPos;
	bool mScrolledLastMove;
};

/**
 * A canvas that scrolls when the ctrl button is pressed and the mouse is moved.
 */
class CtrlScrollCanvas : public MouseMoveScrollCanvas
{
public:
	CtrlScrollCanvas(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	virtual ~CtrlScrollCanvas();
protected:
	virtual bool ShouldScrollOnMouseEvent(wxMouseEvent &event);
	virtual float GetScrollFactor();
private:
	wxPoint mOffset;
	wxPoint mLastPos;
	float mZoomFactor;
};

/**
 * A canvas that scrolls when the ctrl button is pressed and the user clicks and drags the mouse.
 */
class ClickDragCtrlScrollCanvas : public CtrlScrollCanvas
{
public:
	ClickDragCtrlScrollCanvas(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	virtual ~ClickDragCtrlScrollCanvas();
protected:
	virtual bool ShouldScrollOnMouseEvent(wxMouseEvent &event);
};

#endif
