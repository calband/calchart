/* show_ui.h
 * Classes for interacting with shows
 *
 * Modification history:
 * 8-7-95     Garrick Meeker              Created
 *
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
		   Bool multi, wxFrame *frame, char *title,
		   int x = -1, int y = -1, int width = 250, int height = 300);
  ~StuntSheetPicker();
  Bool OnClose(void);
  void OnSize(int w, int h);

  inline Bool Okay() { return ok; };

  inline Bool Get(unsigned n) { return list->Selected(n); }
  inline void Set(unsigned n, Bool v = TRUE) {
    if (!v || !list->Selected(n)) { // so Motif version works
      list->SetSelection(n,v);
      show->GetNthSheet(n)->picked = v;
    }
  }
  void Update();

  CC_show *show;
private:
  void SetListBoxEntries();

  Bool ok;
  wxPanel *panel;
  wxListBox *list;
  CC_WinNodePicker *node;
};

class PointPicker : public wxFrame
{
public:
  PointPicker(CC_show *shw, CC_WinList *lst,
	      Bool multi, wxFrame *frame, char *title,
	      int x = -1, int y = -1, int width = 250, int height = 300);
  ~PointPicker();
  Bool OnClose(void);
  void OnSize(int w, int h);

  inline Bool Okay() { return ok; };

  inline Bool Get(unsigned n) { return list->Selected(n); }
  inline void Set(unsigned n, Bool v = TRUE) {
    if (!v || !list->Selected(n)) { // so Motif version works
      list->SetSelection(n,v);
      show->Select(n,v);
    }
  }
  void Update();

  CC_show *show;
private:
  void SetListBoxEntries();

  Bool ok;
  wxPanel *panel;
  wxListBox *list;
  CC_WinNodePointPicker *node;
};

class ShowInfoReq : public wxFrame {
public:
  ShowInfoReq(CC_show *shw, CC_WinList *lst,
	      wxFrame *frame, char *title,
	      int x = -1, int y = -1, int width = 400, int height = 450);
  ~ShowInfoReq();
  Bool OnClose(void);

  void UpdateLabels();
  void UpdateNumPoints();
  void UpdateMode();
  void UpdateDescr(Bool quick = FALSE); // quick doesn't flush other windows
  void Update(Bool quick = FALSE, CC_show *shw = NULL);

  void FlushDescr(); // Flush changes in description text window

  inline char *GetChoiceStrSelection() { return choice->GetStringSelection(); }
  inline int GetChoiceSelection() { return choice->GetSelection(); }
  unsigned GetNumPoints();
  unsigned GetColumns();
  inline int GetLabelType() { return label_type->GetSelection(); }
  inline int GetLetterSize() { return lettersize->GetValue(); }
  inline Bool GetLetter(unsigned i) { return labels->Selected(i); }
  void SetLabels();

  CC_show *show;
private:
  wxPanel *panel;
  wxText *numpnts;
  wxRadioBox *label_type;
  wxListBox *labels;
  wxChoice *choice;
  wxSlider *lettersize;
  FancyTextWin *text;
  CC_WinNodeInfo *node;
};

#endif
