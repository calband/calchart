/* main_ui.h
 * Header for wxWindows interface
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
 *
 */

#ifndef _MAIN_UI_H_
#define _MAIN_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "basic_ui.h"
#include "show.h"

// Value of 10 translates to a canvas of 1760x1000.
#define FIELD_MAXZOOM 10
#define FIELD_DEFAULT_ZOOM 5

enum CC_DRAG_TYPES { CC_DRAG_NONE, CC_DRAG_BOX, CC_DRAG_POLY,
		     CC_DRAG_LASSO, CC_DRAG_LINE };

class FieldCanvas;

class CC_WinNodeMain : public CC_WinNode {
public:
  CC_WinNodeMain(CC_WinList *lst, FieldCanvas *canv);

  virtual void SetShow(CC_show *shw);
  virtual void ChangeName();
  virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
  virtual void UpdatePoints();
  virtual void UpdatePointsOnSheet(unsigned sht);
  virtual void ChangeNumPoints(wxWindow *win);
  virtual void ChangePointLabels(wxWindow *win);
  virtual void ChangeShowMode(wxWindow *win);
  virtual void UpdateStatusBar();
  virtual void GotoSheet(unsigned sht);
  virtual void AddSheet(unsigned sht);
  virtual void DeleteSheet(unsigned sht);
  virtual void AppendSheets();
  virtual void RemoveSheets(unsigned num);
  virtual void ChangeTitle(unsigned sht);
  virtual void SelectSheet(wxWindow* win, unsigned sht);
  virtual void AddContinuity(unsigned sht, unsigned cont);
  virtual void DeleteContinuity(unsigned sht, unsigned cont);
  virtual void FlushContinuity();
  virtual void SetContinuity(wxWindow* win, unsigned sht, unsigned cont);
  virtual void ChangePrint(wxWindow* win);
  virtual void FlushDescr();
  virtual void SetDescr(wxWindow* win);

  CC_WinList winlist;
private:
  FieldCanvas *canvas;
};

// Define a new application
class CalChartApp : public wxApp {
public:
  wxFrame *OnInit(void);
  int OnExit(void);
};

// Define a new frame
class MainFrame : public wxFrameWithStuff {
public:
  MainFrame(wxFrame *frame, int x, int y, int w, int h,
	    MainFrame *other_frame = NULL);
  ~MainFrame();

  Bool OnClose(void);
  void OnMenuCommand(int id);
  void OnMenuSelect(int id);

  Bool OkayToClearShow();

  void LoadShow();
  void AppendShow();
  void SaveShow();
  void SaveShowAs();

  void SnapToGrid(CC_coord& c);

  wxChoice *grid_choice;
  wxSlider *zoom_slider;

  FieldCanvas *field;
  CC_WinNodeMain *node;
};

class FieldCanvas : public AutoScrollCanvas {
public:
  // Basic functions
  FieldCanvas(CC_show *show, unsigned ss, MainFrame *frame,
	      int def_zoom = FIELD_DEFAULT_ZOOM,
	      int x = -1, int y = -1, int w = -1, int h = -1);
  ~FieldCanvas(void);
  void OnPaint(void);
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnScroll(wxCommandEvent& event);

  // Misc show functions
  void RefreshShow(Bool drawall = TRUE, int point = -1);
  void UpdatePanel();
  void UpdateBars();
  inline void UpdateSS() { RefreshShow(); UpdatePanel(); }
  inline void GotoThisSS() {
    UpdateSS();
    ourframe->node->GotoSheet(show_descr.curr_ss);
  }
  inline void GotoSS(unsigned n) {
    show_descr.curr_ss = n; GotoThisSS();
  }
  inline void PrevSS() {
    if (show_descr.curr_ss > 0) {
      show_descr.curr_ss--; GotoThisSS();
    }
  }
  inline void NextSS() {
    if (show_descr.show) {
      if (++show_descr.curr_ss < show_descr.show->GetNumSheets()) {
	GotoThisSS();
      } else --show_descr.curr_ss;
    }
  }
  inline void SetZoomQuick(int factor) {
    zoomf = factor;
    float f = factor * COORD2FLOAT(1);
    SetUserScale(f, f);
  }
  inline void SetZoom(int factor) {
    SetZoomQuick(factor); UpdateBars(); RefreshShow();
  }

  inline void BeginDrag(CC_DRAG_TYPES type, CC_coord start) {
    DrawDrag(FALSE);
    drag = type;
    drag_start = drag_end = start;
    DrawDrag(TRUE);
  };
  inline void MoveDrag(CC_coord end) {
    DrawDrag(FALSE);
    drag_end = end;
    DrawDrag(TRUE);
  };
  inline void EndDrag() {
    DrawDrag(FALSE);
    drag = CC_DRAG_NONE;
  };

  // Variables
  MainFrame *ourframe;
  CC_descr show_descr;
  CC_DRAG_TYPES curr_lasso;

private:
  void DrawDrag(Bool on = TRUE);

  CC_DRAG_TYPES drag;
  CC_coord drag_start, drag_end;
  Bool dragon;
  int zoomf;
};

class SliderWithField: public wxSlider {
 public:
  FieldCanvas *field;
  SliderWithField(wxPanel *parent, wxFunction func, char *label,
	     int value, int min_value, int max_value, int width):
  wxSlider(parent, func, label, value, min_value, max_value, width),
  field(NULL) {};
};

enum {
  CALCHART__NEW = 100,
  CALCHART__NEW_WINDOW,
  CALCHART__LOAD_FILE,
  CALCHART__APPEND_FILE,
  CALCHART__SAVE,
  CALCHART__SAVE_AS,
  CALCHART__PRINT,
  CALCHART__PRINT_EPS,
  CALCHART__CLOSE,
  CALCHART__QUIT,
  CALCHART__UNDO,
  CALCHART__REDO,
  CALCHART__INSERT_BEFORE,
  CALCHART__INSERT_AFTER,
  CALCHART__DELETE,
  CALCHART__RELABEL,
  CALCHART__EDIT_CONTINUITY,
  CALCHART__EDIT_PRINTCONT,
  CALCHART__SET_TITLE,
  CALCHART__SET_BEATS,
  CALCHART__INFO,
  CALCHART__POINTS,
  CALCHART__ANIMATE,
  CALCHART__ABOUT,
  CALCHART__HELP
};

class MainFrameList: public wxList {
public:
  MainFrameList(): wxList() {}
  ~MainFrameList();

  void CloseAllWindows();
};

#endif
