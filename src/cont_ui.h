/*
 * cont_ui.h
 * Header for continuity editors
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "basic_ui.h"
#include "cc_show.h"

#include <wx/docview.h>
#include <wx/dialog.h>

class ContinuityEditor;

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
// This dialog should notify the user to save if there are any outstanding edits.
class ContinuityEditor : public wxFrame
{
	friend class ContinuityEditorView;
public:
	ContinuityEditor();
	ContinuityEditor(CC_show *dcr,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Edit Continuity"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~ContinuityEditor();

	void OnCloseWindow(wxCommandEvent& event);
	void OnCmdNew(wxCommandEvent& event);
	void OnCmdDelete(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);

	void Update();		  // Refresh all window controls
// Update text window to current continuity
// quick doesn't flush other windows
	void UpdateText();

	void FlushText();						  // Flush changes in text window

	void UpdateContChoice();

	inline void SetInsertionPoint(int x, int y)
	{
		mUserInput->SetInsertionPoint(mUserInput->XYToPosition((long)x-1,(long)y-1));
		mUserInput->SetFocus();
	}
private:
	void Init();

	bool Create(CC_show *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	void ContEditSet(wxCommandEvent&);
	void ContEditSelect(wxCommandEvent&);
	void OnSave(wxCommandEvent&);
	void OnDiscard(wxCommandEvent&);
	void ContEditCurrent(wxCommandEvent&);
	void OnKeyPress(wxCommandEvent&);

	void Save();
	void Discard();
	void SetCurrent(unsigned i);

	CC_show *mShow;
	ContinuityEditorView *mView;
	wxChoice *mContinuityChoices;
	unsigned mCurrentContinuityChoice;
	FancyTextWin *mUserInput;
	CC_show::const_CC_sheet_iterator_t mSheetUnderEdit;

	DECLARE_EVENT_TABLE()
};

#endif
