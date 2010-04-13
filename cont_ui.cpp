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

#include <wx/help.h>

extern wxFont *contPlainFont;
extern wxFont *contBoldFont;
extern wxFont *contItalFont;
extern wxFont *contBoldItalFont;

extern wxHelpControllerBase *help_inst;

enum
{
	ktoolbar_printcont_sym0,
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
EVT_SIZE(ContinuityEditor::OnSize)
EVT_MENU(CALCHART__CONT_NEW, ContinuityEditor::OnCmdNew)
EVT_MENU(CALCHART__CONT_DELETE, ContinuityEditor::OnCmdDelete)
EVT_MENU(CALCHART__CONT_CLOSE, ContinuityEditor::OnCmdClose)
EVT_MENU(CALCHART__CONT_HELP, ContinuityEditor::OnCmdHelp)
EVT_BUTTON(ContinuityEditor_ContEditSet,ContinuityEditor::ContEditSet)
EVT_BUTTON(ContinuityEditor_ContEditSelect,ContinuityEditor::ContEditSelect)
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

void CC_WinNodeCont::SetShow(CC_show *)
{
	editor->DetachText();
	editor->Update(true);
}


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
	if (sht == editor->GetShowDescr()->curr_ss)
	{
		editor->DetachText();
	}
}


void CC_WinNodeCont::RemoveSheets(unsigned num)
{
	if (num <= editor->GetShowDescr()->curr_ss)
	{
		editor->DetachText();
	}
}


void CC_WinNodeCont::AddContinuity(unsigned sht, unsigned cont)
{
	if (sht == editor->GetShowDescr()->curr_ss)
	{
		if (cont <= editor->GetCurrent())
		{
			editor->IncCurrent();
		}
		editor->Update();
	}
}


void CC_WinNodeCont::DeleteContinuity(unsigned sht, unsigned cont)
{
	if (sht == editor->GetShowDescr()->curr_ss)
	{
		if (cont == editor->GetCurrent()) editor->DetachText();
		if (editor->GetCurrent() > 0)
		{
			if (cont <= editor->GetCurrent())
			{
				editor->DecCurrent();
			}
		}
		editor->Update();
	}
}


void CC_WinNodeCont::FlushContinuity()
{
	editor->FlushText();
}


void CC_WinNodeCont::SetContinuity(wxWindow *win,
unsigned sht, unsigned cont)
{
	if ((win != editor) && (sht == editor->GetShowDescr()->curr_ss) &&
		(cont == editor->GetCurrent()))
	{
		editor->UpdateText(true);
	}
}


