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

#include "platconf.h"

typedef short Coord;

#define COORD_SHIFT 4
#define COORD_DECIMAL (1<<COORD_SHIFT)
#define INT2COORD(a) ((a) * COORD_DECIMAL)
#define COORD2INT(a) ((a) / COORD_DECIMAL)
#define FLOAT2COORD(a) (Coord)((a) * (1 << COORD_SHIFT))
#define COORD2FLOAT(a) ((a) / ((float)(1 << COORD_SHIFT)))

#define DEFAULT_GRID INT2COORD(2)

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
  virtual void UpdateSelections();
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
  virtual void ChangeTitle(unsigned sht);
  virtual void SelectSheet(wxWindow* win, unsigned sht);
  virtual void AddContinuity(unsigned sht, unsigned cont);
  virtual void DeleteContinuity(unsigned sht, unsigned cont);
  virtual void ChangePrint(wxWindow* win);

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
  virtual void UpdateSelections();
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
  virtual void ChangeTitle(unsigned sht);
  virtual void SelectSheet(wxWindow* win, unsigned sht);
  virtual void AddContinuity(unsigned sht, unsigned cont);
  virtual void DeleteContinuity(unsigned sht, unsigned cont);
  virtual void ChangePrint(wxWindow* win);

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

struct cc_text {
  cc_text *next;
  cc_text *more;
  wxString text;
  enum PSFONT_TYPE font;
  Bool center;
  Bool on_main;
  Bool on_sheet;
};

class CC_continuity {
public:
  CC_continuity();
  ~CC_continuity();
  void SetName(const char* s);
  void SetText(const char* s);
  void AppendText(const char* s);

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

  float Magnitude();
  float DM_Magnitude(); // check for diagonal military also
  float Direction(const CC_coord& c);

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
inline int operator != (const CC_coord& a, const CC_coord& b) {
  return ((a.x != b.x) || (a.y != b.y));
}

#define PNT_LABEL 1
class CC_point {
public:
  CC_point()
    :flags(0), sym(0), cont(0) {}
  CC_point(const CC_coord p)
    :flags(0), sym(0), cont(0), pos(p) {
      for (unsigned i = 0; i < NUM_REF_PNTS; i++) {
	ref[i] = p;
      }
    }
  CC_point(const cc_oldpoint old) { *this = old; }

  CC_point& operator = (const cc_oldpoint& old);

  inline Bool GetFlip() { return (Bool)(flags & PNT_LABEL); }
  inline void Flip(Bool val = TRUE) {
    if (val) flags |= PNT_LABEL;
    else flags &= ~PNT_LABEL;
  };
  inline void FlipToggle() { Flip(GetFlip() ? FALSE:TRUE); }

  unsigned short flags;
  unsigned char sym, cont;
  CC_coord pos;
  CC_coord ref[NUM_REF_PNTS];
};

class CC_sheet {
public:
  CC_sheet(CC_show *shw);
  CC_sheet(CC_show *shw, const char *newname);
  CC_sheet(CC_sheet *sht);
  ~CC_sheet();

  void Draw(wxDC *dc);

  // internal use only
  char *PrintStandard(FILE *fp);
  char *PrintSpringshow(FILE *fp);
  char *PrintOverview(FILE *fp);
  char *PrintCont(FILE *fp);

  unsigned GetNumSelectedPoints();
  int FindPoint(Coord x, Coord y);
  Bool SelectPointsInRect(CC_coord c1, CC_coord c2);
  Bool SelectContinuity(unsigned i);
  void SetContinuity(unsigned i);
  void SetNumPoints(unsigned num, unsigned columns);

  inline const char *GetName() { return name; }
  inline void SetName(const char *newname) { name = newname; }
  inline const char *GetNumber() { return number; }
  inline void SetNumber(const char *newnumber) { number = newnumber; }
  void UserSetName(const char *newname);
  void UserSetBeats(unsigned short b);
  Bool SetPointsSym(unsigned char sym);
  Bool SetPointsLabel(Bool right);
  Bool SetPointsLabelFlip();

  Bool TranslatePoints(CC_coord delta);

  CC_sheet *next;
  CC_point *pts;
  cc_text *continuity;
  CC_continuity *animcont;
  CC_show *show;
  unsigned numanimcont;
  unsigned short beats;
  Bool picked; /* for requestors like printing */
private:
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

  int Print(FILE *fp, Bool eps = FALSE, Bool overview = FALSE,
	    unsigned curr_ss = 0);

  inline char *GetError() { return error; }
  inline Bool Ok() { return (error == NULL); }

  char *Save(const char *filename);

  inline const char *GetName() { return name; }
  const char *UserGetName();
  inline void SetName(const char *newname) { name = newname; }
  inline void UserSetName(const char *newname) {
    name = newname; winlist->ChangeName();
  }

  inline Bool Modified() { return modified; }
  inline void SetModified(Bool b) { modified = b; winlist->UpdateStatusBar(); }

  inline unsigned short GetNumSheets() { return numsheets; }
  inline CC_sheet *GetSheet() { return sheets; }
  CC_sheet *GetNthSheet(unsigned n);
  unsigned GetSheetPos(CC_sheet *sheet);
  CC_sheet *RemoveNthSheet(unsigned sheetidx);
  void DeleteNthSheet(unsigned sheetidx);
  void UserDeleteSheet(unsigned sheetidx);
  void InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx);
  void InsertSheet(CC_sheet *nsheet, unsigned sheetidx);
  void UserInsertSheet(CC_sheet *sht, unsigned sheetidx);
  inline unsigned short GetNumPoints() { return numpoints; }
  void SetNumPoints(unsigned num, unsigned columns);

  void SetNumPointsInternal(unsigned num); //Only for creating show class

  inline char *GetPointLabel(unsigned i) { return pt_labels[i]; }
  inline Bool& GetBoolLandscape() { return print_landscape; }
  inline Bool& GetBoolDoCont() { return print_do_cont; }
  inline Bool& GetBoolDoContSheet() { return print_do_cont_sheet; }

  inline void SetGrid(Coord c) {
    gridadjust = c >> 1; // Half of grid value
    gridmask = ~(c-1); // Create mask to snap to this coord
  }
  inline Coord SnapX(Coord c) { return ((c+gridadjust) & gridmask); }
  inline Coord SnapY(Coord c) {
    // Adjust so 4 step grid will be on visible grid
    return ((c + gridadjust - INT2COORD(2)) & gridmask) + INT2COORD(2);
  }

  Bool UnselectAll();
  inline Bool IsSelected(unsigned i) { return selections[i]; }
  inline void Select(unsigned i, Bool val = TRUE) { selections[i] = val; }
  inline void SelectToggle(unsigned i) {
    selections[i] = selections[i] ? FALSE:TRUE;
  }

  CC_WinListShow *winlist;
  ShowUndoList *undolist;
  ShowMode *mode;
  Bool *selections; // array for each point

private:
  void PrintSheets(FILE *fp); // called by Print()

  char *error;
  wxString name;
  unsigned short numpoints;
  unsigned short numsheets;
  CC_sheet *sheets;
  char (*pt_labels)[4];
  Bool modified;
  Bool print_landscape;
  Bool print_do_cont;
  Bool print_do_cont_sheet;
  Coord gridmask;
  Coord gridadjust;
};

class CC_descr {
public:
  CC_show *show;
  unsigned curr_ss;
  inline CC_sheet *CurrSheet() { return show->GetNthSheet(curr_ss); }
};

#endif
