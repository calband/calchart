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

class CC_WinNodeCont : public CC_WinNode
{
public:
	CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req);

	virtual void GotoSheet(unsigned sht);
	virtual void GotoContLocation(unsigned sht, unsigned contnum,
		int line = -1, int col = -1);
	virtual void DeleteSheet(unsigned sht);
	virtual void RemoveSheets(unsigned num);
	virtual void FlushContinuity();

private:
	ContinuityEditor *editor;
};

class CC_WinNodePrintCont : public CC_WinNode
{
public:
	CC_WinNodePrintCont(CC_WinList *lst, PrintContEditor *req);

	virtual void GotoSheet(unsigned sht);

private:
	PrintContEditor *editor;
};

// View for linking CC_show with ContinuityEditor
class ContinuityEditorView : public wxView
{
public:
	ContinuityEditorView();
	~ContinuityEditorView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	void DoSetContinuityIndex(unsigned cont);
	void DoSetNthContinuity(const wxString& text, unsigned i);
	void DoNewContinuity(const wxString& text);
	void DoDeleteContinuity(unsigned cont);
};

// ContinuityEditor
// The way you edit the continuity for individual marchers
class ContinuityEditor : public wxFrame
{
public:
	ContinuityEditor(CC_show *dcr, CC_WinList *lst,
		wxFrame *parent, const wxString& title,
		int x = -1, int y = -1, int width = 400, int height = 300);
	~ContinuityEditor();

	void OnCloseWindow(wxCloseEvent& event);
	void OnCmdNew(wxCommandEvent& event);
	void OnCmdDelete(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);

	void Update();		  // Refresh all window controls
// Update text window to current continuity
// quick doesn't flush other windows
	void UpdateText();

	void FlushText();						  // Flush changes in text window
	void DetachText();				  // When sheet goes away

	inline unsigned GetCurrent() { return mCurrentContinuityChoice; }
	inline void SetCurrent(unsigned i) { mCurrentContinuityChoice = i; UpdateText(); }
	void UpdateContChoice();

	const CC_show *GetShow() { return mShow; }

	void SelectPoints();
	void SetPoints();

	inline void SetInsertionPoint(int x, int y)
	{
		mUserInput->SetInsertionPoint(mUserInput->XYToPosition((long)x-1,(long)y-1));
		mUserInput->SetFocus();
	}
private:
	void ContEditSet(wxCommandEvent&);
	void ContEditSelect(wxCommandEvent&);
	void ContEditSave(wxCommandEvent&);
	void ContEditCurrent(wxCommandEvent&);
	CC_show *mShow;
	ContinuityEditorView *mView;
	wxPanel *panel;
	wxChoice *mContinuityChoices;
	unsigned mCurrentContinuityChoice;
	FancyTextWin *mUserInput;
	CC_sheet *mSheetUnderEdit;
	CC_WinNodeCont *node;

	DECLARE_EVENT_TABLE()
};

class PrintContCanvas : public wxScrolledWindow
{
public:
	PrintContCanvas(wxFrame *frame, CC_show *show);
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

	CC_show *mShow;
	wxFrame *ourframe;
	unsigned topline;
	float width, height;
	unsigned cursorx, cursory;
	unsigned maxlines, maxcolumns;

	DECLARE_EVENT_TABLE()
};

class PrintContEditorView : public wxView
{
public:
	PrintContEditorView();
	~PrintContEditorView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

class PrintContEditor : public wxFrame
{
public:
	PrintContEditor(CC_show *show, CC_WinList *lst,
		wxFrame *parent, const wxString& title,
		int x = -1, int y = -1, int width = 400, int height = 400);
	~PrintContEditor();

	void OnActivate(wxActivateEvent& event);

	PrintContCanvas *canvas;
private:
	PrintContEditorView *mView;
	CC_WinNodePrintCont *node;

	DECLARE_EVENT_TABLE()
};

enum
{
	CALCHART__CONT_NEW = 100,
	CALCHART__CONT_DELETE,
	CALCHART__CONT_CLOSE,
	CALCHART__CONT_HELP
};
#endif
