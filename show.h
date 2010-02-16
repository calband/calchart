/* show.h
 * Definitions for the show classes
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 * 4-16-95    Garrick Meeker              Converted to C++
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#ifndef _SHOW_H_
#define _SHOW_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wx/defs.h>  // For basic wx defines
#include <wx/string.h>
#include <wx/list.h>

#include "platconf.h"
#include "linmath.h"

#include <vector>

typedef int16_t Coord;

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

  bool MultipleWindows();

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
  uint16_t x;
  uint16_t y;
};

/* Format of old calchart stuntsheet files */
#define OLD_FLAG_FLIP 1
struct cc_oldpoint {
  uint8_t sym;
  uint8_t flags;
  cc_oldcoord pos;
  uint16_t color;
  int8_t code[2];
  uint16_t cont;
  cc_oldcoord ref[3];
};

struct cc_reallyoldpoint {
  uint8_t sym;
  uint8_t flags;
  cc_oldcoord pos;
  uint16_t color;
  int8_t code[2];
  uint16_t cont;
  int16_t refnum;
  cc_oldcoord ref;
};

class CC_textchunk {
public:
  wxString text;
  enum PSFONT_TYPE font;
};

typedef std::vector<CC_textchunk> CC_textchunk_list;

class CC_textline {
public:
  CC_textline();
  ~CC_textline();

  CC_textchunk_list chunks;
  bool center;
  bool on_main;
  bool on_sheet;
};

typedef std::vector<CC_textline> CC_textline_list;

class CC_text {
public:
  CC_text();
  ~CC_text();

  CC_textline_list lines;
};

class CC_continuity {
public:
  CC_continuity();
  ~CC_continuity();
  void SetName(const wxChar* s);
  void SetText(const wxChar* s);
  void AppendText(const wxChar* s);

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

  bool Collides(const CC_coord& c) const;

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

bool IsDiagonalDirection(float f);

void CreateVector(CC_coord& c, float dir, float mag);

void CreateUnitVector(float& a, float& b, float dir);

#define PNT_LABEL 1
class CC_point {
public:
  CC_point()
    :flags(0), sym(SYMBOL_PLAIN), cont(0) {}

  inline bool GetFlip() const { return (bool)(flags & PNT_LABEL); }
  inline void Flip(bool val = true) {
    if (val) flags |= PNT_LABEL;
    else flags &= ~PNT_LABEL;
  };
  inline void FlipToggle() { Flip(GetFlip() ? false:true); }

  unsigned short flags;
  SYMBOL_TYPE sym;
  unsigned char cont;
  CC_coord pos;
  CC_coord ref[NUM_REF_PNTS];
};

class CC_sheet {
public:
  CC_sheet(CC_show *shw);
  CC_sheet(CC_show *shw, const wxString& newname);
  CC_sheet(CC_sheet *sht);
  ~CC_sheet();

  void Draw(wxDC *dc, unsigned ref, bool primary = true,
	    bool drawall = true, int point = -1);

  // internal use only
  const wxChar *PrintStandard(FILE *fp) const;
  const wxChar *PrintSpringshow(FILE *fp) const;
  const wxChar *PrintOverview(FILE *fp) const;
  const wxChar *PrintCont(FILE *fp) const;

  unsigned GetNumSelectedPoints();
  int FindPoint(Coord x, Coord y, unsigned ref = 0);
  bool SelectContinuity(unsigned i);
  void SetContinuity(unsigned i);
  void SetNumPoints(unsigned num, unsigned columns);
  void RelabelSheet(unsigned *table);

  CC_continuity *GetNthContinuity(unsigned i);
  CC_continuity *UserGetNthContinuity(unsigned i);
  void SetNthContinuity(const wxString& text, unsigned i);
  void UserSetNthContinuity(const wxString& text, unsigned i, wxWindow* win);
  CC_continuity *RemoveNthContinuity(unsigned i);
  void UserDeleteContinuity(unsigned i);
  void InsertContinuity(CC_continuity *newcont, unsigned i);
  void AppendContinuity(CC_continuity *newcont);
  CC_continuity *UserNewContinuity(const wxString& name);
  unsigned NextUnusedContinuityNum();
  // creates if doesn't exist
  CC_continuity *GetStandardContinuity(SYMBOL_TYPE sym);
  // return 0 if not found else index+1
  unsigned FindContinuityByName(const wxString& name);
  bool ContinuityInUse(unsigned idx);

  inline const wxString& GetName() const { return name; }
  inline void SetName(const wxString& newname) { name = newname; }
  inline const wxString& GetNumber() const { return number; }
  inline void SetNumber(const wxString& newnumber) { number = newnumber; }
  inline unsigned short GetBeats() const { return beats; }
  inline void SetBeats(unsigned short b) { beats = b; }
  inline bool IsInAnimation() { return (beats != 0); }
  void UserSetName(const wxString& newname);
  void UserSetBeats(unsigned short b);
  bool SetPointsSym(SYMBOL_TYPE sym);
  bool SetPointsLabel(bool right);
  bool SetPointsLabelFlip();

  inline const CC_point& GetPoint(unsigned i) const { return pts[i]; }
  inline CC_point& GetPoint(unsigned i) { return pts[i]; }
  inline void SetPoint(const cc_oldpoint& val, unsigned i);

