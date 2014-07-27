/*
 * cont_ui.cpp
 * Continuity editors
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

#include "precomp.h"

#include "basic_ui.h"
#include "cont_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_show.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>

enum
{
	CALCHART__CONT_CLOSE = 100,
	ContinuityEditor_ContEditSelect,
	ContinuityEditor_Save,
	ContinuityEditor_Discard,
	ContinuityEditor_ContEditCurrent,
	ContinuityEditor_KeyPress,
};

BEGIN_EVENT_TABLE(ContinuityEditor, wxFrame)
EVT_MENU(wxID_CLOSE, ContinuityEditor::OnCloseWindow)
EVT_MENU(wxID_HELP, ContinuityEditor::OnCmdHelp)
EVT_MENU(ContinuityEditor_ContEditSelect,ContinuityEditor::ContEditSelect)
EVT_MENU(ContinuityEditor_Save,ContinuityEditor::OnSave)
EVT_MENU(ContinuityEditor_Discard,ContinuityEditor::OnDiscard)
EVT_BUTTON(wxID_CLOSE, ContinuityEditor::OnCloseWindow)
EVT_BUTTON(wxID_HELP, ContinuityEditor::OnCmdHelp)
EVT_BUTTON(ContinuityEditor_ContEditSelect,ContinuityEditor::ContEditSelect)
EVT_BUTTON(ContinuityEditor_Save,ContinuityEditor::OnSave)
EVT_BUTTON(ContinuityEditor_Discard,ContinuityEditor::OnDiscard)
EVT_CHOICE(ContinuityEditor_ContEditCurrent,ContinuityEditor::ContEditCurrent)
EVT_TEXT(ContinuityEditor_KeyPress,ContinuityEditor::OnKeyPress)
END_EVENT_TABLE()

ContinuityEditorView::ContinuityEditorView() {}
ContinuityEditorView::~ContinuityEditorView() {}

void ContinuityEditorView::OnDraw(wxDC *dc) {}
void ContinuityEditorView::OnUpdate(wxView *sender, wxObject *hint)
{
	ContinuityEditor* editor = static_cast<ContinuityEditor*>(GetFrame());
	if (hint && hint->IsKindOf(CLASSINFO(CalChartDoc_FlushAllViews)))
	{
		editor->FlushText();
	}
	else
	{
		editor->Update();
	}
}

void ContinuityEditorView::DoSetContinuityText(SYMBOL_TYPE which, const wxString& text)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetContinuityTextCommand(*static_cast<CalChartDoc*>(GetDocument()), which, text), true);
}

ContinuityEditor::ContinuityEditor()
{
	Init();
}

ContinuityEditor::ContinuityEditor(CalChartDoc *show,
		wxWindow *parent, wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	Init();
	
	Create(show, parent, id, caption, pos, size, style);
}

void ContinuityEditor::Init()
{
}

bool ContinuityEditor::Create(CalChartDoc *show,
		wxWindow *parent, wxWindowID id,
		const wxString& caption,
		const wxPoint& pos,
		const wxSize& size,
		long style )
{
	if (!wxFrame::Create(parent, id, caption, pos, size, style))
		return false;

	mDoc = show;
	mCurrentContinuityChoice = 0;
	mView = new ContinuityEditorView;
	mView->SetDocument(show);
	mView->SetFrame(this);

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	// now update the current screen
	Update();

	return true;
}

void ContinuityEditor::CreateControls()
{
	// menu bar
	wxMenu *cont_menu = new wxMenu;
	cont_menu->Append(ContinuityEditor_Save, wxT("&Save Continuity\tCTRL-S"), wxT("Save continuity"));
	cont_menu->Append(ContinuityEditor_ContEditSelect, wxT("Select &Points\tCTRL-P"), wxT("Select Points"));
	cont_menu->Append(wxID_CLOSE, wxT("&Close Window\tCTRL-W"), wxT("Close this window"));

	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_HELP, wxT("&Help on Continuity...\tCTRL-H"), wxT("Help on Continuity"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(cont_menu, wxT("&File"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);

// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
	SetSizer( topsizer );

// add buttons to the top row
	// New, delete, choices
	wxBoxSizer *top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	mContinuityChoices = new wxChoice(this, ContinuityEditor_ContEditCurrent);
	top_button_sizer->Add(mContinuityChoices, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	// select
	wxButton* button = new wxButton(this, ContinuityEditor_ContEditSelect, wxT("Select &Points"));
	top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	topsizer->Add(top_button_sizer);


	mUserInput = new FancyTextWin(this, ContinuityEditor_KeyPress, wxEmptyString, wxDefaultPosition, wxSize(50, 300));
	
	topsizer->Add(mUserInput, 0, wxGROW|wxALL, 5 );

	// add a horizontal bar to make things clear:
	wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	topsizer->Add(line, 0, wxGROW|wxALL, 5);
	
	// add a save, discard, close, and help
	top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	button = new wxButton(this, ContinuityEditor_Save, wxT("&Save"));
	top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	button = new wxButton(this, ContinuityEditor_Discard, wxT("&Discard"));
	top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	button = new wxButton(this, wxID_CLOSE, wxT("&Close"));
	top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	button = new wxButton(this, wxID_HELP, wxT("&Help"));
	top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	topsizer->Add(top_button_sizer);
}


ContinuityEditor::~ContinuityEditor()
{
	if (mView)
		delete mView;
}


void ContinuityEditor::OnCloseWindow(wxCommandEvent& event)
{
	// if the current field is modified, then do something
	if (mUserInput->IsModified())
	{
		// give the user a chance to save, discard, or cancle the action
		int userchoice = wxMessageBox(wxT("Continuity modified.  Save changes or cancel?"), wxT("Save changes?"), wxYES_NO|wxCANCEL);
		if (userchoice == wxYES)
		{
			Save();
		}
		if (userchoice == wxNO)
		{
			Discard();
		}
		if (userchoice == wxCANCEL)
		{
			wxString message = wxT("Close cancelled.");
			wxMessageBox(message, message);
			return;
		}
	}

	FlushText();
	Close();
}

void ContinuityEditor::OnCmdHelp(wxCommandEvent& event)
{
	wxGetApp().GetGlobalHelpController().LoadFile();
	wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
}


void ContinuityEditor::Update()
{
	CC_show::const_CC_sheet_iterator_t sht = mDoc->GetCurrentSheet();

	mContinuityChoices->Clear();
	for (auto& curranimcont : k_symbols)
	{
		if (sht->ContinuityInUse(curranimcont))
		{
			mContinuityChoices->Append(GetNameForSymbol(curranimcont));
		}
	}
	if (mCurrentContinuityChoice >= mContinuityChoices->GetCount() && mContinuityChoices->GetCount() > 0)
		mCurrentContinuityChoice = mContinuityChoices->GetCount()-1;
	mContinuityChoices->SetSelection(mCurrentContinuityChoice);
	UpdateText();
}


void ContinuityEditor::SetInsertionPoint(int x, int y)
{
	mUserInput->SetInsertionPoint(mUserInput->XYToPosition((long)x-1,(long)y-1));
	mUserInput->SetFocus();
}


SYMBOL_TYPE ContinuityEditor::CurrentSymbolChoice() const
{
	auto name = mContinuityChoices->GetString(mCurrentContinuityChoice);
	return GetSymbolForName(name.ToStdString());
}


void ContinuityEditor::UpdateText()
{
	mUserInput->Clear();
	CC_show::const_CC_sheet_iterator_t current_sheet = mDoc->GetCurrentSheet();
	const CC_continuity& c = current_sheet->GetContinuityBySymbol(CurrentSymbolChoice());
	if (!c.GetText().empty())
	{
		mUserInput->WriteText(c.GetText());
		mUserInput->SetInsertionPoint(0);
	}
	mUserInput->DiscardEdits();
	// disable the save and discard buttons as they are not active.
	wxButton* button = (wxButton*) FindWindow(ContinuityEditor_Save);
	button->Disable();
	button = (wxButton*) FindWindow(ContinuityEditor_Discard);
	button->Disable();
}


// flush out the text to the show.  This will treat the text box as unedited
// it is assumed that the user has already been notified that this will modify the show
void ContinuityEditor::FlushText()
{
	wxString conttext;

	conttext = mUserInput->GetValue();
	CC_show::const_CC_sheet_iterator_t current_sheet = mDoc->GetCurrentSheet();
	const CC_continuity& cont = current_sheet->GetContinuityBySymbol(CurrentSymbolChoice());
	if (conttext != cont.GetText())
	{
		mView->DoSetContinuityText(CurrentSymbolChoice(), conttext);
	}
	mUserInput->DiscardEdits();
}


void ContinuityEditor::SetCurrent(unsigned i)
{
	mCurrentContinuityChoice = i;
	mContinuityChoices->SetSelection(mCurrentContinuityChoice);
	UpdateText();
}


void ContinuityEditor::ContEditSelect(wxCommandEvent&)
{
	CC_show::const_CC_sheet_iterator_t sht = mDoc->GetCurrentSheet();
	mDoc->SetSelection(sht->SelectPointsBySymbol(CurrentSymbolChoice()));
}


void ContinuityEditor::OnSave(wxCommandEvent&)
{
	Save();
}


void ContinuityEditor::Save()
{
	FlushText();
}


void ContinuityEditor::OnDiscard(wxCommandEvent&)
{
	Discard();
}


void ContinuityEditor::Discard()
{
	UpdateText();
}


void ContinuityEditor::ContEditCurrent(wxCommandEvent&)
{
	// which value did we choose
	int newSelection = mContinuityChoices->GetSelection();
	// if the current field is modified, then do something
	if (mUserInput->IsModified())
	{
		// give the user a chance to save, discard, or cancle the action
		int userchoice = wxMessageBox(wxT("Continuity modified.  Save changes or cancel?"), wxT("Save changes?"), wxYES_NO|wxCANCEL);
		if (userchoice == wxYES)
		{
			Save();
		}
		if (userchoice == wxNO)
		{
			Discard();
		}
		if (userchoice == wxCANCEL)
		{
			mContinuityChoices->SetSelection(mCurrentContinuityChoice);
			return;
		}
	}
	SetCurrent(newSelection);
}


void ContinuityEditor::OnKeyPress(wxCommandEvent&)
{
	wxButton* button = (wxButton*) FindWindow(ContinuityEditor_Save);
	button->Enable();
	button = (wxButton*) FindWindow(ContinuityEditor_Discard);
	button->Enable();
}

