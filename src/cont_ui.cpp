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
	ktoolbar_printcont_sym0 = 1000,
	ktoolbar_printcont_sym1,
	ktoolbar_printcont_sym2,
	ktoolbar_printcont_sym3,
	ktoolbar_printcont_sym4,
	ktoolbar_printcont_sym5,
	ktoolbar_printcont_sym6,
	ktoolbar_printcont_sym7,
};
enum
{
	ContinuityEditor_ContEditSet,
	ContinuityEditor_ContEditSelect,
	ContinuityEditor_ContEditSave,
	ContinuityEditor_ContEditCurrent,
};

ToolBarEntry printcont_tb[] =
{
	{ wxITEM_NORMAL, NULL, wxT("Insert plainman"), ktoolbar_printcont_sym0 },
	{ wxITEM_NORMAL, NULL, wxT("Insert solidman"), ktoolbar_printcont_sym1 },
	{ wxITEM_NORMAL, NULL, wxT("Insert backslash man"), ktoolbar_printcont_sym2 },
	{ wxITEM_NORMAL, NULL, wxT("Insert slash man"), ktoolbar_printcont_sym3 },
	{ wxITEM_NORMAL, NULL, wxT("Insert x man"), ktoolbar_printcont_sym4 },
	{ wxITEM_NORMAL, NULL, wxT("Insert solid backslash man"), ktoolbar_printcont_sym5 },
	{ wxITEM_NORMAL, NULL, wxT("Insert solid slash man"), ktoolbar_printcont_sym6 },
	{ wxITEM_NORMAL, NULL, wxT("Insert solid x man"), ktoolbar_printcont_sym7 }
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

BEGIN_EVENT_TABLE(PrintContCanvas, wxScrolledWindow)
EVT_CHAR(PrintContCanvas::OnChar)
EVT_MOUSE_EVENTS(PrintContCanvas::OnMouseEvent)
EVT_PAINT(PrintContCanvas::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(PrintContEditor, wxFrame)
EVT_ACTIVATE(PrintContEditor::OnActivate)
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


void CC_WinNodeCont::DeleteSheet(unsigned sht)
{
	if (sht == editor->GetShow()->GetCurrentSheetNum())
	{
		editor->DetachText();
	}
}


void CC_WinNodeCont::RemoveSheets(unsigned num)
{
	if (num <= editor->GetShow()->GetCurrentSheetNum())
	{
		editor->DetachText();
	}
}


void CC_WinNodeCont::FlushContinuity()
{
	editor->FlushText();
}


CC_WinNodePrintCont::CC_WinNodePrintCont(CC_WinList *lst,
PrintContEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodePrintCont::GotoSheet(unsigned)
{
	editor->canvas->Update();
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
	CC_sheet *sht = mShow->GetCurrentSheet();
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
	CC_sheet *sht = mShow->GetCurrentSheet();

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
	CC_sheet *sht = mShow->GetCurrentSheet();
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

	if (mSheetUnderEdit)
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
	mSheetUnderEdit = NULL;
}


void ContinuityEditor::SelectPoints()
{
	CC_sheet *sht = mShow->GetCurrentSheet();
	const CC_continuity& c = sht->GetNthContinuity(mCurrentContinuityChoice);
	sht->SelectContinuity(c.GetNum());
}


void ContinuityEditor::SetPoints()
{
	CC_sheet *sht = mShow->GetCurrentSheet();
	const CC_continuity& c = sht->GetNthContinuity(mCurrentContinuityChoice);
	mView->DoSetContinuityIndex(c.GetNum());
}


PrintContCanvas::PrintContCanvas(wxFrame *frame, CC_show *show):
wxScrolledWindow(frame), mShow(show), ourframe(frame),
topline(0), width(0), height(0), cursorx(0), cursory(0),
maxlines(0), maxcolumns(0)
{
}


PrintContCanvas::~PrintContCanvas() {}

void PrintContCanvas::Draw(wxDC *dc, int firstrow, int lastrow)
{
	const CC_sheet *sht = mShow->GetCurrentSheet();
	if (!sht)
		return;
	bool do_tab;
	unsigned row, column;
	float x, y;
	wxCoord textw, texth, textd, maxtexth;
	int devx, devy;
	unsigned tabnum;
	float tabw;
	float cur_posx, cur_posy, cur_height;
	wxString tmpstr;
	bool drawall;

	drawall = ((firstrow == 0) && (lastrow < 0));
	cur_posx = cur_posy = cur_height = 0;
	maxlines = maxcolumns = 0;

	if (drawall) dc->Clear();
	dc->SetTextForeground(*wxBLACK);

	width = 0;
	CC_textline_list::const_iterator cont(sht->continuity.begin());
	for (row = 0; (row < topline) && (cont != sht->continuity.end());
		row++, ++cont);
	y = 0.0;
	dc->SetFont(*contPlainFont);
	tabw = GetCharWidth() * 6;					  // Size of tab
	while (cont != sht->continuity.end())
	{
		CC_textchunk_list::const_iterator c;
		x = 0.0;
		column = 0;
		if (cont->center)
		{
			for (c = cont->chunks.begin();
				c != cont->chunks.end();
				++c)
			{
				do_tab = false;
				switch (c->font)
				{
					case PSFONT_SYMBOL:
						dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
						x += textw * c->text.length();
						break;
					case PSFONT_NORM:
						dc->SetFont(*contPlainFont);
						break;
					case PSFONT_BOLD:
						dc->SetFont(*contBoldFont);
						break;
					case PSFONT_ITAL:
						dc->SetFont(*contItalFont);
						break;
					case PSFONT_BOLDITAL:
						dc->SetFont(*contBoldItalFont);
						break;
					case PSFONT_TAB:
						do_tab = true;
						break;
				}
				if (!do_tab && (c->font != PSFONT_SYMBOL))
				{
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					x += textw;
				}
			}
			GetVirtualSize(&devx, &devy);
			x = (dc->DeviceToLogicalX(devx) - x) / 2;
			if (x < 0.0) x = 0.0;
		}
		maxtexth = contPlainFont->GetPointSize();
		tabnum = 0;
		for (c = cont->chunks.begin();
			c != cont->chunks.end();
			++c)
		{
			do_tab = false;
			switch (c->font)
			{
				case PSFONT_NORM:
				case PSFONT_SYMBOL:
					dc->SetFont(*contPlainFont);
					break;
				case PSFONT_BOLD:
					dc->SetFont(*contBoldFont);
					break;
				case PSFONT_ITAL:
					dc->SetFont(*contItalFont);
					break;
				case PSFONT_BOLDITAL:
					dc->SetFont(*contBoldItalFont);
					break;
				case PSFONT_TAB:
					if ((row == cursory) && (column <= cursorx))
					{
						cur_posx = x;
					}
					column++;
					tabnum++;
					textw = tabnum * tabw;
					if (textw >= x) x = textw;
					else x += tabw/6;
					do_tab = true;
					break;
				default:
					break;
			}
			if (c->font == PSFONT_SYMBOL)
			{
				dc->GetTextExtent(wxT("O"), &textw, &texth, &textd);
				float d = textw;
				SYMBOL_TYPE sym;

				float top_y = y + texth - textd - d;

				for (const wxChar *s = c->text; *s; s++, column++)
				{
					if ((row == cursory) && (column <= cursorx))
					{
						cur_posx = x;
					}

					if (drawall ||
						((row >= (unsigned)firstrow) && (lastrow >= 0) &&
						(row <= (unsigned)lastrow)))
					{
						dc->SetPen(*wxBLACK_PEN);
						sym = (SYMBOL_TYPE)(*s - 'A');
						switch (sym)
						{
							case SYMBOL_SOL:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->SetBrush(*wxBLACK_BRUSH);
								break;
							default:
								dc->SetBrush(*wxTRANSPARENT_BRUSH);
						}
						dc->DrawEllipse(x, top_y, d, d);
						switch (sym)
						{
							case SYMBOL_SL:
							case SYMBOL_X:
							case SYMBOL_SOLSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x, top_y + d, x + d, top_y);
								break;
							default:
								break;
						}
						switch (sym)
						{
							case SYMBOL_BKSL:
							case SYMBOL_X:
							case SYMBOL_SOLBKSL:
							case SYMBOL_SOLX:
								dc->DrawLine(x, top_y, x + d, top_y + d);
								break;
							default:
								break;
						}
					}
					x += d;
				}
				if (texth > maxtexth) maxtexth = texth;
			}
			else
			{
				if (!do_tab)
				{
					int len = (int)c->text.length();
					dc->GetTextExtent(c->text, &textw, &texth, &textd);
					if (texth > maxtexth) maxtexth = texth;
					if (drawall ||
						((row >= (unsigned)firstrow) && (lastrow >= 0) &&
						(row <= (unsigned)lastrow)))
						dc->DrawText(c->text, x, y);
					if (row == cursory)
					{
						if (column == cursorx)
						{
							cur_posx = x;
						}
						else if ((column + len) <= cursorx)
						{
							cur_posx = x+textw;
						}
						else if (column < cursorx)
						{
							float w;
							tmpstr = c->text.SubString(0, (cursorx - column - 1));
//dc->GetTextExtent(tmpstr, &w, &texth, &textd);
							cur_posx = x+w;
						}
					}
					x += textw;
					column += len;
				}
			}
		}
		if (row <= cursory)
		{
			cur_posy = y;
			cur_height = maxtexth;
			maxcolumns = column;
		}
		if (x > width) width = x;
		y += maxtexth;
		++cont;
		maxlines++;
		row++;
	}
	height = y;

	DrawCursor(dc, cur_posx, cur_posy, cur_height);

	dc->SetFont(wxNullFont);
}


void PrintContCanvas::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	DoPrepareDC(dc);
	dc.SetMapMode(MM_TEXT);
	dc.SetBackground(*wxWHITE_BRUSH);
	Draw(&dc);
}


void PrintContCanvas::OnMouseEvent(wxMouseEvent& /*event*/) {
}


void PrintContCanvas::OnChar(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_LEFT:
			if (cursorx > 0)
			{
				MoveCursor(MIN(maxcolumns-1,cursorx-1), cursory);
			}
			break;
		case WXK_RIGHT:
			if (cursorx < maxcolumns)
			{
				MoveCursor(cursorx+1, cursory);
			}
			break;
		case WXK_UP:
			if (cursory > 0)
			{
				MoveCursor(cursorx, cursory-1);
			}
			break;
		case WXK_DOWN:
			if (cursory < (maxlines-1))
			{
				MoveCursor(cursorx, cursory+1);
			}
			break;
		default:
			event.Skip();
			break;
	}
	Refresh();
}


