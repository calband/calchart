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

enum CC_DRAG_TYPES { CC_DRAG_NONE, CC_DRAG_BOX, CC_DRAG_POLY,
		     CC_DRAG_LASSO, CC_DRAG_LINE, CC_DRAG_CROSS };
enum CC_MOVE_MODES { CC_MOVE_NORMAL, CC_MOVE_LINE, CC_MOVE_ROTATE,
		     CC_MOVE_SHEAR, CC_MOVE_REFL, CC_MOVE_SIZE,
		     CC_MOVE_GENIUS};

enum {
  CALCHART__NEW = 100,
  CALCHART__NEW_WINDOW,
  CALCHART__LOAD_FILE,
  CALCHART__APPEND_FILE,
  CALCHART__IMPORT_CONT_FILE,
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
  CALCHART__CLEAR_REF,
  CALCHART__EDIT_CONTINUITY,
  CALCHART__EDIT_PRINTCONT,
  CALCHART__SET_TITLE,
  CALCHART__SET_BEATS,
  CALCHART__SETUP,
  CALCHART__POINTS,
  CALCHART__ANIMATE,
  CALCHART__SELECTION,
  CALCHART__ROWS,
  CALCHART__COLUMNS,
  CALCHART__NEAREST,
  CALCHART__ABOUT,
  CALCHART__HELP
};
enum CC_SELECT_TYPES {
  CC_SELECT_ROWS = CALCHART__ROWS,
  CC_SELECT_COLUMNS = CALCHART__COLUMNS,
  CC_SELECT_NEAREST = CALCHART__NEAREST,
};

class MainFrame;

class CC_shape {
public:
  CC_shape();
  virtual ~CC_shape();
  
  virtual void Draw(wxDC *dc, float x, float y) const = 0;
  virtual void OnMove(const CC_coord& p, MainFrame *frame) = 0;
};

class CC_shape_1point: public CC_shape {
public:
  CC_shape_1point(const CC_coord& p);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  void MoveOrigin(const CC_coord& p);
  CC_coord GetOrigin() const;

protected:
  wxPoint origin;
};

class CC_shape_cross: public CC_shape_1point {
public:
  CC_shape_cross(const CC_coord& p, Coord width);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  virtual void Draw(wxDC *dc, float x, float y) const;

protected:
  Coord cross_width;
};

class CC_shape_2point: public CC_shape_1point {
public:
  CC_shape_2point(const CC_coord& p);
  CC_shape_2point(const CC_coord& p1, const CC_coord& p2);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  void MovePoint(const CC_coord& p);
  CC_coord GetPoint() const;

protected:
  wxPoint point;
};

class CC_shape_line: public CC_shape_2point {
public:
  CC_shape_line(const CC_coord& p);
  CC_shape_line(const CC_coord& p1, const CC_coord& p2);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  virtual void Draw(wxDC *dc, float x, float y) const;
};

class CC_shape_angline: public CC_shape_line {
public:
  CC_shape_angline(const CC_coord& p, const CC_coord& refvect);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
private:
  CC_coord vect;
  float mag;
};

class CC_shape_arc: public CC_shape_2point {
public:
  CC_shape_arc(const CC_coord& c, const CC_coord& p);
  CC_shape_arc(const CC_coord& c, const CC_coord& p1, const CC_coord& p2);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  virtual void Draw(wxDC *dc, float x, float y) const;

  inline float GetAngle() const { return r-r0; }
private:
  float r,r0,d;
};

class CC_shape_rect: public CC_shape_2point {
public:
  CC_shape_rect(const CC_coord& p);
  CC_shape_rect(const CC_coord& p1, const CC_coord& p2);

  virtual void Draw(wxDC *dc, float x, float y) const;
};

class CC_lasso: public CC_shape {
public:
  CC_lasso(const CC_coord& p);
  virtual ~CC_lasso();

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
  void Clear();
  void Start(const CC_coord& p);
  void End();
  void Append(const CC_coord& p);
  Bool Inside(const CC_coord& p) const;
  virtual void Draw(wxDC *dc, float x, float y) const;
  void Drag(const CC_coord& p);
  inline const wxPoint *FirstPoint() const {
    wxNode *n = ((wxList*)&pntlist)->Last();
    if (n != NULL)
      return (wxPoint*)n->Data();
    else return NULL;
  }
private:
  Bool CrossesLine(const wxPoint* start, const wxPoint* end,
		   const CC_coord& p) const;
  wxList pntlist;
};

class CC_poly: public CC_lasso {
public:
  CC_poly(const CC_coord& p);

  virtual void OnMove(const CC_coord& p, MainFrame *frame);
};

class CC_WinNodeMain : public CC_WinNode {
public:
  CC_WinNodeMain(CC_WinList *lst, MainFrame *frm);

