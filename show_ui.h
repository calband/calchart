/* show_ui.h
 * Classes for interacting with shows
 *
 * Modification history:
 * 8-7-95     Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#ifndef _SHOW_UI_H_
#define _SHOW_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "show.h"
#include "basic_ui.h"

class StuntSheetPicker;
class PointPicker;
class ShowInfoReq;

class CC_WinNodePicker : public CC_WinNode {
public:
  CC_WinNodePicker(CC_WinList *lst, StuntSheetPicker *req);

  virtual void SetShow(CC_show *shw);
  virtual void AddSheet(unsigned sht);
  virtual void DeleteSheet(unsigned sht);
  virtual void RemoveSheets(unsigned num);
  virtual void AppendSheets();
  virtual void ChangeTitle(unsigned sht);
  virtual void SelectSheet(wxWindow* win, unsigned sht);

private:
  StuntSheetPicker *picker;
};

class CC_WinNodePointPicker : public CC_WinNode {
public:
  CC_WinNodePointPicker(CC_WinList *lst, PointPicker *req);

  virtual void SetShow(CC_show *shw);
  virtual void UpdateSelections(wxWindow* win, int point = -1);
  virtual void ChangeNumPoints(wxWindow *win);
  virtual void ChangePointLabels(wxWindow *win);

private:
  PointPicker *picker;
};

class CC_WinNodeInfo : public CC_WinNode {
public:
  CC_WinNodeInfo(CC_WinList *lst, ShowInfoReq *req);

  virtual void SetShow(CC_show *shw);
  virtual void ChangeNumPoints(wxWindow *win);
  virtual void ChangePointLabels(wxWindow *win);
  virtual void ChangeShowMode(wxWindow *win);
  virtual void FlushDescr();
  virtual void SetDescr(wxWindow *win);

private:
  ShowInfoReq *inforeq;
};

class StuntSheetPicker : public wxFrame
{
public:
  StuntSheetPicker(CC_show *shw, CC_WinList *lst,
		   bool multi, wxFrame *frame, char *title,
		   int x = -1, int y = -1, int width = 250, int height = 300);
  ~StuntSheetPicker();
  bool OnClose(void);
  void OnSize(int w, int h);

  inline bool Okay() { return ok; };

  inline bool Get(unsigned n) { return list->Selected(n); }
  inline void Set(unsigned n, bool v = true) {
    list->SetSelection(n,v);
    show->GetNthSheet(n)->picked = v;
  }
  void Update();

  CC_show *show;
private:
  void SetListBoxEntries();

  bool ok;
  wxPanel *panel;
  GoodListBox *list;
  CC_WinNodePicker *node;
};

class PointPicker : public wxFrame
{
public:
  PointPicker(CC_show *shw, CC_WinList *lst,
	      bool multi, wxFrame *frame, char *title,
	      int x = -1, int y = -1, int width = 250, int height = 300);
  ~PointPicker();
  bool OnClose(void);
  void OnSize(int w, int h);

  inline bool Okay() { return ok; };

  inline bool Get(unsigned n) { return list->Selected(n); }
  inline void Set(unsigned n, bool v = true) {
    list->SetSelection(n,v);
    show->Select(n,v);
  }
  void Update();
  void UpdateSelections();

  CC_show *show;
private:
  void SetListBoxEntries();

  bool ok;
  wxPanel *panel;
  GoodListBox *list;
  CC_WinNodePointPicker *node;
};

class ShowInfoReq : public wxFrame {
public:
  ShowInfoReq(CC_show *shw, CC_WinList *lst,
	      wxFrame *frame, char *title,
	      int x = -1, int y = -1, int width = 400, int height = 450);
  ~ShowInfoReq();
  bool OnClose(void);

  void UpdateLabels();
  void UpdateNumPoints();
  void UpdateMode();
  void UpdateDescr(bool quick = false); // quick doesn't flush other windows
  void Update(bool quick = false, CC_show *shw = NULL);

  void FlushDescr(); // Flush changes in description text window

  inline char *GetChoiceStrSelection() { return choice->GetStringSelection(); }
  inline int GetChoiceSelection() { return choice->GetSelection(); }
  unsigned GetNumPoints();
  unsigned GetColumns();
  inline int GetLabelType() { return label_type->GetSelection(); }
  inline int GetLetterSize() { return lettersize->GetValue(); }
  inline bool GetLetter(unsigned i) { return labels->Selected(i); }
  void SetLabels();

  CC_show *show;
private:
  wxPanel *panel;
  wxText *numpnts;
  wxRadioBox *label_type;
  GoodListBox *labels;
  wxChoice *choice;
  wxSlider *lettersize;
  FancyTextWin *text;
  CC_WinNodeInfo *node;
};

#endif
