/* cont_ui.h
 * Header for continuity editors
 *
 * Modification history:
 * 1-10-96    Garrick Meeker              Created
 *
 */

#ifndef _CONT_UI_H_
#define _CONT_UI_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "basic_ui.h"
#include "show.h"

class ContinuityEditor;
class PrintContEditor;

class CC_WinNodeCont : public CC_WinNode {
public:
  CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req);

  virtual void SetShow(CC_show *shw);
  virtual void GotoSheet(unsigned sht);
  virtual void DeleteSheet(unsigned sht);
  virtual void RemoveSheets(unsigned num);
  virtual void AddContinuity(unsigned sht, unsigned cont);
  virtual void DeleteContinuity(unsigned sht, unsigned cont);
  virtual void FlushContinuity();
  virtual void SetContinuity(wxWindow *win, unsigned sht, unsigned cont);

private:
  ContinuityEditor *editor;
};

class CC_WinNodePrintCont : public CC_WinNode {
public:
  CC_WinNodePrintCont(CC_WinList *lst, PrintContEditor *req);

  virtual void SetShow(CC_show *shw);
  virtual void GotoSheet(unsigned sht);

private:
  PrintContEditor *editor;
};

class ContinuityEditor : public wxFrame {
public:
  ContinuityEditor(CC_descr *dcr, CC_WinList *lst,
		   wxFrame *parent, char *title,
		   int x = -1, int y = -1, int width = 400, int height = 300);
  ~ContinuityEditor();
  void OnSize(int w, int h);

  Bool OnClose(void);
  void OnMenuCommand(int id);
  void OnMenuSelect(int id);

  void Update(Bool quick = FALSE); // Refresh all window controls
  // Update text window to current continuity
  // quick doesn't flush other windows
  void UpdateText(Bool quick = FALSE);

  void FlushText(); // Flush changes in text window
  inline void DetachText() { text_sheet = NULL; } // When sheet goes away

  inline unsigned GetCurrent() { return curr_cont; }
  inline void SetCurrent(unsigned i) { curr_cont = i; UpdateText(); }

  inline void IncCurrent() { curr_cont++; text_contnum++; }
  inline void DecCurrent() { curr_cont--; text_contnum--; }

  inline CC_descr *GetShowDescr() { return descr; }

  void SelectPoints();
  void SetPoints();

private:
  CC_descr *descr;
  unsigned curr_cont;
  wxPanel *panel;
  wxChoice *conts;
  FancyTextWin *text;
  CC_sheet *text_sheet;
  unsigned text_contnum;
  CC_WinNodeCont *node;
};

class PrintContCanvas : public wxCanvas {
public:
  PrintContCanvas(wxFrame *frame, CC_descr *dcr);
  ~PrintContCanvas();

  void OnPaint();
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);

  inline void Update() { topline = 0; OnPaint(); }

private:
  CC_descr *show_descr;
  wxFrame *ourframe;
  unsigned topline;
};

class PrintContEditor : public wxFrameWithStuff {
public:
  PrintContEditor(CC_descr *dcr, CC_WinList *lst,
		  wxFrame *parent, char *title,
		  int x = -1, int y = -1, int width = 400, int height = 400);
  ~PrintContEditor();

  PrintContCanvas *canvas;
private:
  CC_WinNodePrintCont *node;
};

enum {
  CALCHART__CONT_NEW = 100,
  CALCHART__CONT_DELETE,
  CALCHART__CONT_CLOSE
};

#endif
