/* show.h
 * Definitions for the show classes
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 * 4-16-95    Garrick Meeker              Converted to C++
 *
 */

#ifndef _SHOW_H_
#define _SHOW_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common.h>  // For basic wx defines
#include <wxstring.h>
#include <wx_list.h>

#include "platconf.h"
#include "linmath.h"

typedef short Coord;

#define COORD_SHIFT 4
#define COORD_DECIMAL (1<<COORD_SHIFT)
#define INT2COORD(a) ((a) * COORD_DECIMAL)
#define COORD2INT(a) ((a) / COORD_DECIMAL)
#define FLOAT2NUM(a) CLIPFLOAT((a) * (1 << COORD_SHIFT))
#define FLOAT2COORD(a) (Coord)FLOAT2NUM((a))
#define COORD2FLOAT(a) ((a) / ((float)(1 << COORD_SHIFT)))
#define CLIPFLOAT(a) (((a) < 0) ? ((a) - 0.5) : ((a) + 0.5))

#define BASICALLY_ZERO_BASICALLY 0.00001
#define IS_ZERO(a) (ABS((a)) < BASICALLY_ZERO_BASICALLY)
#define DEG2RAD(a) ((a) * PI / 180.0)
#define SQRT2 1.4142136

#define MAX_POINTS 1000
#define NUM_REF_PNTS 3

class wxWindow;
class wxDC;
class CC_show;

class CC_WinList;

class CC_WinNode {
public:
  CC_WinNode(CC_WinList *lst);
  virtual ~CC_WinNode();

  void Remove();
  inline CC_WinList* GetList() { return list; }

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
  virtual void GotoContLocation(unsigned sht, unsigned contnum,
				int line = -1, int col = -1);
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

  CC_WinNode *next;
protected:
  CC_WinList *list;
};

class CC_WinList {
public:
  CC_WinList();
  virtual ~CC_WinList();

  Bool MultipleWindows();

  void Add(CC_WinNode *node);
  void Remove(CC_WinNode *node);
  virtual void Empty();

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
  virtual void GotoContLocation(unsigned sht, unsigned contnum,
				int line = -1, int col = -1);
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

private:
  CC_WinNode *list;
};

class CC_WinListShow : public CC_WinList {
public:
  CC_WinListShow(CC_show *shw);
  
  virtual void Empty();
private:
  CC_show *show;
};

enum PSFONT_TYPE {
  PSFONT_SYMBOL, PSFONT_NORM, PSFONT_BOLD, PSFONT_ITAL, PSFONT_BOLDITAL,
  PSFONT_TAB
};

enum SYMBOL_TYPE {
  SYMBOL_PLAIN = 0, SYMBOL_SOL, SYMBOL_BKSL, SYMBOL_SL,
  SYMBOL_X, SYMBOL_SOLBKSL, SYMBOL_SOLSL, SYMBOL_SOLX
};

struct cc_oldcoord {
  unsigned short x;
  unsigned short y;
};

/* Format of old calchart stuntsheet files */
#define OLD_FLAG_FLIP 1
struct cc_oldpoint {
  unsigned char sym;
  unsigned char flags;
  cc_oldcoord pos;
  unsigned short color;
  char code[2];
  unsigned short cont;
  cc_oldcoord ref[3];
};

struct cc_reallyoldpoint {
  unsigned char sym;
  unsigned char flags;
  cc_oldcoord pos;
  unsigned short color;
  char code[2];
  unsigned short cont;
  short refnum;
  cc_oldcoord ref;
};

class CC_textchunk : public wxObject {
public:
  wxString text;
  enum PSFONT_TYPE font;
};

class CC_textline : public wxObject {
public:
  CC_textline();
  ~CC_textline();

  wxList chunks;
  Bool center;
  Bool on_main;
  Bool on_sheet;
};

class CC_text {
public:
  CC_text();
  ~CC_text();

  wxList lines;
};

class CC_continuity {
public:
  CC_continuity();
  ~CC_continuity();
  void SetName(const char* s);
  void SetText(const char* s);
  void AppendText(const char* s);

  CC_continuity *next;
  unsigned num;
  wxString name;
  wxString text;
};

class CC_coord {
public:
  CC_coord(Coord xval = 0, Coord yval = 0) {
    x = xval;
    y = yval;
  }
  CC_coord(const CC_coord& c) { *this = c; }
  CC_coord(const cc_oldcoord& old) { *this = old; }

  float Magnitude() const;
  float DM_Magnitude() const; // check for diagonal military also
  float Direction() const;
  float Direction(const CC_coord& c) const;

  Bool Collides(const CC_coord& c) const;

