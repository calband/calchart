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
		long style = wxTE_MULTILINE|wxHSCROLL,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxTextCtrlNameStr);
#ifdef TEXT_DOS_STYLE
	wxString GetValue(void) const;
#endif
};


///// CtrlScrollCanvas /////
// A canvas that does zooming and ctrlScrolling
// ctrlScrolling scrolls when you hold the ctrl key and move the mouse
// To use, call PrepareDC on your OnPaint routines with the dc.
// Call OnMouseMove on the MouseMove handler to inform the Canvas a move has occurred.
//////////
class CtrlScrollCanvas : public wxScrolledWindow
{
private:
	typedef wxScrolledWindow super;
public:
	CtrlScrollCanvas(wxWindow *parent,
					 wxWindowID id = wxID_ANY,
					 const wxPoint& pos = wxDefaultPosition,
					 const wxSize& size = wxDefaultSize,
					 long style = 0);
	virtual ~CtrlScrollCanvas();

protected:
	void PrepareDC(wxDC&);

public:
	void SetZoom(float z);
	float GetZoom() const;

	// Inform the ctrl scrolling when the mouse moves
    void OnMouseMove(wxMouseEvent &event);

private:
	wxPoint mOffset;
	wxPoint mLastPos;
	float mZoomFactor;
};

struct ToolBarEntry
{
	wxItemKind kind;
	wxBitmap *bm;
	const wxString desc;
	int id;
	bool space;
};

wxToolBar* CreateCoolToolBar(const ToolBarEntry *entries, size_t n, wxFrame *frame, wxWindowID id = -1, const wxString& name = wxToolBarNameStr);

#endif
