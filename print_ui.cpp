/* print_ui.cc
 * Dialox box for printing
 *
 * Modification history:
 * 7-16-95    Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "print_ui.h"
#include "show_ui.h"
#include "confgr.h"

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
  char *s;
  char buf[256];
  Bool overview;

  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();

  StringToFloat(dialog->text_x->GetValue(), &page_offset_x);
  StringToFloat(dialog->text_y->GetValue(), &page_offset_y);
  StringToFloat(dialog->text_width->GetValue(), &page_width);
  StringToFloat(dialog->text_height->GetValue(), &page_height);

  dialog->show_descr->show->GetBoolLandscape() =
    (dialog->radio_orient->GetSelection() == PS_LANDSCAPE);

  dialog->show_descr->show->GetBoolDoCont() = dialog->check_cont->GetValue();
  if (!dialog->eps)
    dialog->show_descr->show->GetBoolDoContSheet() =
      dialog->check_pages->GetValue();
  overview = dialog->check_overview->GetValue();

  switch (dialog->radio_method->GetSelection()) {
  case PS_PREVIEW:
#ifdef PRINT__RUN_CMD
    strcpy(print_view_cmd, dialog->text_view_cmd->GetValue());
    strcpy(print_view_opts, dialog->text_view_opts->GetValue());
    s = tmpnam(NULL);
    sprintf(buf, "%s %s %s", print_view_cmd, print_view_opts, s);
#endif
    break;
  case PS_FILE:
    s = wxFileSelector("Print to file", NULL, NULL, NULL,
		       dialog->eps ? "*.eps":"*.ps");
    if (!s) return;
    break;
  case PS_PRINTER:
#ifdef PRINT__RUN_CMD
    strcpy(print_cmd, dialog->text_cmd->GetValue());
    strcpy(print_opts, dialog->text_opts->GetValue());
    s = tmpnam(NULL);
    sprintf(buf, "%s %s %s", print_cmd, print_opts, s);
#else
    strcpy(print_file, dialog->text_cmd->GetValue());
    s = print_file;
#endif
    break;
  default:
    s = NULL;
  }
  
  // Update other windows
  dialog->show_descr->show->winlist->ChangePrint(dialog);

  FILE *fp;

  fp = fopen(s, "w");
  if (fp) {
    int n;
    char tempbuf[32];

    wxBeginBusyCursor();
    n = dialog->show_descr->show->Print(fp, dialog->eps, overview,
					dialog->show_descr->curr_ss);
    fflush(fp);
    fclose(fp);

#ifdef PRINT__RUN_CMD
    switch (dialog->radio_method->GetSelection()) {
    case PS_FILE:
      break;
    default:
      if (dialog->show_descr->show->Ok()) {
	system(buf);
      }
      wxRemoveFile(s);
      break;
    }
#endif
    wxEndBusyCursor();

    if (dialog->show_descr->show->Ok()) {
      sprintf(tempbuf, "Printed %d pages.", n);
      (void)wxMessageBox(tempbuf, (char *)dialog->show_descr->show->GetName());
    } else {
      (void)wxMessageBox(dialog->show_descr->show->GetError(),
			 (char *)dialog->show_descr->show->GetName());
    }
  } else {
    (void)wxMessageBox("Unable to open print file for writing!",
		       (char *)dialog->show_descr->show->GetName());
  }
}

static void ShowPrintCancel(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  dialog->Close();
}

static void ShowPrintSelect(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  (void)new StuntSheetPicker(dialog->show_descr->show, dialog->node->GetList(),
			     TRUE, dialog->frame, "Select stuntsheets");
}

ShowPrintDialog::ShowPrintDialog(CC_descr *dcr, CC_WinList *lst,
				 Bool printEPS, wxFrame *parent, char *title,
				 Bool isModal, int x, int y,
				 int width, int height):
wxDialogBox(parent, title, isModal, x, y, width, height),
show_descr(dcr), eps(printEPS), frame(parent) {
  char buf[16];

  node = new CC_WinNodePrint(lst, this);

  SetLabelPosition(wxVERTICAL);

  wxButton *okBut = new wxButton(this, (wxFunction)ShowPrintOk, "&Print");
  (void) new wxButton(this, (wxFunction)ShowPrintCancel, "&Close");
  NewLine();
  NewLine();
  okBut->SetDefault();

#ifdef PRINT__RUN_CMD
  text_cmd = new wxText(this, (wxFunction)NULL, "Printer Command: ",
			print_cmd,
			-1, -1, 100, -1);

  text_opts = new wxText(this, (wxFunction)NULL, "Printer Options: ",
			 print_opts,
			 -1, -1, 150, -1);
  NewLine();
  text_view_cmd = new wxText(this, (wxFunction)NULL, "Preview Command: ",
			     print_view_cmd,
			     -1, -1, 100, -1);

  text_view_opts = new wxText(this, (wxFunction)NULL, "Preview Options: ",
			      print_view_opts,
			      -1, -1, 150, -1);
  NewLine();
  NewLine();
#endif

#ifndef PRINT__RUN_CMD
  text_cmd = new wxText(this, (wxFunction)NULL, "Printer &Device: ",
			print_file,
			-1, -1, 100, -1);

  NewLine();
  NewLine();
#endif

  char *orientation[2];
  orientation[0] = "Portrait";
  orientation[1] = "Landscape";
  radio_orient = new wxRadioBox(this, (wxFunction)NULL, "&Orientation: ",
		    -1,-1,-1,-1,2,orientation,2,wxHORIZONTAL|wxFLAT);
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());

  char *print_modes[3];
  print_modes[0] = "Send to Printer";
  print_modes[1] = "Print to File";
  print_modes[2] = "Preview Only";
#ifdef PRINT__RUN_CMD
  int features = 3;
#else
  int features = 2;
#endif
  radio_method = new wxRadioBox(this, (wxFunction)NULL, "Post&Script:",
				-1,-1,-1,-1, features, print_modes,
				features, wxHORIZONTAL|wxFLAT);
  radio_method->SetSelection(0);
  
  NewLine();
  NewLine();

  check_overview = new wxCheckBox(this, (wxFunction)NULL, "Over&view");
  check_overview->SetValue(FALSE);
  check_cont = new wxCheckBox(this, (wxFunction)NULL, "Continuit&y");
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
  if (!eps) {
    check_pages = new wxCheckBox(this, (wxFunction)NULL, "Cove&r pages");
    check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
  }

  if (!eps) {
    NewLine();
    (void) new wxButton(this, (wxFunction)ShowPrintSelect,"S&elect sheets...");
  }

  NewLine();
  NewLine();

  sprintf(buf, "%.2f", page_width);
  text_width = new wxText(this, (wxFunction)NULL, "Page &width: ",
		      buf, -1, -1, 100, -1);

  sprintf(buf, "%.2f", page_height);
  text_height = new wxText(this, (wxFunction)NULL, "Page &height: ",
		      buf, -1, -1, 100, -1);

  sprintf(buf, "%.2f", page_offset_x);
  text_x = new wxText(this, (wxFunction)NULL, "&Left margin: ",
			  buf, -1, -1, 100, -1);

  sprintf(buf, "%.2f", page_offset_y);
  text_y = new wxText(this, (wxFunction)NULL, "&Top margin: ",
			   buf, -1, -1, 100, -1);

  NewLine();
  NewLine();

  Fit();

  Show(TRUE);
}

ShowPrintDialog::~ShowPrintDialog() {
  if (node) {
    node->Remove();
    delete node;
  }
}

Bool ShowPrintDialog::OnClose(void) {
  return TRUE;
}

void ShowPrintDialog::Update() {
  radio_orient->SetSelection((int)show_descr->show->GetBoolLandscape());
  check_cont->SetValue(show_descr->show->GetBoolDoCont());
  if (!eps) check_pages->SetValue(show_descr->show->GetBoolDoContSheet());
}