CC_WinNodePrintCont::CC_WinNodePrintCont(CC_WinList *lst,
PrintContEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodePrintCont::SetShow(CC_show *)
{
	editor->canvas->Update();
}


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


void ContinuityEditor::ContEditCurrent(wxCommandEvent&)
{
	SetCurrent(conts->GetSelection());
}


ContinuityEditor::ContinuityEditor(CC_descr *dcr, CC_WinList *lst,
wxFrame *parent, const wxString& title,
int x, int y, int width, int height):
wxFrame(parent, -1, title, wxPoint(x, y), wxSize(width, height)),
descr(dcr), curr_cont(0), text_sheet(NULL), text_contnum(0)
{
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
	conts = new wxChoice(panel, ContinuityEditor_ContEditCurrent);
	top_button_sizer->Add(conts, 0, wxALL, 5 );
	topsizer->Add(top_button_sizer);

	text = new FancyTextWin(panel, -1);
	topsizer->Add(text, 0, wxEXPAND);
	panel->SetSizer( topsizer );
	topsizer->SetSizeHints( panel );

	wxMenu *cont_menu = new wxMenu;
	cont_menu->Append(CALCHART__CONT_NEW, wxT("&New"), wxT("Add new continuity"));
	cont_menu->Append(CALCHART__CONT_DELETE, wxT("&Delete"), wxT("Delete this continuity"));
	cont_menu->Append(CALCHART__CONT_CLOSE, wxT("&Close window"), wxT("Close window"));
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
	if (node)
	{
		node->Remove();
		delete node;
	}
}


void ContinuityEditor::OnSize(wxSizeEvent& event)
{
	int width, height;
	int text_x, text_y;

	GetClientSize(&width, &height);
	panel->Fit();
	panel->GetSize(&text_x, &text_y);
	panel->SetSize(0, 0, width, text_y);
// this is done by the sizer
//  text->SetSize(0, text_y, width, height-text_y);
}


void ContinuityEditor::OnCloseWindow(wxCloseEvent& event)
{
	FlushText();
	Destroy();
}


void ContinuityEditor::OnCmdNew(wxCommandEvent& event)
{
	CC_sheet *sht = descr->CurrSheet();
	wxString contname(wxGetTextFromUser(wxT("Enter the new continuity's name"),
		wxT("New Continuity"),
		wxT(""), this));
	if (!contname.empty())
	{
		sht->UserNewContinuity(contname);
	}
}


void ContinuityEditor::OnCmdDelete(wxCommandEvent& event)
{
	CC_sheet *sht = descr->CurrSheet();
	if (sht->ContinuityInUse(curr_cont))
	{
		(void)wxMessageBox(wxT("This continuity is being used.\nSet these points to a different continuity first."), wxT("Delete continuity"));
	}
	else
	{
		sht->UserDeleteContinuity(curr_cont);
	}
}


void ContinuityEditor::OnCmdClose(wxCommandEvent& event)
{
	Close();
}


void ContinuityEditor::OnCmdHelp(wxCommandEvent& event)
{
	help_inst->LoadFile();
	help_inst->KeywordSearch(wxT("Animation Commands"));
}


void ContinuityEditor::Update(bool quick)
{
	CC_sheet *sht = descr->CurrSheet();
	CC_continuity *curranimcont;

	conts->Clear();
	for (curranimcont = sht->animcont; curranimcont != NULL;
		curranimcont = curranimcont->next)
	{
		conts->Append(curranimcont->name);
	}
	UpdateContChoice();
	UpdateText(quick);
}


void ContinuityEditor::UpdateContChoice()
{
	CC_sheet *sht = descr->CurrSheet();
	if (curr_cont >= sht->numanimcont && sht->numanimcont > 0)
		curr_cont = sht->numanimcont-1;
	conts->SetSelection(curr_cont);
}


void ContinuityEditor::UpdateText(bool quick)
{
	CC_sheet *sht = descr->CurrSheet();
	CC_continuity *c;

	if (quick)
	{
		c = sht->GetNthContinuity(curr_cont);
	}
	else
	{
		c = sht->UserGetNthContinuity(curr_cont);
	}
	text_sheet = sht;
	text_contnum = curr_cont;
	text->Clear();
	if (c != NULL)
	{
		if (c->text)
		{
			text->WriteText(c->text);
			text->SetInsertionPoint(0);
		}
	}
}


void ContinuityEditor::FlushText()
{
	wxString conttext;
	CC_continuity *cont;

	if (text_sheet)
	{
		cont = text_sheet->GetNthContinuity(text_contnum);
		if (cont != NULL)
		{
			conttext = text->GetValue();
			if (conttext != cont->text)
			{
				text_sheet->UserSetNthContinuity(conttext, text_contnum, this);
			}
		}
	}
}


void ContinuityEditor::SelectPoints()
{
	CC_sheet *sht = descr->CurrSheet();
	CC_continuity *c;

	c = sht->GetNthContinuity(curr_cont);
	if (c != NULL)
	{
		if (sht->SelectContinuity(c->num))
		{
			descr->show->winlist->UpdateSelections();
		}
	}
}


void ContinuityEditor::SetPoints()
{
	CC_sheet *sht = descr->CurrSheet();
	CC_continuity *c;

	c = sht->GetNthContinuity(curr_cont);
	if (c != NULL)
	{
		sht->SetContinuity(c->num);
	}
}


PrintContCanvas::PrintContCanvas(wxFrame *frame, CC_descr *dcr):
wxScrolledWindow(frame), show_descr(dcr), ourframe(frame),
topline(0), width(0), height(0), cursorx(0), cursory(0),
maxlines(0), maxcolumns(0)
{
}


PrintContCanvas::~PrintContCanvas() {}

void PrintContCanvas::Draw(wxDC *dc, int firstrow, int lastrow)
{
	const CC_sheet *sht = show_descr->CurrSheet();
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
	CC_textline_list::const_iterator cont(sht->continuity.lines.begin());
	for (row = 0; (row < topline) && (cont != sht->continuity.lines.end());
		row++, ++cont);
	y = 0.0;
	dc->SetFont(*contPlainFont);
	tabw = GetCharWidth() * 6;					  // Size of tab
	while (cont != sht->continuity.lines.end())
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


PrintContEditor::PrintContEditor(CC_descr *dcr, CC_WinList *lst,
wxFrame *parent, const wxString& title,
int x, int y, int width, int height)
: wxFrame(parent, -1, title, wxPoint(x, y), wxSize(width, height))
{
// Give it an icon
	SetBandIcon(this);

	CreateStatusBar();

// Add a toolbar
	CreateCoolToolBar(printcont_tb, sizeof(printcont_tb)/sizeof(ToolBarEntry), this);

// Add the canvas
	canvas = new PrintContCanvas(this, dcr);
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
