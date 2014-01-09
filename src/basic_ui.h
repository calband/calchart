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
/**
 * Sets the icon of a frame to the band icon.
 * @param frame The frame to install the band icon to.
 */
void SetBandIcon(wxFrame *frame);

/**
 * A text subwindow that can respond to drag-and-drop.
 */
class FancyTextWin : public wxTextCtrl
{
public:
	/**
	 * Makes the subwindow.
	 * @param parent The parent for this dialog. If the parent is
	 * closed, this dialog will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the dialog box.
	 * @param pos The position of the dialog.
	 * @param size The size of the dialog.
	 * @param style The style in which the dialog will be drawn.
	 */
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
 * A canvas that does zooming and 'ctrlScrolling'.
 * ctrlScrolling scrolls the canvas when the user holds the ctrl key and moves
 * the mouse.
 * To make this work:
 * The OnPaint routine of any child class must call PrepareDC
 * at the beginning to make this canvas draw properly.
 * The OnMouseMove routine must be called when the mouse moves, so any child
 * class must recieve mouse events.
 */
class CtrlScrollCanvas : public wxScrolledWindow
{
private:
	typedef wxScrolledWindow super;
public:
	/**
	 * Makes the canvas.
	  * @param parent The parent for this dialog. If the parent is
	  * closed, this dialog will be closed too.
	  * @param id The identity of the window.
	  * @param caption The title of the dialog box.
	  * @param pos The position of the dialog.
	  * @param size The size of the dialog.
	  * @param style The style in which the dialog will be drawn.
	  */
	CtrlScrollCanvas(wxWindow *parent,
					 wxWindowID id = wxID_ANY,
					 const wxPoint& pos = wxDefaultPosition,
					 const wxSize& size = wxDefaultSize,
					 long style = 0);
	/**
	 * Cleanup.
	 */
	virtual ~CtrlScrollCanvas();

protected:
	/**
	 * Sets up the device context so that anything drawn to the
	 * canvas using that context will be scrolled.
	 */
	void PrepareDC(wxDC&);

public:
	/**
	 * Sets the zoom to a given percentage of the standard zoom.
	 * @param z The percentage of the standard zoom to which the
	 * canvas should be zoomed.
	 */
	virtual void SetZoom(float z);
	/**
	 * Returns the zoom factor.
	 * @return The zoom factor (the percentage that the current
	 * zoom is of the standard zoom).
	 */
	virtual float GetZoom() const;

	/**
	 * Called when the mouse is moved. This handles canvas scrolling.
	 * @param An event containing mouse-related information.
	 */
    virtual void OnMouseMove(wxMouseEvent &event);

private:
	
	/**
	 * The offset of the canvas as a result of scrolling.
	 */
	wxPoint mOffset;
	/**
	 * The position that the mouse was at when the last mouse event
	 * was sent to this canvas.
	 */
	wxPoint mLastPos;
	/**
	 * The zoom factor of the canvas. That is, the ratio of the
	 * current zoom to the standard zoom.
	 */
	float mZoomFactor;
};

#endif
