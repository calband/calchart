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

// Function for allowing XOR drawing
void SetXOR(wxDC *dc);

// Set icon to band's insignia
void SetBandIcon(wxFrame *frame);

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextWindow {
public:
  FancyTextWin(wxFrame *frame, int x=-1, int y=-1, int width=-1, int height=-1,
	       long style=wxNATIVE_IMPL|wxHSCROLL);
#ifdef TEXT_DOS_STYLE
  char *GetContents(void);
#endif
  void OnDropFiles(int n, char *files[], int x, int y);
};

enum FRAMESTUFF_LAYOUT {
  wxFRAMESTUFF_TB_PNL,
  wxFRAMESTUFF_TB_CNV,
  wxFRAMESTUFF_PNL_TB,
  wxFRAMESTUFF_PNL_CNV,
  wxFRAMESTUFF_CNV_TB,
  wxFRAMESTUFF_CNV_PNL
};

// Define a list box that fixes Motif bug
class GoodListBox : public wxListBox {
public:
  GoodListBox(wxPanel *panel, wxFunction func, char *Title,
	      Bool Multiple = wxSINGLE|wxNEEDED_SB,
	      int x = -1, int y = -1, int width = -1, int height = -1,
	      int N = 0, char **Choices = NULL,
	      long style = 0, char *name = "listBox");
  void SetSelection(int N, Bool select = TRUE);
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
  inline void SetToolBar(PlainToolBar *tb) { frameToolBar = tb; }
  inline void SetPanel(wxPanel *pnl) { framePanel = pnl; }
  inline void SetCanvas(wxCanvas *cnv) { frameCanvas = cnv; }
  inline PlainToolBar *GetFrameToolBar(void) { return frameToolBar; }
  inline wxPanel *GetFramePanel(void) { return framePanel; }
  inline wxCanvas *GetFrameCanvas(void) { return frameCanvas; }
  inline void SetLayoutMethod(FRAMESTUFF_LAYOUT lo) { layout = lo; }
  inline FRAMESTUFF_LAYOUT GetLayoutMethod(void) { return layout; }
};

// Define a frame with a toolbar and a panel and fixed size canvas
class wxFrameWithStuffSized: public wxFrameWithStuff
{
public:
  wxFrameWithStuffSized(wxFrame *frame, char *title,
			int x = -1, int y = -1,
			long style = wxSDI | wxDEFAULT_FRAME);
  void OnSize(int w, int h);  // Default OnSize handler
  void Fit();  // Shrinks frame to fit around a given canvas size
};

class AutoScrollCanvas: public wxCanvas
{
public:
  AutoScrollCanvas(wxWindow *parent, int x = -1, int y = -1,
		   int w = -1, int h = -1);
  ~AutoScrollCanvas();

  inline wxDC *GetMemDC() { return memdc; }
  void SetSize(int width, int height);
  void SetBackground(wxBrush *brush);
  void SetColourMap(wxColourMap *colourMap);
  void SetUserScale(float x, float y);
  inline void SetPosition(float x, float y) {
    x_off = x; y_off = y;
  }
  inline float GetPositionX() { return x_off/x_scale; }
  inline float GetPositionY() { return y_off/y_scale; }

  void Move(float x, float y);
  void Blit();

private:
  void FreeMem();

  wxMemoryDC *memdc;
  wxBitmap *membm;
  float x_off, y_off;
  float x_scale, y_scale;
  wxColourMap *cmap;
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

#define TOOLBAR_SPACE 1
#define TOOLBAR_TOGGLE 2

struct ToolBarEntry {
  unsigned flags;
  wxBitmap *bm;
  char *desc;
  void (*func)(CoolToolBar *);
};
#endif
