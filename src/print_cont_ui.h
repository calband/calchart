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

#pragma once

#include "calchartdoc.h"

#include <wx/docview.h>
#include <wx/dialog.h>

class PrintContinuityEditor;
class FancyTextWin;
class FancyTextPanel;
class wxSplitterWindow;

// View for linking CalChartDoc with PrintContinuityEditor
class PrintContinuityEditorView : public wxView
{
public:
	PrintContinuityEditorView();
	~PrintContinuityEditorView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	void DoSetPrintContinuity(unsigned which_sheet, const wxString& number, const wxString& cont);
};

// PrintContinuityEditor
// The way you edit the continuity for individual marchers
// This dialog should notify the user to save if there are any outstanding edits.
class PrintContinuityEditor : public wxFrame
{
	friend class PrintContinuityEditorView;
public:
	PrintContinuityEditor();
	PrintContinuityEditor(CalChartDoc *dcr,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Edit Print Continuity"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~PrintContinuityEditor();

	void OnCloseWindow(wxCommandEvent& event);
	void OnCmdHelp(wxCommandEvent& event);

	void Update();		  // Refresh all window controls
// Update text window to current continuity
// quick doesn't flush other windows
	void UpdateText();

	void FlushText();						  // Flush changes in text window

	void SetInsertionPoint(int x, int y);
	void UpdateOrientation(wxCommandEvent&);

private:
	void Init();

	bool Create(CalChartDoc *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	void OnSave(wxCommandEvent&);
	void OnDiscard(wxCommandEvent&);
	void ContEditCurrent(wxCommandEvent&);
	void OnKeyPress(wxCommandEvent&);

	void Save();
	void Discard();

	CalChartDoc *mDoc;
	PrintContinuityEditorView *mView;
	FancyTextWin *mUserInput;
	FancyTextPanel *mPrintContDisplay;
	wxSplitterWindow *mSplitter;

	DECLARE_EVENT_TABLE()
};