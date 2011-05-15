/* print_ui.cpp
 * Dialox box for printing
 *
 * Modification history:
 * 7-16-95    Garrick Meeker              Created
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

#include "print_ui.h"
#include "show_ui.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include <set>

#include <wx/filename.h>

enum
{
	CC_PRINT_ORIENT_PORTRAIT,
	CC_PRINT_ORIENT_LANDSCAPE
};

enum
{
	CC_PRINT_ACTION_PRINTER,
	CC_PRINT_ACTION_FILE,
	CC_PRINT_ACTION_PREVIEW
};

enum
{
	CC_PRINT_BUTTON_PRINT = 1000,
	CC_PRINT_BUTTON_SELECT,
};

BEGIN_EVENT_TABLE(ShowPrintDialog, wxDialog)
EVT_BUTTON(CC_PRINT_BUTTON_SELECT,ShowPrintDialog::ShowPrintSelect)
END_EVENT_TABLE()

void ShowPrintDialog::PrintShow()
{
	wxString s;
#ifdef PRINT__RUN_CMD
	wxString buf;
#endif
	bool overview;
	long minyards;

	double dval;
	text_x->GetValue().ToDouble(&dval);
	page_offset_x = (float)dval;
	text_y->GetValue().ToDouble(&dval);
	page_offset_y = (float)dval;
	text_width->GetValue().ToDouble(&dval);
	page_width = (float)dval;
	text_height->GetValue().ToDouble(&dval);
	page_height = (float)dval;
	text_minyards->GetValue().ToLong(&minyards);

	mShow->SetBoolLandscape(radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);

	mShow->SetBoolDoCont(check_cont->GetValue());
	if (!eps)
		mShow->SetBoolDoContSheet(check_pages->GetValue());
	overview = check_overview->GetValue();

	switch (radio_method->GetSelection())
	{
		case CC_PRINT_ACTION_PREVIEW:
#ifdef PRINT__RUN_CMD
			print_view_cmd = text_view_cmd->GetValue();
			print_view_opts = text_view_opts->GetValue();
			s = wxFileName::CreateTempFileName(wxT("cc_"));
			buf.sprintf(wxT("%s %s \"%s\""), print_view_cmd.c_str(), print_view_opts.c_str(), s.c_str());
#endif
			break;
		case CC_PRINT_ACTION_FILE:
			s = wxFileSelector(wxT("Print to file"), NULL, NULL, NULL,
				eps ? wxT("*.eps"):wxT("*.ps"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
			if (s.empty()) return;
			break;
		case CC_PRINT_ACTION_PRINTER:
#ifdef PRINT__RUN_CMD
			print_cmd = text_cmd->GetValue();
			print_opts = text_opts->GetValue();
			s = wxFileName::CreateTempFileName(wxT("cc_"));
			buf.sprintf(wxT("%s %s \"%s\""), print_cmd.c_str(), print_opts.c_str(), s.c_str());
#else
			s = text_cmd->GetValue();
			print_file = s;
#endif
			break;
		default:
			break;
	}

	FILE *fp;

	fp = CC_fopen(s.fn_str(), "w");
	if (fp)
	{
		int n;
		wxString tempbuf;

		wxBeginBusyCursor();
		n = mShow->Print(fp, eps, overview,
			mShow->GetCurrentSheetNum(), minyards);
		fflush(fp);
		fclose(fp);

#ifdef PRINT__RUN_CMD
		switch (radio_method->GetSelection())
		{
			case CC_PRINT_ACTION_FILE:
				break;
			default:
				if (mShow->Ok())
				{
					system(buf.utf8_str());
				}
				wxRemoveFile(s);
				break;
		}
#endif
		wxEndBusyCursor();

		if (mShow->Ok())
		{
			tempbuf.sprintf(wxT("Printed %d pages."), n);
			(void)wxMessageBox(tempbuf,
				mShow->GetTitle());
		}
		else
		{
			(void)wxMessageBox(mShow->GetError(),
				mShow->GetTitle());
		}
	}
	else
	{
		(void)wxMessageBox(wxT("Unable to open print file for writing!"),
			mShow->GetTitle());
	}
}


void ShowPrintDialog::ShowPrintSelect(wxCommandEvent&)
{
	wxArrayString choices;
	for (CC_show::const_CC_sheet_iterator_t sheet = mShow->GetSheetBegin(); sheet!=mShow->GetSheetEnd(); ++sheet)
	{
		choices.Add(sheet->GetName());
	}
	wxMultiChoiceDialog dialog(this,
		wxT("Choose which pages to print"),
		wxT("Pagest to Print"),
		choices);
	wxArrayInt markedChoices;
	size_t n = 0;
	for (CC_show::const_CC_sheet_iterator_t sheet = mShow->GetSheetBegin(); sheet!=mShow->GetSheetEnd(); ++sheet, ++n)
	{
		if (sheet->picked)
			markedChoices.Add(n);
	}
	dialog.SetSelections(markedChoices);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxArrayInt selections = dialog.GetSelections();
// build up a set of what's been selected:
		std::set<int> selected;
		for (n = 0; n < selections.GetCount(); ++n)
			selected.insert(selections[n]);
// now mark the sheets
		n = 0;
		for (CC_show::CC_sheet_iterator_t sheet = mShow->GetSheetBegin(); sheet!=mShow->GetSheetEnd(); ++sheet, ++n)
			sheet->picked = (selected.find(n) != selected.end());
	}
}


IMPLEMENT_CLASS( ShowPrintDialog, wxDialog )

ShowPrintDialog::ShowPrintDialog()
{
	Init();
}


ShowPrintDialog::ShowPrintDialog(CC_show *show, bool printEPS,
wxFrame *parent, wxWindowID id, const wxString& caption,
const wxPoint& pos, const wxSize& size,
long style)
{
	Init();

	Create(show, printEPS, parent, id, caption, pos, size, style);
}


ShowPrintDialog::~ShowPrintDialog()
{
}


void ShowPrintDialog::Init()
{
}


bool ShowPrintDialog::Create(CC_show *show, bool printEPS,
wxFrame *parent, wxWindowID id, const wxString& caption,
const wxPoint& pos, const wxSize& size,
long style)
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;
	mShow = show;
	eps = printEPS;

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


void ShowPrintDialog::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer( topsizer );

	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	wxButton *print = new wxButton(this, wxID_OK, wxT("&Print"));
	horizontalsizer->Add(print, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxButton *close = new wxButton(this, wxID_CANCEL, wxT("&Cancel"));
	horizontalsizer->Add(close, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	close->SetDefault();

	topsizer->Add(horizontalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer *verticalsizer = NULL;
#ifdef PRINT__RUN_CMD
	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Printer Command:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_cmd = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_cmd, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Printer Options:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_opts = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_opts, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Preview Command:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_view_cmd = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_view_cmd, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Preview Options:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_view_opts = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_view_opts, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

#else // PRINT__RUN_CMD

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Printer &Device:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_cmd = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_cmd, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );
#endif

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	wxString orientation[] = { wxT("Portrait"), wxT("Landscape") };
	radio_orient = new wxRadioBox(this, wxID_ANY, wxT("&Orientation:"),wxDefaultPosition,wxDefaultSize,
		sizeof(orientation)/sizeof(wxString),orientation);
	radio_orient->SetSelection((int)mShow->GetBoolLandscape());
	horizontalsizer->Add(radio_orient, 0, wxALL, 5 );

#ifdef PRINT__RUN_CMD
	wxString print_modes[] = { wxT("Send to Printer"), wxT("Print to File"), wxT("Preview Only") };
#else
	wxString print_modes[] = { wxT("Send to Printer"), wxT("Print to File") };
#endif
	radio_method = new wxRadioBox(this, -1, wxT("Post&Script:"),wxDefaultPosition,wxDefaultSize,
		sizeof(print_modes)/sizeof(wxString), print_modes);
	radio_method->SetSelection(0);
	horizontalsizer->Add(radio_method, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	check_overview = new wxCheckBox(this, -1, wxT("Over&view"));
	check_overview->SetValue(false);
	horizontalsizer->Add(check_overview, 0, wxALL, 5 );

	check_cont = new wxCheckBox(this, -1, wxT("Continuit&y"));
	check_cont->SetValue(mShow->GetBoolDoCont());
	horizontalsizer->Add(check_cont, 0, wxALL, 5 );

	if (!eps)
	{
		check_pages = new wxCheckBox(this, -1, wxT("Cove&r pages"));
		check_pages->SetValue(mShow->GetBoolDoContSheet());
		horizontalsizer->Add(check_pages, 0, wxALL, 5 );
	}
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

	if (!eps)
	{
		topsizer->Add(new wxButton(this, CC_PRINT_BUTTON_SELECT,wxT("S&elect sheets...")), 0, wxALL, 5 );
	}

	wxString buf;

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Page &width: "), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	buf.sprintf(wxT("%.2f"), page_width);
	text_width = new wxTextCtrl(this, wxID_ANY, buf);
	verticalsizer->Add(text_width, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Page &height: "), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	buf.sprintf(wxT("%.2f"), page_height);
	text_height = new wxTextCtrl(this, wxID_ANY, buf);
	verticalsizer->Add(text_height, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Left margin: "), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	buf.sprintf(wxT("%.2f"), page_offset_x);
	text_x = new wxTextCtrl(this, wxID_ANY, buf);
	verticalsizer->Add(text_x, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Top margin: "), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	buf.sprintf(wxT("%.2f"), page_offset_y);
	text_y = new wxTextCtrl(this, wxID_ANY, buf);
	verticalsizer->Add(text_y, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	verticalsizer = new wxBoxSizer( wxVERTICAL );
	verticalsizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Yards: "), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_LEFT|wxALL, 5);
	text_minyards = new wxTextCtrl(this, wxID_ANY);
	verticalsizer->Add(text_minyards, 0, wxGROW|wxALL, 5 );
	horizontalsizer->Add(verticalsizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	topsizer->Add(horizontalsizer, 0, wxALL, 5 );
}