  CC_coord& operator = (const cc_oldcoord& old);
  inline CC_coord& operator = (const CC_coord& c) {
    x = c.x; y = c.y;
    return *this;
  }
  inline CC_coord& operator += (const CC_coord& c) {
    x += c.x; y += c.y;
    return *this;
  }
  inline CC_coord& operator -= (const CC_coord& c) {
    x -= c.x; y -= c.y;
    return *this;
  }
  inline CC_coord& operator *= (short s) {
    x *= s; y *= s;
    return *this;
  }
  inline CC_coord& operator /= (short s) {
    x /= s; y /= s;
    return *this;
  }

  Coord x, y;
};
inline CC_coord operator + (const CC_coord& a, const CC_coord& b) {
  return CC_coord(a.x + b.x, a.y + b.y);
}
inline CC_coord operator - (const CC_coord& a, const CC_coord& b) {
  return CC_coord(a.x - b.x, a.y - b.y);
}
inline CC_coord operator * (const CC_coord& a, short s) {
  return CC_coord(a.x * s, a.y * s);
}
inline CC_coord operator / (const CC_coord& a, short s) {
  return CC_coord(a.x / s, a.y / s);
}
inline CC_coord operator - (const CC_coord& c) {
  return CC_coord(-c.x, -c.y);
}
inline int operator == (const CC_coord& a, const CC_coord& b) {
  return ((a.x == b.x) && (a.y == b.y));
}
inline int operator == (const CC_coord& a, const short b) {
  return ((a.x == b) && (a.y == b));
}
inline int operator != (const CC_coord& a, const CC_coord& b) {
  return ((a.x != b.x) || (a.y != b.y));
}
inline int operator != (const CC_coord& a, const short b) {
  return ((a.x != b) || (a.y != b));
}

float BoundDirection(float f);

float BoundDirectionSigned(float f);

Bool IsDiagonalDirection(float f);

void CreateVector(CC_coord& c, float dir, float mag);

void CreateUnitVector(float& a, float& b, float dir);

#define PNT_LABEL 1
class CC_point {
public:
  CC_point()
    :flags(0), sym(SYMBOL_PLAIN), cont(0) {}

  inline Bool GetFlip() { return (Bool)(flags & PNT_LABEL); }
  inline void Flip(Bool val = TRUE) {
    if (val) flags |= PNT_LABEL;
    else flags &= ~PNT_LABEL;
  };
  inline void FlipToggle() { Flip(GetFlip() ? FALSE:TRUE); }

  unsigned short flags;
  SYMBOL_TYPE sym;
  unsigned char cont;
  CC_coord pos;
  CC_coord ref[NUM_REF_PNTS];
};

class CC_sheet {
public:
  CC_sheet(CC_show *shw);
  CC_sheet(CC_show *shw, const char *newname);
  CC_sheet(CC_sheet *sht);
  ~CC_sheet();

  void Draw(wxDC *dc, unsigned ref, Bool primary = TRUE,
	    Bool drawall = TRUE, int point = -1);

  // internal use only
  char *PrintStandard(FILE *fp);
  char *PrintSpringshow(FILE *fp);
  char *PrintOverview(FILE *fp);
  char *PrintCont(FILE *fp);

  unsigned GetNumSelectedPoints();
  int FindPoint(Coord x, Coord y, unsigned ref = 0);
  Bool SelectContinuity(unsigned i);
  void SetContinuity(unsigned i);
  void SetNumPoints(unsigned num, unsigned columns);
  void RelabelSheet(unsigned *table);

  CC_continuity *GetNthContinuity(unsigned i);
  CC_continuity *UserGetNthContinuity(unsigned i);
  void SetNthContinuity(const char *text, unsigned i);
  void UserSetNthContinuity(const char *text, unsigned i, wxWindow* win);
  CC_continuity *RemoveNthContinuity(unsigned i);
  void UserDeleteContinuity(unsigned i);
  void InsertContinuity(CC_continuity *newcont, unsigned i);
  void AppendContinuity(CC_continuity *newcont);
  CC_continuity *UserNewContinuity(const char *name);
  unsigned NextUnusedContinuityNum();
  // creates if doesn't exist
  CC_continuity *GetStandardContinuity(SYMBOL_TYPE sym);
  // return 0 if not found else index+1
  unsigned FindContinuityByName(const char *name);
  Bool ContinuityInUse(unsigned idx);

  inline const char *GetName() { return name; }
  inline void SetName(const char *newname) { name = newname; }
  inline const char *GetNumber() { return number; }
  inline void SetNumber(const char *newnumber) { number = newnumber; }
  inline unsigned short GetBeats() { return beats; }
  inline void SetBeats(unsigned short b) { beats = b; }
  inline Bool IsInAnimation() { return (beats != 0); }
  void UserSetName(const char *newname);
  void UserSetBeats(unsigned short b);
  Bool SetPointsSym(SYMBOL_TYPE sym);
  Bool SetPointsLabel(Bool right);
  Bool SetPointsLabelFlip();

  inline CC_point& GetPoint(unsigned i) { return pts[i]; }
  inline void SetPoint(const cc_oldpoint& val, unsigned i);

