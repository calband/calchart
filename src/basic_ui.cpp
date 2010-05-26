/* basic_ui.cpp
 * Handle basic wxWindows classes
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "basic_ui.h"
#include "confgr.h"

#ifdef __CC_INCLUDE_BITMAPS__
#include "calchart.xpm"
#endif

// Function for allowing XOR drawing
void SetXOR(wxDC *dc)
{
	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	dc->SetPen(*CalChartPens[COLOR_SHAPES]);
/*
dc->SetPen(wxWHITE_PEN);
dc->SetLogicalFunction(wxINVERT);
*/
}


// Set icon to band's insignia
void SetBandIcon(wxFrame *frame)
{
#ifdef __CC_SET_ICON__
	wxIcon icon(ICON_NAME(calchart));
	frame->SetIcon(icon);
#endif
}


// Text subwindow that can respond to drag-and-drop
FancyTextWin::FancyTextWin(wxWindow* parent, wxWindowID id,
const wxString& value,
const wxPoint& pos,
const wxSize& size,
long style,
const wxValidator& validator,
const wxString& name)
:wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
	SetDropTarget(new FancyTextWinDropTarget(this));
}


#ifdef TEXT_DOS_STYLE
wxString FancyTextWin::GetValue(void) const
{
	wxString result;
	unsigned i;

// Remove carriage returns
	result = wxTextCtrl::GetValue();
	for (i = 0; i < result.length();)
	{
		if (result[i] != '\r') i++;
		else result.erase(i,1);
	}

	return result;
}
#endif

bool FancyTextWinDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	if (filenames.Count() > 0)
	{
		win->LoadFile(filenames[0]);
	}
	return true;
}


AutoScrollCanvas::
AutoScrollCanvas(wxWindow *parent, wxWindowID id,
const wxPoint& pos, const wxSize& size,
long style, const wxString& name)
: wxPanel(parent, id, pos, size, style, name),
memdc(NULL), membm(NULL),
x_scale(1.0), y_scale(1.0), palette(NULL)
{
	if (size.GetX() != wxDefaultSize.GetX() ||
		size.GetY() != wxDefaultSize.GetY())
	{
		SetSize(size);
	}
}


AutoScrollCanvas::~AutoScrollCanvas()
{
	FreeMem();
}


void AutoScrollCanvas::SetSize(const wxSize& size)
{
	wxWindowDC dc(this);

	dc.SetLogicalFunction(wxCOPY);
	Refresh();
	FreeMem();
	x_off = y_off = 0.0;
	membm = new wxBitmap(size.GetX(), size.GetY());
	if (!membm->Ok())
	{
		FreeMem();
	}
	else
	{
		memdc = new wxMemoryDC(*membm);
		if (!memdc->IsOk())
		{
			FreeMem();
		}
		else
		{
			memdc->SelectObject(*membm);
			memdc->SetBackground(dc.GetBackground());
			if (palette) memdc->SetPalette(*palette);
			memdc->SetUserScale(x_scale, y_scale);
			memdc->Clear();
		}
	}
}


void AutoScrollCanvas::SetBackground(const wxBrush& brush)
{
	wxWindow::SetBackgroundColour(brush.GetColour());
	if (memdc) memdc->SetBackground(brush);
}


void AutoScrollCanvas::SetPalette(wxPalette *pal)
{
	palette = pal;
	wxWindow::SetPalette(*palette);
	if (memdc) memdc->SetPalette(*palette);
}


void AutoScrollCanvas::SetUserScale(float x, float y)
{
	x_scale = x;
	y_scale = y;
	if (memdc) memdc->SetUserScale(x, y);
}


void AutoScrollCanvas::Move(float x, float y, bool noscroll)
{
	if (memdc)
	{
		if (!noscroll)
		{
			x_off += (x - last_pos.x);
			y_off += (y - last_pos.y);
		}
	}
	last_pos.x = x;
	last_pos.y = y;
}


void AutoScrollCanvas::Blit(wxDC& dc)
{
	if (memdc)
	{
		memdc->SetUserScale(1.0, 1.0);
// Set the fg and bg colors in case source is a mono bitmap
		dc.SetPen(*wxWHITE_PEN);
		dc.SetBackground(*wxBLACK_BRUSH);
		dc.Blit(x_off, y_off, membm->GetWidth(), membm->GetHeight(),
			memdc, 0, 0, wxCOPY);
// Restore original background
		dc.SetBackground(memdc->GetBackground());
		memdc->SetUserScale(x_scale, y_scale);
	}
}


void AutoScrollCanvas::FreeMem()
{
	if (memdc)
	{
		delete memdc;
		memdc = NULL;
	}
	if (membm)
	{
		delete membm;
		membm = NULL;
	}
}


wxToolBar* CreateCoolToolBar(const ToolBarEntry *entries, size_t n, wxFrame *frame, wxWindowID id, const wxString& name)
{
	wxToolBar* tb = frame->CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL, id, name);
	
	for (size_t i = 0; i < n; i++)
	{
		tb->AddTool(entries[i].id, wxT(""), *(entries[i].bm),
			entries[i].desc,
			entries[i].kind);
		if (entries[i].space) tb->AddSeparator();
	}
	tb->Realize();

	return tb;
}
