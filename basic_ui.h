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

#include <wx/wx.h>

#include "platconf.h"

#include <wx/toolbar.h>

#define CC_USE_MDI
#ifdef CC_USE_MDI
#define CC_FRAME_TOP (wxMDI_PARENT | wxMAXIMIZE | wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_CHILD (wxMDI_CHILD | wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_OTHER (wxDEFAULT_FRAME_STYLE)
#else
#define CC_FRAME_TOP (wxSDI | wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_CHILD (wxSDI | wxDEFAULT_FRAME_STYLE)
#define CC_FRAME_OTHER (wxSDI | wxDEFAULT_FRAME_STYLE)
#endif

// Function for allowing XOR drawing
void SetXOR(wxDC *dc);

// Set icon to band's insignia
#ifdef __WXMSW__
inline void SetBandIcon(wxFrame *) {}
#else
void SetBandIcon(wxFrame *frame);
#endif

// Define a text subwindow that can respond to drag-and-drop
class FancyTextWin : public wxTextCtrl {
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
  //  void OnDropFiles(int n, char *files[], int x, int y);
};

// Define a frame with a toolbar and a panel and canvas
class wxFrameWithStuff: public wxFrame
{
public:
  wxToolBar *frameToolBar;
  wxPanel *framePanel;
  wxWindow *frameCanvas;

  wxFrameWithStuff(wxWindow *parent, wxWindowID id, const wxString& title,
		   const wxPoint& pos = wxDefaultPosition,
		   const wxSize& size = wxDefaultSize,
		   long style = CC_FRAME_OTHER,
		   const wxString& name = wxFrameNameStr);
  inline void SetToolBar(wxToolBar *tb) { frameToolBar = tb; }
  inline void SetPanel(wxPanel *pnl) { framePanel = pnl; }
  inline void SetCanvas(wxWindow *cnv) { frameCanvas = cnv; }
  inline wxToolBar *GetFrameToolBar(void) { return frameToolBar; }
  inline wxPanel *GetFramePanel(void) { return framePanel; }
  inline wxWindow *GetFrameCanvas(void) { return frameCanvas; }
  void DoLayout();
};

// Define a frame with a toolbar and a panel and fixed size canvas
class wxFrameWithStuffSized: public wxFrameWithStuff
{
public:
  wxFrameWithStuffSized(wxWindow *parent, wxWindowID id, const wxString& title,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = CC_FRAME_OTHER,
			const wxString& name = wxFrameNameStr);
  void Fit();  // Shrinks frame to fit around a given canvas size
};

class AutoScrollCanvas: public wxWindow
{
public:
  AutoScrollCanvas(wxWindow *parent, wxWindowID id,
		   const wxPoint& pos = wxDefaultPosition,
		   const wxSize& size = wxDefaultSize,
		   long style = 0,
		   const wxString& name = "canvas");
  ~AutoScrollCanvas();

  inline wxDC *GetMemDC() { return memdc; }
  void SetSize(const wxSize& size);
  void SetBackground(wxBrush *brush);
  void SetColourMap(wxColourMap *colourMap);
  void SetUserScale(float x, float y);
  inline void SetPosition(float x, float y) {
    x_off = x; y_off = y;
  }
  inline float GetPositionX() const { return x_off/x_scale; }
  inline float GetPositionY() const { return y_off/y_scale; }

  void Move(float x, float y, bool noscroll=0);
  void Blit();

private:
  void FreeMem();

  wxMemoryDC *memdc;
  wxBitmap *membm;
  float x_off, y_off;
  float x_scale, y_scale;
  wxColourMap *cmap;
  wxPoint last_pos;
};

struct ToolBarEntry;
class CoolToolBar: public wxToolBar
{
public:
  CoolToolBar(wxFrame *frame, wxWindowID id,
	      const wxPoint& pos = wxDefaultPosition,
	      const wxSize& size = wxDefaultSize,
	      long style = wxTB_HORIZONTAL | wxNO_BORDER,
	      const wxString& name = wxPanelNameStr);
  void SetupBar(ToolBarEntry *tbe, int n);

  bool OnLeftClick(int toolIndex, bool toggled);
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
