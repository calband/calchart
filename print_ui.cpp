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

BEGIN_EVENT_TABLE(ShowPrintDialog, wxDialog)
  EVT_CLOSE(ShowPrintDialog::OnCloseWindow)
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

static void ShowPrintOk(wxButton& button, wxEvent&) {
  wxString s;
#ifdef PRINT__RUN_CMD
  wxString buf;
#endif
  bool overview;
  long minyards;

  ShowPrintDialog* dialog = (ShowPrintDialog*)button.GetParent();

  double dval;
  dialog->text_x->GetValue().ToDouble(&dval);
  page_offset_x = (float)dval;
  dialog->text_y->GetValue().ToDouble(&dval);
  page_offset_y = (float)dval;
  dialog->text_width->GetValue().ToDouble(&dval);
  page_width = (float)dval;
  dialog->text_height->GetValue().ToDouble(&dval);
  page_height = (float)dval;
  dialog->text_minyards->GetValue().ToLong(&minyards);

  dialog->show_descr->show->GetBoolLandscape() =
    (dialog->radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);

  dialog->show_descr->show->GetBoolDoCont() = dialog->check_cont->GetValue();
  if (!dialog->eps)
    dialog->show_descr->show->GetBoolDoContSheet() =
      dialog->check_pages->GetValue();
  overview = dialog->check_overview->GetValue();

  switch (dialog->radio_method->GetSelection()) {
  case CC_PRINT_ACTION_PREVIEW:
#ifdef PRINT__RUN_CMD
    print_view_cmd = dialog->text_view_cmd->GetValue();
    print_view_opts = dialog->text_view_opts->GetValue();
    s = wxFileName::CreateTempFileName(wxT("cc_"));
    buf.sprintf(wxT("%s %s \"%s\""), print_view_cmd.c_str(), print_view_opts.c_str(), s.c_str());
#endif
    break;
  case CC_PRINT_ACTION_FILE:
    s = wxFileSelector(wxT("Print to file"), NULL, NULL, NULL,
		       dialog->eps ? wxT("*.eps"):wxT("*.ps"));
    if (s.empty()) return;
    break;
  case CC_PRINT_ACTION_PRINTER:
#ifdef PRINT__RUN_CMD
    print_cmd = dialog->text_cmd->GetValue();
    print_opts = dialog->text_opts->GetValue();
    s = wxFileName::CreateTempFileName(wxT("cc_"));
    buf.sprintf(wxT("%s %s \"%s\""), print_cmd.c_str(), print_opts.c_str(), s.c_str());
#else
    s = dialog->text_cmd->GetValue();
    print_file = s;
#endif
    break;
  default:
    break;
  }
  
  // Update other windows
  dialog->show_descr->show->winlist->ChangePrint(dialog);

  FILE *fp;

  fp = CC_fopen(s.fn_str(), "w");
  if (fp) {
    int n;
    wxString tempbuf;

    wxBeginBusyCursor();
    n = dialog->show_descr->show->Print(fp, dialog->eps, overview,
					dialog->show_descr->curr_ss, minyards);
    fflush(fp);
    fclose(fp);

#ifdef PRINT__RUN_CMD
    switch (dialog->radio_method->GetSelection()) {
    case CC_PRINT_ACTION_FILE:
      break;
    default:
      if (dialog->show_descr->show->Ok()) {
	system(buf.utf8_str());
      }
      wxRemoveFile(s);
      break;
    }
#endif
    wxEndBusyCursor();

    if (dialog->show_descr->show->Ok()) {
      tempbuf.sprintf(wxT("Printed %d pages."), n);
      (void)wxMessageBox(tempbuf,
			 dialog->show_descr->show->GetName());
    } else {
      (void)wxMessageBox(dialog->show_descr->show->GetError(),
			 dialog->show_descr->show->GetName());
    }
  } else {
    (void)wxMessageBox(wxT("Unable to open print file for writing!"),
		       dialog->show_descr->show->GetName());
  }
}

static void ShowPrintCancel(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  if (dialog->IsModal())
    dialog->EndModal();
  else
    dialog->Close();
}

static void ShowPrintSelect(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  (void)new StuntSheetPicker(dialog->show_descr->show, dialog->node->GetList(),
			     true, dialog->frame, wxT("Select stuntsheets"));
}