  const CC_coord& GetPosition(unsigned i, unsigned ref = 0) const;
  void SetAllPositions(const CC_coord& val, unsigned i);
  void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);
  void SetPositionQuick(const CC_coord& val, unsigned i, unsigned ref = 0);
  Bool ClearRefPositions(unsigned ref);
  Bool TranslatePoints(const CC_coord& delta, unsigned ref = 0);
  Bool TransformPoints(const Matrix& transmat, unsigned ref = 0);
  Bool MovePointsInLine(const CC_coord& start, const CC_coord& second,
			unsigned ref);

  CC_sheet *next;
  CC_text continuity;
  CC_continuity *animcont;
  CC_show *show;
  unsigned numanimcont;
  Bool picked; /* for requestors like printing */
private:
  unsigned short beats;
  CC_point *pts;
  wxString name;
  wxString number;
};

#define DEF_HASH_W 32
#define DEF_HASH_E 52

class ShowUndoList;
class StuntSheetPicker;
class ShowInfoReq;
class ShowMode;

class CC_show {
public:
  CC_show(unsigned npoints = 0);
  CC_show(const char *file);
  ~CC_show();

  wxString* ImportContinuity(const wxString& file);

  int Print(FILE *fp, Bool eps = FALSE, Bool overview = FALSE,
	    unsigned curr_ss = 0, int min_yards = 50);

  inline const char *GetError() { return error; }
  inline Bool Ok() { return okay; }

  void Append(CC_show *shw);
  void Append(CC_sheet *newsheets);
  char *Save(const char *filename);
  char *Autosave();
  void ClearAutosave();

  inline void FlushAllTextWindows() {
    winlist->FlushDescr(); winlist->FlushContinuity();
  }

  inline const char *GetName() { return name; }
  const char *UserGetName();
  void SetName(const char *newname);
  inline void UserSetName(const char *newname) {
    SetName(newname); winlist->ChangeName();
  }

  inline const char *GetDescr() { return descr; }
  inline const char *UserGetDescr() { winlist->FlushDescr(); return descr; }
  inline void SetDescr(const char *newdescr) { descr = newdescr; }
  void UserSetDescr(const char *newdescr, wxWindow* win);

  inline Bool Modified() { return modified; }
  inline void SetModified(Bool b) { modified = b; winlist->UpdateStatusBar(); }

  inline unsigned short GetNumSheets() { return numsheets; }
  inline CC_sheet *GetSheet() { return sheets; }
  CC_sheet *GetNthSheet(unsigned n);
  unsigned GetSheetPos(CC_sheet *sheet);
  CC_sheet *RemoveNthSheet(unsigned sheetidx);
  CC_sheet *RemoveLastSheets(unsigned numtoremain);
  void DeleteNthSheet(unsigned sheetidx);
  void UserDeleteSheet(unsigned sheetidx);
  void InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx);
  void InsertSheet(CC_sheet *nsheet, unsigned sheetidx);
  void UserInsertSheet(CC_sheet *sht, unsigned sheetidx);
  inline unsigned short GetNumPoints() { return numpoints; }
  void SetNumPoints(unsigned num, unsigned columns);
  void SetNumPointsInternal(unsigned num); //Only for creating show class
  Bool RelabelSheets(unsigned sht);

  inline char *GetPointLabel(unsigned i) { return pt_labels[i]; }
  inline Bool& GetBoolLandscape() { return print_landscape; }
  inline Bool& GetBoolDoCont() { return print_do_cont; }
  inline Bool& GetBoolDoContSheet() { return print_do_cont_sheet; }

  Bool UnselectAll();
  inline Bool IsSelected(unsigned i) { return selections[i]; }
  void Select(unsigned i, Bool val = TRUE);
  inline void SelectToggle(unsigned i) {
    selections[i] = selections[i] ? FALSE:TRUE;
  }
  inline wxList& GetSelectionList() { return selectionList; }

  CC_WinListShow *winlist;
  ShowUndoList *undolist;
  ShowMode *mode;

private:
  void PrintSheets(FILE *fp); // called by Print()
  char *SaveInternal(const char *filename);
  void SetAutosaveName(const char *realname);

  void AddError(const wxString& str) {
    error += str + '\n';
    okay = FALSE;
  }

  wxString error;
  Bool okay;

  wxString name;
  wxString autosave_name;
  wxString descr;
  unsigned short numpoints;
  unsigned short numsheets;
  CC_sheet *sheets;
  char (*pt_labels)[4];
  Bool *selections; // array for each point
  wxList selectionList; // order of selections
  Bool modified;
  Bool print_landscape;
  Bool print_do_cont;
  Bool print_do_cont_sheet;
};

class CC_descr {
public:
  CC_show *show;
  unsigned curr_ss;
  inline CC_sheet *CurrSheet() { return show->GetNthSheet(curr_ss); }
};

void SetAutoSave(int secs);

#endif
