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
void SetXOR(wxDC *dc) {
  dc->SetBrush(*wxTRANSPARENT_BRUSH);
  dc->SetPen(*CalChartPens[COLOR_SHAPES]);
  /*
  dc->SetPen(wxWHITE_PEN);
  dc->SetLogicalFunction(wxINVERT);
  */
}

#ifndef __WXMSW__
// Set icon to band's insignia
void SetBandIcon(wxFrame *frame) {
  wxIcon icon(ICON_NAME(calchart));
  frame->SetIcon(icon);
}
#endif

// Text subwindow that can respoind to drag-and-drop
FancyTextWin::FancyTextWin(wxWindow* parent, wxWindowID id,
			   const wxString& value = wxEmptyString,
			   const wxPoint& pos = wxDefaultPosition,
			   const wxSize& size = wxDefaultSize,
			   long style = wxTE_MULTILINE|wxHSCROLL,
			   const wxValidator& validator = wxDefaultValidator,
			   const wxString& name = wxTextCtrlNameStr)
:wxTextCtrl(parent, id, value, pos, size, style, validator, name) {
  //  DragAcceptFiles(true);
}

#ifdef TEXT_DOS_STYLE
wxString FancyTextWin::GetValue(void) const {
  wxString result;
  unsigned i;

  // Remove carriage returns
  result = wxTextCntl::GetValue();
  for (i = 0; i < result.length();) {
    if (result[i] != '\r') i++;
    else result.erase(i,1);
  }

  return result;
}
#endif

/*
void FancyTextWin::OnDropFiles(int, char *files[], int, int) {
  LoadFile(files[0]);
}
*/

// Toolbar-handling frame constructor
wxFrameWithStuff::
wxFrameWithStuff(wxWindow *parent, wxWindowID id,
		 const wxString& title,
		 const wxPoint& pos, const wxSize& size,
		 long style, const wxString& name)
: wxFrame(parent, id, title, pos, size, style, name) {
  frameToolBar = NULL;
  framePanel = NULL;
  frameCanvas = NULL;
  SetAutoLayout(true);
}

// Reposition the toolbar and child subwindow
void wxFrameWithStuff::DoLayout() {
  wxSize maxToolBarSize;
  wxLayoutConstraints *lcTb, *lcPnl, *lcCnv;

  if (framePanel) {
    framePanel->Fit();
    int width_pnl, height_pnl;
    framePanel->GetSize(&width_pnl, &height_pnl);
    lcPnl = new wxLayoutConstraints;
    lcPnl->width.Absolute(width_pnl);
    lcPnl->height.Absolute(height_pnl);
    lcPnl->top.SameAs(this, wxTop, 0);
  }
  if (frameToolBar) {
    maxToolBarSize = frameToolBar->GetMaxSize();
    lcTb = new wxLayoutConstraints;
    lcTb->width.Absolute(maxToolBarSize.GetX());
    lcTb->height.Absolute(maxToolBarSize.GetY());
    if (frameToolBar)
      lcTb->top.Below(framePanel, 0);
    else
      lcTb->top.SameAs(this, wxTop, 0);
  }
  if (frameCanvas) {
    lcCnv = new wxLayoutConstraints;
    lcCnv->right.SameAs(this, wxRight, 0);
    if (framePanel)
      lcCnv->top.Below(framePanel, 0);
    else if (frameToolBar)
      lcCnv->top.Below(frameToolBar, 0);
    else
      lcCnv->top.SameAs(this, wxTop, 0);
  }
}

// Toolbar-handling frame constructor
wxFrameWithStuffSized::
wxFrameWithStuffSized(wxWindow *parent, wxWindowID id,
		      const wxString& title,
		      const wxPoint& pos, const wxSize& size,
		      long style, const wxString& name)
: wxFrameWithStuff(parent, id, title, pos, size, style, name) {
}

// Shrink frame to fit around a given canvas size
void wxFrameWithStuffSized::Fit()
{
  wxSize maxToolBarSize;
  int width_frm, width_tb, width_pnl, width_cnv;
  int height_frm, height_tb, height_pnl, height_cnv;

  frameCanvas->GetSize(&width_cnv, &height_cnv);

  if (frameToolBar) {
    maxToolBarSize = frameToolBar->GetMaxSize();
    width_tb = maxToolBarSize.GetX();
    height_tb = maxToolBarSize.GetY();
  } else {
    width_tb = 0;
    height_tb = 0;
  }
  if (framePanel) {
    framePanel->Fit();
    framePanel->GetClientSize(&width_pnl, &height_pnl);
  } else {
    width_pnl = 0;
    height_pnl = 0;
  }
  width_frm = width_cnv;
  if (width_pnl > width_frm) width_frm = width_pnl;
  if (width_tb > width_frm) width_frm = width_tb;

  height_frm = height_cnv + height_tb + height_pnl;

  SetClientSize(width_frm, height_frm);
  GetSize(&width_frm, &height_frm);
  SetSizeHints(width_frm, height_frm, width_frm, height_frm);

  Layout();
}