  virtual void SetShow(CC_show *shw);
  virtual void ChangeName();
  virtual void UpdateSelections(wxWindow* win = NULL, int point = -1);
  virtual void UpdatePoints();
  virtual void UpdatePointsOnSheet(unsigned sht, int ref = -1);
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
  MainFrame *frame;
};

// Define a new application
class CalChartApp : public wxApp {
public:
  wxFrame *OnInit(void);
  int OnExit(void);
};

// Top-level frame
class TopFrame : public wxFrame {
public:
  TopFrame(int width, int height);
  ~TopFrame();
  Bool OnClose(void);
#ifdef CC_USE_MDI
  void OnMenuCommand(int id);
  void OnMenuSelect(int id);
#endif
  void OnDropFiles(int n, char *files[], int x, int y);
  void NewShow(CC_show *shw = NULL);
  void OpenShow(const char *filename = NULL);
  void Quit();
  void About();
  void Help();
};

class FieldCanvas;
// Define the main editing frame
class MainFrame : public wxFrameWithStuff {
public:
  MainFrame(wxFrame *frame, int x, int y, int w, int h,
	    CC_show *show = NULL, MainFrame *other_frame = NULL);
  ~MainFrame();

  Bool OnClose(void);
  void OnMenuCommand(int id);
  void OnMenuSelect(int id);

  Bool OkayToClearShow();

  void LoadShow();
  void AppendShow();
  void ImportContFile();
  void SaveShow();
  void SaveShowAs();

  void SnapToGrid(CC_coord& c);
  void UpdatePanel();

  void SetCurrentLasso(CC_DRAG_TYPES type);
  void SetCurrentMove(CC_MOVE_MODES type);

  void Setup();

  wxChoice *grid_choice;
  wxChoice *ref_choice;
  wxSlider *zoom_slider;
  wxSlider *sheet_slider;

  FieldCanvas *field;
  CC_WinNodeMain *node;
};

class FieldCanvas : public AutoScrollCanvas {
public:
  // Basic functions
  FieldCanvas(CC_show *show, unsigned ss, MainFrame *frame,
	      int def_zoom,
	      FieldCanvas *from_canvas = NULL,
	      int x = -1, int y = -1, int w = -1, int h = -1);
  ~FieldCanvas(void);
  void OnPaint(void);
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnScroll(wxCommandEvent& event);

  // Misc show functions
  void RefreshShow(Bool drawall = TRUE, int point = -1);
  void UpdateBars();
  inline void UpdateSS() { RefreshShow(); ourframe->UpdatePanel(); }
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
  inline Bool SetZoomQuick(int factor) {
    if (factor != zoomf) {
      zoomf = factor;
      float f = factor * COORD2FLOAT(1);
      SetUserScale(f, f);
      return TRUE;
    }
    return FALSE;
  }
  inline void SetZoom(int factor) {
    if (SetZoomQuick(factor)) {
      UpdateBars(); RefreshShow();
    }
  }

  void BeginDrag(CC_DRAG_TYPES type, CC_coord start);
  void BeginDrag(CC_DRAG_TYPES type, CC_shape *shape);
  void AddDrag(CC_DRAG_TYPES type, CC_shape *shape);
  void MoveDrag(CC_coord end);
  void EndDrag();

  // Variables
  MainFrame *ourframe;
  CC_descr show_descr;
  CC_DRAG_TYPES curr_lasso;
  CC_MOVE_MODES curr_move;
  CC_SELECT_TYPES curr_select;
  unsigned curr_ref;

private:
  void ClearShapes();
  void DrawDrag(Bool on = TRUE);
  void SelectOrdered(wxList& pointlist, const CC_coord& start);
  Bool SelectWithLasso(const CC_lasso *lasso);
  Bool SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
			  unsigned ref = 0);

  CC_DRAG_TYPES drag;
  wxList shape_list;
  CC_shape *curr_shape;
  Bool dragon;
  int zoomf;
};

class ChoiceWithField: public wxChoice {
public:
  FieldCanvas *field;
  ChoiceWithField(wxPanel *panel, wxFunction func, char *Title,
		  int x = -1, int y = -1, int width = -1, int height = -1,
		  int N = 0, char **Choices = NULL,
		  long style = 0, char *name = "choice"):
    wxChoice(panel, func, Title, x, y, width, height, N, Choices, style, name),
    field(NULL) {};
};

class SliderWithField: public wxSlider {
public:
  FieldCanvas *field;
  SliderWithField(wxPanel *parent, wxFunction func, char *label,
	     int value, int min_value, int max_value, int width):
  wxSlider(parent, func, label, value, min_value, max_value, width),
  field(NULL) {};
};

class MainFrameList: public wxList {
public:
  Bool CloseAllWindows();
};

#endif
