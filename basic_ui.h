/* basic_ui.h
 * Header for basic wxWindows classes
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
 *
 */

#ifndef _BASIC_UI_H_
#define _BASIC_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <wx.h>

#include "platconf.h"

#ifdef CC_USE_BTNBAR
#include <wx_bbar.h>
#else
#include <wx_tbar.h>
#endif

#ifdef CC_USE_BTNBAR
#define PlainToolBar wxButtonBar
#else
#define PlainToolBar wxToolBar
#endif
#define TB_MARGIN 5

// Function for allowing XOR drawing
void SetXOR(wxDC *dc);

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextWindow {
public:
  FancyTextWin(wxFrame *frame, int x=-1, int y=-1,
	       int width=-1, int height=-1, long style=0);
  void OnDropFiles(int n, char *files[], int x, int y);
};

// Define a frame with a toolbar and one subwindow
class wxFrameWithToolBar: public wxFrame
{
public:
  PlainToolBar *frameToolBar;
  wxFrameWithToolBar(wxFrame *frame, char *title, int x, int y, int w, int h,
		     long style = wxSDI | wxDEFAULT_FRAME);
  void OnSize(int w, int h);  // Default OnSize handler
  Bool OnClose(void);
  inline void SetToolBar(PlainToolBar *tb) { frameToolBar = tb; }
  inline PlainToolBar *GetFrameToolBar(void) { return frameToolBar; }
};

enum FRAMESTUFF_LAYOUT {
  wxFRAMESTUFF_TB_PNL,
  wxFRAMESTUFF_TB_CNV,
  wxFRAMESTUFF_PNL_TB,
  wxFRAMESTUFF_PNL_CNV,
  wxFRAMESTUFF_CNV_TB,
  wxFRAMESTUFF_CNV_PNL
};

// Define a frame with a toolbar and a panel and canvas
class wxFrameWithStuff: public wxFrame
{
public:
  PlainToolBar *frameToolBar;
  wxPanel *framePanel;
  wxCanvas *frameCanvas;
  FRAMESTUFF_LAYOUT layout;

  wxFrameWithStuff(wxFrame *frame, char *title,
		   int x = -1, int y = -1, int w = -1, int h = -1,
		   long style = wxSDI | wxDEFAULT_FRAME);
  void OnSize(int w, int h);  // Default OnSize handler
  Bool OnClose(void);
  void Fit();  // Shrinks frame to fit around a given canvas size
  inline void SetToolBar(PlainToolBar *tb) { frameToolBar = tb; }
  inline void SetPanel(wxPanel *pnl) { framePanel = pnl; }
  inline void SetCanvas(wxCanvas *cnv) { frameCanvas = cnv; }
  inline PlainToolBar *GetFrameToolBar(void) { return frameToolBar; }
  inline wxPanel *GetFramePanel(void) { return framePanel; }
  inline wxCanvas *GetFrameCanvas(void) { return frameCanvas; }
  inline void SetLayoutMethod(FRAMESTUFF_LAYOUT lo) { layout = lo; }
  inline FRAMESTUFF_LAYOUT GetLayoutMethod(void) { return layout; }
};

struct ToolBarEntry;
class CoolToolBar: public PlainToolBar
{
public:
  CoolToolBar(wxFrame *frame, int x = 0, int y = 0, int w = -1, int h = -1,
	      long style = 0, int direction = wxVERTICAL,
	      int RowsOrColumns = 2);
  void SetupBar(ToolBarEntry *tbe, int n);

  Bool OnLeftClick(int toolIndex, Bool toggled);
  void OnMouseEnter(int toolIndex);

  wxFrame *ourframe;
  ToolBarEntry *entries;
};

struct ToolBarEntry {
  wxBitmap *bm;
  char *desc;
  void (*func)(CoolToolBar *);
};
#endif
