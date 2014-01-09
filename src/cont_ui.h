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

#include "calchartdoc.h"

#include <wx/docview.h>
#include <wx/dialog.h>

class ContinuityEditor;
class FancyTextWin;


/**
 * A view used to link the ContinuityEditor to the CalChartDoc. When the
 * CalChartDoc updates all of its views, it will send updates to this
 * view, and this view will route those updates to the Continuity Editor.
 * Also, when the Continuity Editor changes a continuity, it informs the
 * CalChartDoc through the view.
 */
class ContinuityEditorView : public wxView
{
public:
	/**
	 * Makes the view.
	 */
	ContinuityEditorView();
	/**
	 * Cleanup.
	 */
	~ContinuityEditorView();
	/**
	 * Does nothing - this view is designed ONLY to connect the
	 * Continuity Editor to the CalChartDoc.
	 * @param dc Unused.
	 */
    virtual void OnDraw(wxDC *dc);
	/**
	 * Called when the CalChartDoc updates its views. This will route
	 * The updates to the Continuity Editor, which is the frame that
	 * this view is associated with.
	 * @param sender The view that sent the update.
	 * @param hint A message sent indicating the reason for the update.
	 */
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	/**
	 * Sets the text of a continuity for the current sheet of the CalChartDoc.
	 * @param which The symbol type which the continuity is associate with.
	 * @param text The new text for the continuity.
	 */
	void DoSetContinuityText(SYMBOL_TYPE which, const wxString& text);
};


/**
 * The continuity editor.
 */
class ContinuityEditor : public wxFrame
{
	friend class ContinuityEditorView;
public:
	/**
	 * Makes the continuity editor.
	 */
	ContinuityEditor();
	/**
	 * Makes the continuity editor.
	 * @param dcr The doc to modify.
	 * @param parent The parent for this frame. If the parent is
	 * closed, this frame will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the frame.
	 * @param pos The position of the frame.
	 * @param size The size of the frame.
	 * @param style The style in which the frame will be drawn.
	 */
	ContinuityEditor(CalChartDoc *dcr,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Edit Continuity"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	/**
	 * Cleanup.
	 */
	~ContinuityEditor();

	/**
	 * Called when the window is closed.
	 * @param event Unused.
	 */
	void OnCloseWindow(wxCommandEvent& event);
	/**
	 * Called when the user selects the "Help" button.
	 * @param event Unused.
	 */
	void OnCmdHelp(wxCommandEvent& event);

	/**
	 * Updates all components of the frame to reflect the
	 * information stored in the CalChartDoc and the current
	 * settings for the Continuity Editor.
	 */
	void Update();

	/**
	 * Updates the Continuity text so that it reflects the continuity text
	 * that is currently saved in the CalChartDoc.
	 */
	void UpdateText();

	/**
	 * Sends the continuity text stored in the text box to the CalChartDoc,
	 * thereby saving any changes.
	 */
	void FlushText();						  

	/**
	 * Sets up the text box so that the user can insert text at a certain
	 * point.
	 * @param x The x coordinate on the screen where the user should insert
	 * text.
	 * @param y The y coordinate on the screen where the user should insert
	 * text.
	 */
	void SetInsertionPoint(int x, int y);

private:
	/**
	 * Initializes the window, before the Create(...) method is called.
	 */
	void Init();

	/**
	 * Creates the window.
	 * @param shw The doc to modify.
	 * @param parent The parent for this frame. If the parent is
	 * closed, this frame will be closed too.
	 * @param id The identity of the window.
	 * @param caption The title of the frame.
	 * @param pos The position of the frame.
	 * @param size The size of the frame.
	 * @param style The style in which the frame will be drawn.
	 */
	bool Create(CalChartDoc *shw,
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Select Points"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	/**
	 * Creates the GUI components of the frame.
	 */
	void CreateControls();
	/**
	 * Called when the user clicks the "Set Points" button.
	 */
	void ContEditSet(wxCommandEvent&);
	/**
	 * Called when the user clicks the "Select Points" button.
	 */
	void ContEditSelect(wxCommandEvent&);
	/**
	 * Called when the user clicks the "Save" button.
	 */
	void OnSave(wxCommandEvent&);
	/**
	 * Called when the user clicks the "Discard" button.
	 */
	void OnDiscard(wxCommandEvent&);
	/**
	 * Called when the user selects a new dot type to edit continuities for.
	 */
	void ContEditCurrent(wxCommandEvent&);
	/**
	 * Called when a keyboard key is pressed.
	 */
	void OnKeyPress(wxCommandEvent&);
	/**
	 * Returns the symbol whose continuities are currently
	 * being edited.
	 * @return The symbol whose continuities are currently being
	 * edited.
	 */
	SYMBOL_TYPE CurrentSymbolChoice() const;

	/**
	 * Saves all changes to the continuity text to the CalChartDoc.
	 */
	void Save();
	/**
	 * Discards all changes to the continuity text without
	 * saving.
	 */
	void Discard();
	/**
	 * Specifies which symbol type's continuities should be edited.
	 * @param i The index of the symbol type whose continuities should
	 * be edited.
	 */
	void SetCurrent(unsigned i);

	/**
	 * The document to modify.
	 */
	CalChartDoc *mDoc;
	/**
	 * The view through which the editor is connected to the CalChartDoc.
	 */
	ContinuityEditorView *mView;
	/**
	 * The dropdown menu that is used to select the symbol type whose
	 * continuities will be edited.
	 */
	wxChoice *mContinuityChoices;
	/**
	 * The index representing the user's choice for the active symbol type.
	 * This index is understood by mContinuityChoices.
	 * The active symbol type is the symbol type whose continuities are
	 * being edited.
	 */
	unsigned mCurrentContinuityChoice;
	/**
	 * The box where the user enters continuities.
	 */
	FancyTextWin *mUserInput;
	/**
	 * TODO - Unused?
	 */
	CC_show::const_CC_sheet_iterator_t mSheetUnderEdit;

	DECLARE_EVENT_TABLE()
};

#endif