  const CC_coord& GetPosition(unsigned i, unsigned ref = 0) const;
  void SetAllPositions(const CC_coord& val, unsigned i);
  void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);
  void SetPositionQuick(const CC_coord& val, unsigned i, unsigned ref = 0);
  bool ClearRefPositions(unsigned ref);
  bool TranslatePoints(const CC_coord& delta, unsigned ref = 0);
  bool TransformPoints(const Matrix& transmat, unsigned ref = 0);
  bool MovePointsInLine(const CC_coord& start, const CC_coord& second,
			unsigned ref);

  CC_sheet *next;
  CC_text continuity;
  CC_continuity *animcont;
  CC_show *show;
  unsigned numanimcont;
  bool picked; /* for requestors like printing */
private:
  unsigned short beats;
  std::vector<CC_point> pts;
  wxString name;
  wxString number;
};

#define DEF_HASH_W 32
#define DEF_HASH_E 52

class ShowUndoList;
class ShowInfoReq;
class ShowMode;

class CC_show {
public:
  CC_show(unsigned npoints = 0);
  CC_show(const wxString& file);
  ~CC_show();

  wxString ImportContinuity(const wxString& file);

  int Print(FILE *fp, bool eps = false, bool overview = false,
	    unsigned curr_ss = 0, int min_yards = 50) const;

  inline const wxString& GetError() const { return error; }
  inline bool Ok() const { return okay; }

  void Append(CC_show *shw);
  void Append(CC_sheet *newsheets);
  wxString Save(const wxString& filename) const;
  wxString Autosave() const;
  void ClearAutosave() const;

  inline void FlushAllTextWindows() const {
    winlist->FlushDescr(); winlist->FlushContinuity();
  }

  inline const wxString& GetName() { return name; }
  wxString UserGetName() const;
  void SetName(const wxString& newname) const;
  inline void UserSetName(const wxString& newname) const {
    SetName(newname); winlist->ChangeName();
  }

  inline const wxString& GetDescr() const { return descr; }
  inline const wxString& UserGetDescr() const { winlist->FlushDescr(); return descr; }
  inline void SetDescr(const wxString& newdescr) { descr = newdescr; }
  void UserSetDescr(const wxString& newdescr, wxWindow* win);

  inline bool Modified() const { return modified; }
  inline void SetModified(bool b) const { modified = b; winlist->UpdateStatusBar(); }

  inline unsigned short GetNumSheets() const { return numsheets; }
  inline CC_sheet *GetSheet() const { return sheets; }
  const CC_sheet *GetNthSheet(unsigned n) const;
  CC_sheet *GetNthSheet(unsigned n);
  unsigned GetSheetPos(const CC_sheet *sheet) const;
  CC_sheet *RemoveNthSheet(unsigned sheetidx);
  CC_sheet *RemoveLastSheets(unsigned numtoremain);
  void DeleteNthSheet(unsigned sheetidx);
  void UserDeleteSheet(unsigned sheetidx);
  void InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx);
  void InsertSheet(CC_sheet *nsheet, unsigned sheetidx);
  void UserInsertSheet(CC_sheet *sht, unsigned sheetidx);
  inline unsigned short GetNumPoints() const { return numpoints; }
  void SetNumPoints(unsigned num, unsigned columns);
  void SetNumPointsInternal(unsigned num); //Only for creating show class
  bool RelabelSheets(unsigned sht);

  inline const wxString& GetPointLabel(unsigned i) const { return pt_labels[i]; }
  inline wxString& GetPointLabel(unsigned i) { return pt_labels[i]; }
  inline const wxString* GetPointLabels() const { return &pt_labels[0]; }
  inline wxString* GetPointLabels() { return &pt_labels[0]; }
  inline bool GetBoolLandscape() const { return print_landscape; }
  inline bool GetBoolDoCont() const { return print_do_cont; }
  inline bool GetBoolDoContSheet() const { return print_do_cont_sheet; }
  inline bool& GetBoolLandscape() { return print_landscape; }
  inline bool& GetBoolDoCont() { return print_do_cont; }
  inline bool& GetBoolDoContSheet() { return print_do_cont_sheet; }

  bool UnselectAll();
  inline bool IsSelected(unsigned i) const { return selections[i]; }
  void Select(unsigned i, bool val = true);
  inline void SelectToggle(unsigned i) {
    selections[i] = selections[i] ? false:true;
  }
  typedef std::vector<unsigned> SelectionList;
  inline const SelectionList& GetSelectionList() const { return selectionList; }

  CC_WinListShow *winlist;
  ShowUndoList *undolist;
  ShowMode *mode;

private:
  void PrintSheets(FILE *fp) const; // called by Print()
  wxString SaveInternal(const wxString& filename) const;
  void SetAutosaveName(const wxString& realname) const;

  void AddError(const wxString& str) const {
    error += str + wxT("\n");
    okay = false;
  }

  mutable wxString error;
  mutable bool okay;

  mutable wxString name;
  mutable wxString autosave_name;
  wxString descr;
  unsigned short numpoints;
  unsigned short numsheets;
  CC_sheet *sheets;
  std::vector<wxString> pt_labels;
  bool *selections; // array for each point
  std::vector<unsigned> selectionList; // order of selections
  mutable bool modified;
  bool print_landscape;
  bool print_do_cont;
  bool print_do_cont_sheet;
};

class CC_descr {
public:
  CC_show *show;
  unsigned curr_ss;
  inline CC_sheet *CurrSheet() { return show->GetNthSheet(curr_ss); }
};

void SetAutoSave(int secs);

#endif
