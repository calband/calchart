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
#include "confgr.h"

#ifdef wx_x
#include "calchart.xpm"
#endif

// Function for allowing XOR drawing
void SetXOR(wxDC *dc) {
  dc->SetBrush(wxTRANSPARENT_BRUSH);
  dc->SetPen(CalChartPens[COLOR_SHAPES]);
  /*
  dc->SetPen(wxWHITE_PEN);
  dc->SetLogicalFunction(wxINVERT);
  */
}

// Set icon to band's insignia
void SetBandIcon(wxFrame *frame) {
  frame->SetIcon(new wxIcon(ICON_NAME(calchart)));
}

// Text subwindow that can respoind to drag-and-drop
FancyTextWin::FancyTextWin(wxFrame *frame, int x, int y,
			   int width, int height, long style)
:wxTextWindow(frame, x, y, width, height, style) {
  DragAcceptFiles(TRUE);
}

#ifdef TEXT_DOS_STYLE
char *FancyTextWin::GetContents(void) {
  char *src;
  char *dest;
  unsigned i, len;

  // Copy string but not carriage returns
  src = wxTextWindow::GetContents();
  for (i = 0, len = 0; src[i]; i++) {
    if (src[i] != '\r') len++;
  }
  dest = new char[len+1];
  for (i = 0, len = 0; src[i]; i++) {
    if (src[i] != '\r') dest[len++] = src[i];
  }
  dest[len] = 0;

  delete src;
  return dest;
}
#endif

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

AutoScrollCanvas::AutoScrollCanvas(wxWindow *parent,
				   int x, int y, int w, int h):
  wxCanvas(parent, x, y, w, h, 0 /* not retained */), memdc(NULL), membm(NULL),
  x_scale(1.0), y_scale(1.0), cmap(NULL)
{
  if ((w > 0) && (h > 0)) {
    SetSize(w, h);
  }
}

AutoScrollCanvas::~AutoScrollCanvas() {
  FreeMem();
}

void AutoScrollCanvas::SetSize(int width, int height) {
  GetDC()->SetLogicalFunction(wxCOPY);
  Clear();
  FreeMem();
  x_off = y_off = 0.0;
  memdc = new wxMemoryDC(GetDC());
  if (!memdc->Ok()) {
    FreeMem();
  } else {
    membm = new wxBitmap(width, height);
    if (!membm->Ok()) {
      FreeMem();
    } else {
      memdc->SelectObject(membm);
      memdc->SetBackground(GetDC()->GetBackground());
      memdc->SetColourMap(cmap);
      memdc->SetUserScale(x_scale, y_scale);
      memdc->Clear();
    }
  }
}

void AutoScrollCanvas::SetBackground(wxBrush *brush) {
  wxCanvas::SetBackground(brush);
  if (memdc) memdc->SetBackground(brush);
}

void AutoScrollCanvas::SetColourMap(wxColourMap *colourMap) {
  cmap = colourMap;
  wxCanvas::SetColourMap(cmap);
  if (memdc) memdc->SetColourMap(cmap);
}

void AutoScrollCanvas::SetUserScale(float x, float y) {
  x_scale = x;
  y_scale = y;
  GetDC()->SetUserScale(x, y);
  if (memdc) memdc->SetUserScale(x, y);
}

void AutoScrollCanvas::Move(float x, float y, Bool noscroll) {
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
    wxDC *dc = GetDC();
    dc->SetUserScale(1.0, 1.0);
    memdc->SetUserScale(1.0, 1.0);
    if (!dc->Colour) {
      // Source is a mono bitmap, so we must set the fg and bg colors
      dc->SetPen(wxWHITE_PEN);
      dc->SetBackground(wxBLACK_BRUSH);
    }
    dc->Blit(x_off, y_off, membm->GetWidth(), membm->GetHeight(),
	     memdc, 0, 0, wxCOPY);
    if (!dc->Colour) {
      // Restore original background
      dc->SetBackground(memdc->GetBackground());
    }
    dc->SetUserScale(x_scale, y_scale);
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

CoolToolBar::CoolToolBar(wxFrame *frame, int x, int y, int w, int h,
			 long style, int direction, int RowsOrColumns):
  PlainToolBar(frame, x, y, w, h, style, direction, RowsOrColumns),
  ourframe(frame) {}

void CoolToolBar::SetupBar(ToolBarEntry *tbe, int n)
{
  entries = tbe;
  GetDC()->SetBackground(wxGREY_BRUSH);

  for (int i = 0; i < n; i++) {
    AddTool(i, entries[i].bm, NULL, (entries[i].flags & TOOLBAR_TOGGLE),
	    -1, -1, NULL);
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
