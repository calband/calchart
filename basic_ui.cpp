/* basic_ui.cc
 * Handle basic wxWindows classes
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "basic_ui.h"

// Function for allowing XOR drawing
void SetXOR(wxDC *dc) {
  dc->SetBrush(wxTRANSPARENT_BRUSH);
  dc->SetPen(wxWHITE_PEN);
  dc->SetLogicalFunction(wxINVERT);
}

// Text subwindow that can respoind to drag-and-drop
FancyTextWin::FancyTextWin(wxFrame *frame, int x, int y,
			   int width, int height, long style)
:wxTextWindow(frame, x, y, width, height, style) {
  DragAcceptFiles(TRUE);
}

void FancyTextWin::OnDropFiles(int, char *files[], int, int) {
  LoadFile(files[0]);
}

GoodListBox::GoodListBox(wxPanel *panel, wxFunction func, char *Title,
			 Bool Multiple, int x, int y, int width, int height,
			 int N, char **Choices, long style, char *name)
  : wxListBox(panel, func, Title, Multiple, x, y, width, height,
	      N, Choices, style, name) {}

void GoodListBox::SetSelection(int N, Bool select) {
  // so Motif version works
  if (!select || !Selected(N)) {
    wxListBox::SetSelection(N, select);
  }
}

// Toolbar-handling frame constructor
wxFrameWithStuff::wxFrameWithStuff(wxFrame *frame, char *title,
				   int x, int y, int w, int h,
				   long style)
: wxFrame(frame, title, x, y, w, h, style) {
  frameToolBar = NULL;
  framePanel = NULL;
  frameCanvas = NULL;
  layout = wxFRAMESTUFF_TB_PNL;
}

Bool wxFrameWithStuff::OnClose(void) {
  return TRUE;
}

// Reposition the toolbar and child subwindow
void wxFrameWithStuff::OnSize(int, int) {
  float maxToolBarWidth, maxToolBarHeight;
  int width_frm, width_pnl;
  int height_frm, height_tb, height_pnl, height_cnv;
  int y_tb, y_pnl, y_cnv;

  GetClientSize(&width_frm, &height_frm);

  if (frameToolBar) {
    frameToolBar->GetMaxSize(&maxToolBarWidth, &maxToolBarHeight);
    height_tb = (int)maxToolBarHeight;
  } else {
    height_tb = 0;
  }
  if (framePanel) {
    framePanel->Fit();
    framePanel->GetSize(&width_pnl, &height_pnl);
  } else {
    height_pnl = 0;
  }
  height_cnv = height_frm - height_tb - height_pnl;

  switch (layout) {
  case wxFRAMESTUFF_TB_PNL:
    y_tb = 0;
    y_pnl = height_tb;
    y_cnv = height_tb + height_pnl;
    break;
  case wxFRAMESTUFF_TB_CNV:
    y_tb = 0;
    y_pnl = height_tb + height_cnv;
    y_cnv = height_tb;
    break;
  case wxFRAMESTUFF_PNL_TB:
    y_tb = height_pnl;
    y_pnl = 0;
    y_cnv = height_pnl + height_tb;
    break;
  case wxFRAMESTUFF_PNL_CNV:
    y_tb = height_pnl + height_cnv;
    y_pnl = 0;
    y_cnv = height_pnl;
    break;
  case wxFRAMESTUFF_CNV_TB:
    y_tb = height_cnv;
    y_pnl = height_cnv + height_tb;
    y_cnv = 0;
    break;
  case wxFRAMESTUFF_CNV_PNL:
  default:
    y_tb = height_cnv + height_pnl;
    y_pnl = height_cnv;
    y_cnv = 0;
    break;
  }
  if (frameToolBar) frameToolBar->SetSize(0, y_tb, width_frm, height_tb);
  if (framePanel) framePanel->SetSize(0, y_pnl, width_frm, height_pnl);
  if (frameCanvas) frameCanvas->SetSize(0, y_cnv, width_frm, height_cnv);
}

// Toolbar-handling frame constructor
wxFrameWithStuffSized::wxFrameWithStuffSized(wxFrame *frame, char *title,
					     int x, int y, long style)
: wxFrameWithStuff(frame, title, x, y, 320, 200, style) {
}

// Reposition the toolbar and child subwindow
void wxFrameWithStuffSized::OnSize(int, int) {
  float maxToolBarWidth, maxToolBarHeight;
  int width_frm, width_pnl, width_cnv;
  int height_frm, height_tb, height_pnl, height_cnv;
  int y_tb, y_pnl, y_cnv;

  GetClientSize(&width_frm, &height_frm);

  if (frameToolBar) {
    frameToolBar->GetMaxSize(&maxToolBarWidth, &maxToolBarHeight);
    height_tb = (int)maxToolBarHeight;
  } else {
    height_tb = 0;
  }
  if (framePanel) {
    framePanel->Fit();
    framePanel->GetSize(&width_pnl, &height_pnl);
  } else {
    height_pnl = 0;
  }
  if (frameCanvas) {
    frameCanvas->GetSize(&width_cnv, &height_cnv);
  } else {
    width_cnv = 0;
    height_cnv = 0;
  }

  switch (layout) {
  case wxFRAMESTUFF_TB_PNL:
    y_tb = 0;
    y_pnl = height_tb;
    y_cnv = height_tb + height_pnl;
    break;
  case wxFRAMESTUFF_TB_CNV:
    y_tb = 0;
    y_pnl = height_tb + height_cnv;
    y_cnv = height_tb;
    break;
  case wxFRAMESTUFF_PNL_TB:
    y_tb = height_pnl;
    y_pnl = 0;
    y_cnv = height_pnl + height_tb;
    break;
  case wxFRAMESTUFF_PNL_CNV:
    y_tb = height_pnl + height_cnv;
    y_pnl = 0;
    y_cnv = height_pnl;
    break;
  case wxFRAMESTUFF_CNV_TB:
    y_tb = height_cnv;
    y_pnl = height_cnv + height_tb;
    y_cnv = 0;
    break;
  case wxFRAMESTUFF_CNV_PNL:
  default:
    y_tb = height_cnv + height_pnl;
    y_pnl = height_cnv;
    y_cnv = 0;
    break;
  }
  if (frameToolBar) frameToolBar->SetSize(0, y_tb, width_frm, height_tb);
  if (framePanel) framePanel->SetSize(0, y_pnl, width_frm, height_pnl);
  if (frameCanvas) frameCanvas->SetSize(0, y_cnv, width_cnv, height_cnv);
}

// Shrink frame to fit around a given canvas size
void wxFrameWithStuffSized::Fit()
{
  float maxToolBarWidth, maxToolBarHeight;
  int width_frm, width_tb, width_pnl, width_cnv;
  int height_frm, height_tb, height_pnl, height_cnv;

  frameCanvas->GetSize(&width_cnv, &height_cnv);

  if (frameToolBar) {
    frameToolBar->GetMaxSize(&maxToolBarWidth, &maxToolBarHeight);
    width_tb = (int)maxToolBarWidth;
    height_tb = (int)maxToolBarHeight;
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
  height_frm = height_cnv + height_tb + height_pnl;

  SetClientSize(width_frm, height_frm);
#ifndef BUGGY_SIZE_HINTS
  GetSize(&width_frm, &height_frm);
  SetSizeHints(width_frm, height_frm, width_frm, height_frm);
#endif

  OnSize(-1, -1);
}

CoolToolBar::CoolToolBar(wxFrame *frame, int x, int y, int w, int h,
            long style, int direction, int RowsOrColumns):
  PlainToolBar(frame, x, y, w, h, style, direction, RowsOrColumns),
  ourframe(frame) {}

void CoolToolBar::SetupBar(ToolBarEntry *tbe, int n)
{
  entries = tbe;
  GetDC()->SetBackground(wxGREY_BRUSH);

  for (int i = 0; i < n; i++) {
    AddTool(i, entries[i].bm, NULL, FALSE, -1, -1, NULL);
    if (entries[i].flags & TOOLBAR_SPACE) AddSeparator();
  }
  CreateTools();
  Layout();
}

Bool CoolToolBar::OnLeftClick(int toolIndex, Bool) {
  if (entries[toolIndex].func) entries[toolIndex].func(this);
  return TRUE;
}

void CoolToolBar::OnMouseEnter(int toolIndex) {
  if (toolIndex > -1)
  {
    ourframe->SetStatusText(entries[toolIndex].desc);
  }
  else ourframe->SetStatusText("");
}
