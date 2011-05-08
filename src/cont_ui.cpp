/* cont_ui.cpp
 * Continuity editors
 *
 * Modification history:
 * 1-10-96    Garrick Meeker              Created
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "cont_ui.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "undo.h"

#include <wx/help.h>

extern wxFont *contPlainFont;
extern wxFont *contBoldFont;
extern wxFont *contItalFont;
extern wxFont *contBoldItalFont;

extern wxHelpControllerBase *help_inst;

enum
{
	ContinuityEditor_ContEditSet,
	ContinuityEditor_ContEditSelect,
	ContinuityEditor_ContEditSave,
	ContinuityEditor_ContEditCurrent,
};

BEGIN_EVENT_TABLE(ContinuityEditor, wxFrame)
EVT_CLOSE(ContinuityEditor::OnCloseWindow)
EVT_MENU(CALCHART__CONT_NEW, ContinuityEditor::OnCmdNew)
EVT_MENU(CALCHART__CONT_DELETE, ContinuityEditor::OnCmdDelete)
EVT_MENU(CALCHART__CONT_HELP, ContinuityEditor::OnCmdHelp)
EVT_BUTTON(ContinuityEditor_ContEditSet,ContinuityEditor::ContEditSet)
EVT_BUTTON(ContinuityEditor_ContEditSelect,ContinuityEditor::ContEditSelect)
EVT_BUTTON(ContinuityEditor_ContEditSave,ContinuityEditor::ContEditSave)
EVT_CHOICE(ContinuityEditor_ContEditCurrent,ContinuityEditor::ContEditCurrent)
END_EVENT_TABLE()

CC_WinNodeCont::CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodeCont::GotoSheet(unsigned)
{
	editor->Update();
}


void CC_WinNodeCont::GotoContLocation(unsigned, unsigned contnum,
int line, int col)
{
	editor->SetCurrent(contnum);
	editor->UpdateContChoice();
	if ((line > 0) && (col > 0))
	{
		editor->SetInsertionPoint(col, line);
	}
}


void CC_WinNodeCont::FlushContinuity()
{
	editor->FlushText();
}


void ContinuityEditor::ContEditSet(wxCommandEvent&)
{
	SetPoints();
}


void ContinuityEditor::ContEditSelect(wxCommandEvent&)
{
	SelectPoints();
}

void ContinuityEditor::ContEditSave(wxCommandEvent&)
{
	FlushText();
}

void ContinuityEditor::ContEditCurrent(wxCommandEvent&)
{
	SetCurrent(mContinuityChoices->GetSelection());
}


ContinuityEditorView::ContinuityEditorView() {}
ContinuityEditorView::~ContinuityEditorView() {}

void ContinuityEditorView::OnDraw(wxDC *dc) {}
void ContinuityEditorView::OnUpdate(wxView *sender, wxObject *hint)
{
	static_cast<ContinuityEditor*>(GetFrame())->Update();
}

void ContinuityEditorView::DoSetContinuityIndex(unsigned cont)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetContinuityIndexCommand(*static_cast<CC_show*>(GetDocument()), cont), true);
}

void ContinuityEditorView::DoSetNthContinuity(const wxString& text, unsigned i)
{
	GetDocument()->GetCommandProcessor()->Submit(new SetContinuityTextCommand(*static_cast<CC_show*>(GetDocument()), i, text), true);
}

void ContinuityEditorView::DoNewContinuity(const wxString& cont)
{
	GetDocument()->GetCommandProcessor()->Submit(new AddContinuityCommand(*static_cast<CC_show*>(GetDocument()), cont), true);
}

void ContinuityEditorView::DoDeleteContinuity(unsigned i)
{
	GetDocument()->GetCommandProcessor()->Submit(new RemoveContinuityCommand(*static_cast<CC_show*>(GetDocument()), i), true);
}

ContinuityEditor::ContinuityEditor(CC_show *show, CC_WinList *lst,
wxFrame *parent, const wxString& title,
int x, int y, int width, int height):
wxFrame(parent, -1, title, wxPoint(x, y), wxSize(width, height)),
mShow(show), mCurrentContinuityChoice(0), mSheetUnderEdit(NULL)
{
	mView = new ContinuityEditorView;
	mView->SetDocument(show);
	mView->SetFrame(this);
// Give it an icon
	SetBandIcon(this);

	CreateStatusBar();

	panel = new wxPanel(this);

// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

// add buttons to the top row
	wxBoxSizer *top_button_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton *button = new wxButton(panel, ContinuityEditor_ContEditSet, wxT("&Set Points"));
	top_button_sizer->Add(button, 0, wxALL, 5 );
	button = new wxButton(panel, ContinuityEditor_ContEditSelect, wxT("Select &Points"));
	top_button_sizer->Add(button, 0, wxALL, 5 );
	button = new wxButton(panel, ContinuityEditor_ContEditSave, wxT("Save &Edits"));
	top_button_sizer->Add(button, 0, wxALL, 5 );
	mContinuityChoices = new wxChoice(panel, ContinuityEditor_ContEditCurrent);
	top_button_sizer->Add(mContinuityChoices, 0, wxALL, 5 );
	topsizer->Add(top_button_sizer);

	mUserInput = new FancyTextWin(panel, -1, wxEmptyString, wxDefaultPosition, wxSize(50, 300));
	
	topsizer->Add(mUserInput, 1, wxEXPAND);
	panel->SetSizer( topsizer );
	topsizer->SetSizeHints( panel );

	wxMenu *cont_menu = new wxMenu;
	cont_menu->Append(CALCHART__CONT_NEW, wxT("&New\tCTRL-N"), wxT("Add new continuity"));
	cont_menu->Append(CALCHART__CONT_DELETE, wxT("&Delete\tCTRL-DEL"), wxT("Delete this continuity"));
	wxMenu *help_menu = new wxMenu;
	help_menu->Append(CALCHART__CONT_HELP, wxT("&Help on Continuity..."), wxT("Help on continuity commands"));
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(cont_menu, wxT("&Continuity"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);

	Show(true);

	Update();

	node = new CC_WinNodeCont(lst, this);
}


ContinuityEditor::~ContinuityEditor()
{
	delete mView;
	if (node)
	{
		node->Remove();
		delete node;
	}
}


void ContinuityEditor::OnCloseWindow(wxCloseEvent& event)
{
	FlushText();
	Destroy();
}

// Add a new continuity that we can modify.  It will be given an name and added to
// the list of names and the next available number
void ContinuityEditor::OnCmdNew(wxCommandEvent& event)
{
	wxString contname(wxGetTextFromUser(wxT("Enter the new continuity's name"),
		wxT("New Continuity"),
		wxT(""), this));
	if (!contname.empty())
	{
		mView->DoNewContinuity(contname);
	}
}


// remove a continuity.  Don't allow the user to delete a continuity that has dots associated with it.
void ContinuityEditor::OnCmdDelete(wxCommandEvent& event)
{
	CC_show::const_CC_sheet_iterator_t sht = mShow->GetCurrentSheet();
	if (sht->ContinuityInUse(mCurrentContinuityChoice))
	{
		(void)wxMessageBox(wxT("This continuity is being used.\nSet these points to a different continuity first."), wxT("Delete continuity"));
	}
	else
	{
		mView->DoDeleteContinuity(mCurrentContinuityChoice);
	}
}


void ContinuityEditor::OnCmdHelp(wxCommandEvent& event)
{
	help_inst->LoadFile();
	help_inst->KeywordSearch(wxT("Animation Commands"));
}


void ContinuityEditor::Update()
{
	CC_show::const_CC_sheet_iterator_t sht = mShow->GetCurrentSheet();

	mContinuityChoices->Clear();
	for (CC_sheet::ContContainer::const_iterator curranimcont = sht->animcont.begin(); curranimcont != sht->animcont.end();
		++curranimcont)
	{
		mContinuityChoices->Append(curranimcont->GetName());
	}
	UpdateContChoice();
	UpdateText();
}


void ContinuityEditor::UpdateContChoice()
{
	CC_show::const_CC_sheet_iterator_t sht = mShow->GetCurrentSheet();
	if (mCurrentContinuityChoice >= sht->animcont.size() && sht->animcont.size() > 0)
		mCurrentContinuityChoice = sht->animcont.size()-1;
	mContinuityChoices->SetSelection(mCurrentContinuityChoice);
}


void ContinuityEditor::UpdateText()
{
	mUserInput->Clear();
	mSheetUnderEdit = mShow->GetCurrentSheet();
	const CC_continuity& c = mSheetUnderEdit->GetNthContinuity(mCurrentContinuityChoice);
	if (c.GetText())
	{
		mUserInput->WriteText(c.GetText());
		mUserInput->SetInsertionPoint(0);
	}
}


void ContinuityEditor::FlushText()
{
	wxString conttext;

	if (mSheetUnderEdit != CC_show::const_CC_sheet_iterator_t(NULL))
	{
		conttext = mUserInput->GetValue();
		const CC_continuity& cont = mSheetUnderEdit->GetNthContinuity(mCurrentContinuityChoice);
		if (conttext != cont.GetText())
		{
			mView->DoSetNthContinuity(conttext, mCurrentContinuityChoice);
		}
	}
}


void ContinuityEditor::DetachText()
{
	mSheetUnderEdit = CC_show::const_CC_sheet_iterator_t(NULL);
}


void ContinuityEditor::SelectPoints()
{
	CC_show::const_CC_sheet_iterator_t sht = mShow->GetCurrentSheet();
	const CC_continuity& c = sht->GetNthContinuity(mCurrentContinuityChoice);
	sht->SelectContinuity(c.GetNum());
}


void ContinuityEditor::SetPoints()
{
	CC_show::const_CC_sheet_iterator_t sht = mShow->GetCurrentSheet();
	const CC_continuity& c = sht->GetNthContinuity(mCurrentContinuityChoice);
	mView->DoSetContinuityIndex(c.GetNum());
}

