/* basic_ui.h
 * Header for basic wxWindows classes
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/toolbar.h>

#include "platconf.h"

#define CC_USE_MDI
#ifdef CC_USE_MDI
typedef wxMDIParentFrame CC_MDIParentFrame;
typedef wxMDIChildFrame CC_MDIChildFrame;
#define CC_FRAME_TOP (wxMAXIMIZE | wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_CHILD (wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_OTHER (wxDEFAULT_FRAME_STYLE)
#else
typedef wxFrame CC_MDIParentFrame;
typedef wxFrame CC_MDIChildFrame;
#define CC_FRAME_TOP (wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_CHILD (wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_OTHER (wxDEFAULT_FRAME_STYLE)
#endif

// Function for allowing XOR drawing
void SetXOR(wxDC *dc);

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

class FancyTextWinDropTarget : public wxFileDropTarget
{
public:
	FancyTextWinDropTarget(FancyTextWin *w) : win(w) {}
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
private:
	FancyTextWin *win;
};

class AutoScrollCanvas: public wxPanel
{
public:
	AutoScrollCanvas(wxWindow *parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxT("canvas"));
	~AutoScrollCanvas();

	inline wxDC *GetMemDC() { return memdc; }
	void SetSize(const wxSize& size);
	void SetBackground(const wxBrush& brush);
	void SetPalette(wxPalette *palette);
	void SetUserScale(float x, float y);
	inline float GetPositionX() const { return x_off/x_scale; }
	inline float GetPositionY() const { return y_off/y_scale; }
	inline float GetScaleX() const { return x_scale; }
	inline float GetScaleY() const { return y_scale; }

	void Move(float x, float y, bool noscroll=0);
	void Blit(wxDC& dc);

protected:
	float x_off, y_off;

private:
	void FreeMem();

	wxMemoryDC *memdc;
	wxBitmap *membm;
	float x_scale, y_scale;
	wxPalette *palette;
	wxPoint last_pos;
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
