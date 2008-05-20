/* cont_ui.h
 * Header for continuity editors
 *
 * Modification history:
 * 1-10-96    Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1996-2008  Garrick Brian Meeker

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
  virtual void GotoContLocation(unsigned sht, unsigned contnum,
				int line = -1, int col = -1);
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
		   wxFrame *parent, const wxString& title,
		   int x = -1, int y = -1, int width = 400, int height = 300);
  ~ContinuityEditor();
  void OnSize(wxSizeEvent& event);

  void OnCloseWindow(wxCloseEvent& event);
  void OnCmdNew(wxCommandEvent& event);
  void OnCmdDelete(wxCommandEvent& event);
  void OnCmdClose(wxCommandEvent& event);
  void OnCmdHelp(wxCommandEvent& event);

  void Update(bool quick = false); // Refresh all window controls
  // Update text window to current continuity
  // quick doesn't flush other windows
  void UpdateText(bool quick = false);

  void FlushText(); // Flush changes in text window
  inline void DetachText() { text_sheet = NULL; } // When sheet goes away

  inline unsigned GetCurrent() { return curr_cont; }
  inline void SetCurrent(unsigned i) { curr_cont = i; UpdateText(); }
  void UpdateContChoice();

  inline void IncCurrent() { curr_cont++; text_contnum++; }
  inline void DecCurrent() { curr_cont--; text_contnum--; }

  inline CC_descr *GetShowDescr() { return descr; }

  void SelectPoints();
  void SetPoints();

  inline void SetInsertionPoint(int x, int y) {
    text->SetInsertionPoint(text->XYToPosition((long)x-1,(long)y-1));
    text->SetFocus();
  }
private:
  CC_descr *descr;
  unsigned curr_cont;
  wxPanel *panel;
  wxChoice *conts;
  FancyTextWin *text;
  CC_sheet *text_sheet;
  unsigned text_contnum;
  CC_WinNodeCont *node;

  DECLARE_EVENT_TABLE()
};

class PrintContCanvas : public wxScrolledWindow {
public:
  PrintContCanvas(wxFrame *frame, CC_descr *dcr);
  ~PrintContCanvas();

  void Draw(wxDC *dc, int firstrow = 0, int lastrow = -1);

  void OnPaint(wxPaintEvent& event);
  void OnMouseEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& event);
  inline void Update() { topline = 0; Refresh(); UpdateBars(); }
  void UpdateBars();

private:
  void MoveCursor(unsigned column, unsigned row);
  void DrawCursor(wxDC *dc, float x, float y, float height);
  void InsertChar(unsigned onechar);
  void DeleteChar(bool backspace = true);

  CC_descr *show_descr;
  wxFrame *ourframe;
  unsigned topline;
  float width, height;
  unsigned cursorx, cursory;
  unsigned maxlines, maxcolumns;

  DECLARE_EVENT_TABLE()
};

class PrintContEditor : public wxFrame {
public:
  PrintContEditor(CC_descr *dcr, CC_WinList *lst,
		  wxFrame *parent, const wxString& title,
		  int x = -1, int y = -1, int width = 400, int height = 400);
  ~PrintContEditor();

  void OnActivate(wxActivateEvent& event);

  PrintContCanvas *canvas;
private:
  CC_WinNodePrintCont *node;

  DECLARE_EVENT_TABLE()
};

enum {
  CALCHART__CONT_NEW = 100,
  CALCHART__CONT_DELETE,
  CALCHART__CONT_CLOSE,
  CALCHART__CONT_HELP
};

#endif