ShowPrintDialog::ShowPrintDialog(CC_descr *dcr, CC_WinList *lst,
				 bool printEPS, wxFrame *parent, const wxString& title,
				 bool isModal, int x, int y,
				 int width, int height):
wxDialog(parent, title, isModal, x, y, width, height),
show_descr(dcr), eps(printEPS), frame(parent) {
  wxString buf;

  node = new CC_WinNodePrint(lst, this);

  SetLabelPosition(wxVERTICAL);

  wxButton *okBut = new wxButton(this, (wxFunction)ShowPrintOk, wxT("&Print"));
  (void) new wxButton(this, (wxFunction)ShowPrintCancel, wxT("&Close"));
  NewLine();
  NewLine();
  okBut->SetDefault();

#ifdef PRINT__RUN_CMD
  text_cmd = new wxTextCtrl(this, (wxFunction)NULL, wxT("Printer Command: "),
			    print_cmd,
			    -1, -1, 100, -1);

  text_opts = new wxTextCtrl(this, (wxFunction)NULL, wxT("Printer Options: "),
			     print_opts,
			     -1, -1, 150, -1);
  NewLine();
  text_view_cmd = new wxTextCtrl(this, (wxFunction)NULL, wxT("Preview Command: "),
				 print_view_cmd,
				 -1, -1, 100, -1);

  text_view_opts = new wxTextCtrl(this, (wxFunction)NULL, wxT("Preview Options: "),
				  print_view_opts,
				  -1, -1, 150, -1);
  NewLine();
  NewLine();
#endif

#ifndef PRINT__RUN_CMD
  text_cmd = new wxTextCtrl(this, (wxFunction)NULL, wxT("Printer &Device: "),
			    print_file,
			    -1, -1, 100, -1);

  NewLine();
  NewLine();
#endif

  wxString orientation[2];
  orientation[0] = wxT("Portrait");
  orientation[1] = wxT("Landscape");
  radio_orient = new wxRadioBox(this, (wxFunction)NULL, wxT("&Orientation: "),
		    -1,-1,-1,-1,2,orientation,2,wxHORIZONTAL|wxFLAT);
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());

  wxString print_modes[3];
  print_modes[0] = wxT("Send to Printer");
  print_modes[1] = wxT("Print to File");
  print_modes[2] = wxT("Preview Only");
#ifdef PRINT__RUN_CMD
  int features = 3;
#else
  int features = 2;
#endif
  radio_method = new wxRadioBox(this, (wxFunction)NULL, wxT("Post&Script:"),
				-1,-1,-1,-1, features, print_modes,
				features, wxHORIZONTAL|wxFLAT);
  radio_method->SetSelection(0);
  
  NewLine();
  NewLine();

  check_overview = new wxCheckBox(this, (wxFunction)NULL, wxT("Over&view"));
  check_overview->SetValue(false);
  check_cont = new wxCheckBox(this, (wxFunction)NULL, wxT("Continuit&y"));
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
  if (!eps) {
    check_pages = new wxCheckBox(this, (wxFunction)NULL, wxT("Cove&r pages"));
    check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
  }

  if (!eps) {
    NewLine();
    (void) new wxButton(this, (wxFunction)ShowPrintSelect,wxT("S&elect sheets..."));
  }

  NewLine();
  NewLine();

  buf.sprintf(wxT("%.2f"), page_width);
  text_width = new wxTextCtrl(this, (wxFunction)NULL, wxT("Page &width: "),
			      buf, -1, -1, 100, -1);

  buf.sprintf(wxT("%.2f"), page_height);
  text_height = new wxTextCtrl(this, (wxFunction)NULL, wxT("Page &height: "),
			       buf, -1, -1, 100, -1);

  buf.sprintf(wxT("%.2f"), page_offset_x);
  text_x = new wxTextCtrl(this, (wxFunction)NULL, wxT("&Left margin: "),
			  buf, -1, -1, 100, -1);
  
  buf.sprintf(wxT("%.2f"), page_offset_y);
  text_y = new wxTextCtrl(this, (wxFunction)NULL, wxT("&Top margin: "),
			  buf, -1, -1, 100, -1);

  text_minyards = new wxTextCtrl(this, (wxFunction)NULL, wxT("Yards: "),
				 wxT("50"), -1, -1, 100, -1);

  NewLine();
  NewLine();

  Fit();

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

void ShowPrintDialog::Update() {
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
  if (!eps) check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
}
