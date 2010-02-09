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

#include <wx/filename.h>

enum {
  CC_PRINT_ORIENT_PORTRAIT,
  CC_PRINT_ORIENT_LANDSCAPE
};

enum {
  CC_PRINT_ACTION_PRINTER,
  CC_PRINT_ACTION_FILE,
  CC_PRINT_ACTION_PREVIEW
};

enum {
  CC_PRINT_BUTTON_PRINT = 1000,
  CC_PRINT_BUTTON_CLOSE,
  CC_PRINT_BUTTON_SELECT,
};

BEGIN_EVENT_TABLE(ShowPrintDialog, wxDialog)
  EVT_CLOSE(ShowPrintDialog::OnCloseWindow)
  EVT_BUTTON(CC_PRINT_BUTTON_CLOSE,ShowPrintDialog::ShowPrintClose)
  EVT_BUTTON(CC_PRINT_BUTTON_PRINT,ShowPrintDialog::ShowPrintOk)
  EVT_BUTTON(CC_PRINT_BUTTON_SELECT,ShowPrintDialog::ShowPrintSelect)
END_EVENT_TABLE()

CC_WinNodePrint::CC_WinNodePrint(CC_WinList *lst, ShowPrintDialog *req)
: CC_WinNode(lst), printreq(req) {
}

void CC_WinNodePrint::SetShow(CC_show *) {
  printreq->Update();
}

void CC_WinNodePrint::ChangePrint(wxWindow* win) {
  if (win != printreq) {
    // Make sure a different window made the changes
    printreq->Update();
  }
}

void ShowPrintDialog::ShowPrintOk(wxCommandEvent&) {
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

  show_descr->show->GetBoolLandscape() =
    (radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);

  show_descr->show->GetBoolDoCont() = check_cont->GetValue();
  if (!eps)
    show_descr->show->GetBoolDoContSheet() =
      check_pages->GetValue();
  overview = check_overview->GetValue();

  switch (radio_method->GetSelection()) {
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
		       eps ? wxT("*.eps"):wxT("*.ps"));
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
  
  // Update other windows
  show_descr->show->winlist->ChangePrint(this);

  FILE *fp;

  fp = CC_fopen(s.fn_str(), "w");
  if (fp) {
    int n;
    wxString tempbuf;

    wxBeginBusyCursor();
    n = show_descr->show->Print(fp, eps, overview,
					show_descr->curr_ss, minyards);
    fflush(fp);
    fclose(fp);

#ifdef PRINT__RUN_CMD
    switch (radio_method->GetSelection()) {
    case CC_PRINT_ACTION_FILE:
      break;
    default:
      if (show_descr->show->Ok()) {
	system(buf.utf8_str());
      }
      wxRemoveFile(s);
      break;
    }
#endif
    wxEndBusyCursor();

    if (show_descr->show->Ok()) {
      tempbuf.sprintf(wxT("Printed %d pages."), n);
      (void)wxMessageBox(tempbuf,
			 show_descr->show->GetName());
    } else {
      (void)wxMessageBox(show_descr->show->GetError(),
			 show_descr->show->GetName());
    }
  } else {
    (void)wxMessageBox(wxT("Unable to open print file for writing!"),
		       show_descr->show->GetName());
  }
}

void ShowPrintDialog::ShowPrintSelect(wxCommandEvent&) {
  (void)new StuntSheetPicker(show_descr->show, node->GetList(),
			     true, frame, wxT("Select stuntsheets"));
}

ShowPrintDialog::ShowPrintDialog(CC_descr *dcr, CC_WinList *lst,
				 bool printEPS, wxFrame *parent, const wxString& title,
				 bool isModal, int x, int y,
				 int width, int height):