void PrintContCanvas::UpdateBars()
{
	SetScrollbars(1, 1, (int)width, (int)height, 1, 1);
}


void PrintContCanvas::MoveCursor(unsigned column, unsigned row)
{
	cursorx = column;
	cursory = row;
//  OnPaint();
}


void PrintContCanvas::DrawCursor(wxDC *dc, float x, float y, float height)
{
	dc->SetPen(*wxRED_PEN);
	dc->DrawLine(x, y, x, y+height-1);
}


void PrintContCanvas::InsertChar(unsigned onechar)
{
}


void PrintContCanvas::DeleteChar(bool backspace)
{
}


void PrintContClose(wxButton& button, wxEvent&)
{
	((PrintContEditor*)(button.GetParent()->GetParent()))->Close();
}


PrintContEditorView::PrintContEditorView() {}
PrintContEditorView::~PrintContEditorView() {}

void PrintContEditorView::OnDraw(wxDC *dc) {}
void PrintContEditorView::OnUpdate(wxView *sender, wxObject *hint)
{
	static_cast<PrintContEditor*>(GetFrame())->canvas->Update();
}

PrintContEditor::PrintContEditor(CC_show *show, CC_WinList *lst,
wxFrame *parent, const wxString& title,
int x, int y, int width, int height)
: wxFrame(parent, -1, title, wxPoint(x, y), wxSize(width, height))
{
	mView = new PrintContEditorView;
	mView->SetDocument(show);
	mView->SetFrame(this);
// Give it an icon
	SetBandIcon(this);

	CreateStatusBar();

// Add a toolbar
	CreateCoolToolBar(printcont_tb, sizeof(printcont_tb)/sizeof(ToolBarEntry), this);

// Add the canvas
	canvas = new PrintContCanvas(this, show);
//  SetCanvas(canvas);

// Add the buttons
//  SetPanel(new wxPanel(this));
//  (void)new wxButton(framePanel, (wxFunction)PrintContClose, wxT("&Close"));

	node = new CC_WinNodePrintCont(lst, this);

	canvas->Update();

	Layout();
	Show(true);
}


PrintContEditor::~PrintContEditor()
{
	if (node)
	{
		node->Remove();
		delete node;
	}
}


void PrintContEditor::OnActivate(wxActivateEvent& event)
{
	if (event.GetActive())
	{
		canvas->SetFocus();
	}
}