AutoScrollCanvas::
AutoScrollCanvas(wxWindow *parent, wxWindowID id,
		 const wxPoint& pos, const wxSize& size,
		 long style, const wxString& name)
: wxWindow(parent, id, pos, size, style, name),
  memdc(NULL), membm(NULL),
  x_scale(1.0), y_scale(1.0), palette(NULL)
{
  if (size.GetX() != wxDefaultSize.GetX() ||
      size.GetY() != wxDefaultSize.GetY()) {
    SetSize(size);
  }
}

AutoScrollCanvas::~AutoScrollCanvas() {
  FreeMem();
}

void AutoScrollCanvas::SetSize(const wxSize& size) {
  wxWindowDC dc(this);

  dc.SetLogicalFunction(wxCOPY);
  Clear();
  FreeMem();
  x_off = y_off = 0.0;
  memdc = new wxMemoryDC();
  if (!memdc->Ok()) {
    FreeMem();
  } else {
    membm = new wxBitmap(size.GetX(), size.GetY());
    if (!membm->Ok()) {
      FreeMem();
    } else {
      memdc->SelectObject(*membm);
      memdc->SetBackground(dc.GetBackground());
      memdc->SetPalette(palette);
      memdc->SetUserScale(x_scale, y_scale);
      memdc->Clear();
    }
  }
}

void AutoScrollCanvas::SetBackground(wxBrush *brush) {
  wxWindow::SetBackground(brush);
  if (memdc) memdc->SetBackground(brush);
}

void AutoScrollCanvas::SetPalette(wxPalette *pal) {
  palette = pal;
  wxWindow::SetPalette(palette);
  if (memdc) memdc->SetPalette(palette);
}

void AutoScrollCanvas::SetUserScale(float x, float y) {
  wxWindowDC dc(this);
  x_scale = x;
  y_scale = y;
  dc.SetUserScale(x, y);
  if (memdc) memdc->SetUserScale(x, y);
}

void AutoScrollCanvas::Move(float x, float y, bool noscroll) {
  if (memdc) {
    if (!noscroll) {
      x_off += (x - last_pos.x) * x_scale;
      y_off += (y - last_pos.y) * y_scale;
    }
  }
  last_pos.x = x;
  last_pos.y = y;
}

void AutoScrollCanvas::Blit() {
  if (memdc) {
    wxWindowDC dc(this);
    dc.SetUserScale(1.0, 1.0);
    memdc->SetUserScale(1.0, 1.0);
    if (!dc.Colour) {
      // Source is a mono bitmap, so we must set the fg and bg colors
      dc.SetPen(wxWHITE_PEN);
      dc.SetBackground(wxBLACK_BRUSH);
    }
    dc.Blit(x_off, y_off, membm->GetWidth(), membm->GetHeight(),
	    memdc, 0, 0, wxCOPY);
    if (!dc.Colour) {
      // Restore original background
      dc.SetBackground(memdc->GetBackground());
    }
    dc.SetUserScale(x_scale, y_scale);
    memdc->SetUserScale(x_scale, y_scale);
  }
}

void AutoScrollCanvas::FreeMem() {
  if (memdc) {
    delete memdc;
    memdc = NULL;
  }
  if (membm) {
    delete membm;
    membm = NULL;
  }
}

CoolToolBar::CoolToolBar(wxFrame *frame, wxWindowID id,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 long style = wxTB_HORIZONTAL | wxNO_BORDER,
			 const wxString& name = wxPanelNameStr)
: wxToolBar(frame, id, pos, size, style, name),
  ourframe(frame) {}

void CoolToolBar::SetupBar(ToolBarEntry *tbe, int n)
{
  entries = tbe;
  GetDC()->SetBackground(*wxGREY_BRUSH);

  for (int i = 0; i < n; i++) {
    AddTool(i, *(entries[i].bm), wxNullBitmap,
	    (entries[i].flags & TOOLBAR_TOGGLE),
	    -1, -1, NULL);
    if (entries[i].flags & TOOLBAR_SPACE) AddSeparator();
  }
  CreateTools();
  Layout();
}

bool CoolToolBar::OnLeftClick(int toolIndex, bool) {
  if (entries[toolIndex].func) entries[toolIndex].func(this);
  return true;
}

void CoolToolBar::OnMouseEnter(int toolIndex) {
  if (toolIndex > -1)
  {
    ourframe->SetStatusText(entries[toolIndex].desc);
  }
  else ourframe->SetStatusText("");
}