wxDialog(parent, -1, title, wxPoint(x, y), wxSize(width, height)),
show_descr(dcr), eps(printEPS), frame(parent) {
  wxString buf;

  node = new CC_WinNodePrint(lst, this);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *horizontalsizer = new wxBoxSizer( wxHORIZONTAL );

	wxButton *button = new wxButton(this, CC_PRINT_BUTTON_PRINT, wxT("&Print"));
	horizontalsizer->Add(button, 0, wxALL, 5 );
	button = new wxButton(this, CC_PRINT_BUTTON_CLOSE, wxT("&Close"));
	button->SetDefault();
	horizontalsizer->Add(button, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

#ifdef PRINT__RUN_CMD
	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
  text_cmd = new wxTextCtrl(this, -1, wxT("Printer Command: "));
	horizontalsizer->Add(text_cmd, 0, wxALL, 5 );
  text_opts = new wxTextCtrl(this, -1, wxT("Printer Options: "));
	horizontalsizer->Add(text_opts, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );
	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
  text_view_cmd = new wxTextCtrl(this, -1, wxT("Preview Command: "));
	horizontalsizer->Add(text_view_cmd, 0, wxALL, 5 );
  text_view_opts = new wxTextCtrl(this, -1, wxT("Preview Options: "));
	horizontalsizer->Add(text_view_opts, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );
#endif

#ifndef PRINT__RUN_CMD
	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
  text_cmd = new wxTextCtrl(this, (wxFunction)NULL, wxT("Printer &Device: "),
			    print_file,
			    -1, -1, 100, -1);
	horizontalsizer->Add(text_cmd, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );
#endif

  wxString orientation[2];
  orientation[0] = wxT("Portrait");
  orientation[1] = wxT("Landscape");
  radio_orient = new wxRadioBox(this, -1, wxT("&Orientation: "),
		    wxPoint(-1,-1),wxSize(-1,-1),2,orientation,2);
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());
	topsizer->Add(radio_orient, 0, wxALL, 5 );

  wxString print_modes[3];
  print_modes[0] = wxT("Send to Printer");
  print_modes[1] = wxT("Print to File");
  print_modes[2] = wxT("Preview Only");
#ifdef PRINT__RUN_CMD
  int features = 3;
#else
  int features = 2;
#endif
  radio_method = new wxRadioBox(this, -1, wxT("Post&Script:"),
				wxPoint(-1,-1),wxSize(-1,-1), features, print_modes,
				features);
  radio_method->SetSelection(0);
	topsizer->Add(radio_method, 0, wxALL, 5 );

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
  check_overview = new wxCheckBox(this, -1, wxT("Over&view"));
  check_overview->SetValue(false);
	horizontalsizer->Add(check_overview, 0, wxALL, 5 );

  check_cont = new wxCheckBox(this, -1, wxT("Continuit&y"));
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
	horizontalsizer->Add(check_cont, 0, wxALL, 5 );

  if (!eps) {
    check_pages = new wxCheckBox(this, -1, wxT("Cove&r pages"));
    check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
	horizontalsizer->Add(check_pages, 0, wxALL, 5 );
  }
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

  if (!eps) {
    button = new wxButton(this, CC_PRINT_BUTTON_SELECT,wxT("S&elect sheets..."));
	topsizer->Add(button, 0, wxALL, 5 );
  }

	horizontalsizer = new wxBoxSizer( wxHORIZONTAL );
  buf.sprintf(wxT("%.2f"), page_width);
  text_width = new wxTextCtrl(this, -1, wxT("Page &width: "));
	horizontalsizer->Add(text_width, 0, wxALL, 5 );

  buf.sprintf(wxT("%.2f"), page_height);
  text_height = new wxTextCtrl(this, -1, wxT("Page &height: "));
	horizontalsizer->Add(text_height, 0, wxALL, 5 );

  buf.sprintf(wxT("%.2f"), page_offset_x);
  text_x = new wxTextCtrl(this, -1, wxT("&Left margin: "));
	horizontalsizer->Add(text_x, 0, wxALL, 5 );
  
  buf.sprintf(wxT("%.2f"), page_offset_y);
  text_y = new wxTextCtrl(this, -1, wxT("&Top margin: "));
	horizontalsizer->Add(text_y, 0, wxALL, 5 );

  text_minyards = new wxTextCtrl(this, -1, wxT("Yards: "));
	horizontalsizer->Add(text_minyards, 0, wxALL, 5 );
	topsizer->Add(horizontalsizer, 0, wxALL, 5 );

	SetSizer( topsizer );

  if (isModal)
    ShowModal();
  else
    Show(true);
}

ShowPrintDialog::~ShowPrintDialog() {
  if (node) {
    node->Remove();
    delete node;
  }
}

void ShowPrintDialog::OnCloseWindow(wxCloseEvent& event) {
  Destroy();
}

void ShowPrintDialog::ShowPrintClose(wxCommandEvent& event) {
  Close();
}

void ShowPrintDialog::Update() {
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
  if (!eps) check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
}
