/* show_ui.cc
 * Classes for interacting with shows
 *
 * Modification history:
 * 8-7-95     Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "show_ui.h"
#include "modes.h"
#include <ctype.h>

extern ShowModeList *modelist;

CC_WinNodePicker::CC_WinNodePicker(CC_WinList *lst, StuntSheetPicker *req)
: CC_WinNode(lst), picker(req) {}

void CC_WinNodePicker::SetShow(CC_show *shw) {
  picker->show = shw;
  picker->Update();
}

void CC_WinNodePicker::AddSheet(unsigned) {
  picker->Update();
}

void CC_WinNodePicker::DeleteSheet(unsigned) {
  picker->Update();
}

void CC_WinNodePicker::AppendSheets() {
  picker->Update();
}

void CC_WinNodePicker::ChangeTitle(unsigned) {
  picker->Update();
}

void CC_WinNodePicker::SelectSheet(wxWindow *win, unsigned sht) {
  if (win != picker) {
    picker->Set(sht, picker->show->GetNthSheet(sht)->picked);
  }
}

CC_WinNodeInfo::CC_WinNodeInfo(CC_WinList *lst, ShowInfoReq *req)
: CC_WinNode(lst), inforeq(req) {}

void CC_WinNodeInfo::SetShow(CC_show *shw) {
  inforeq->Update(shw);
}

void CC_WinNodeInfo::ChangeNumPoints(wxWindow *win) {
  if (win != inforeq) {
     // make sure changes came from a different window
    inforeq->UpdateNumPoints();
  }
}

void CC_WinNodeInfo::ChangePointLabels(wxWindow *win) {
  if (win != inforeq) {
     // make sure changes came from a different window
    inforeq->UpdateLabels();
  }
}

void CC_WinNodeInfo::ChangeShowMode(wxWindow *win) {
  if (win != inforeq) {
     // make sure changes came from a different window
    inforeq->UpdateMode();
  }
}

static void SheetPickerClose(wxButton& button, wxEvent&) {
  ((StuntSheetPicker*)button.GetParent()->GetParent())->Close();
}

static void SheetPickerAll(wxButton& button, wxEvent&) {
  StuntSheetPicker* picker =
    (StuntSheetPicker*)button.GetParent()->GetParent();

  for (unsigned i=0; i < picker->show->GetNumSheets(); i++) {
    picker->Set(i, TRUE);
    picker->show->winlist->SelectSheet(picker, i);
  }
}

static void SheetPickerNone(wxButton& button, wxEvent&) {
  StuntSheetPicker* picker =
    (StuntSheetPicker*)button.GetParent()->GetParent();

  for (unsigned i=0; i < picker->show->GetNumSheets(); i++) {
    picker->Set(i, FALSE);
    picker->show->winlist->SelectSheet(picker, i);
  }
}

static void SheetPickerClick(wxListBox& list, wxCommandEvent&) {
  unsigned n;
  Bool sel;
  CC_sheet *sheet = NULL;

  StuntSheetPicker *picker = (StuntSheetPicker*)list.GetParent()->GetParent();
  for (n = 0, sheet = picker->show->GetSheet(); sheet != NULL;
       n++, sheet = sheet->next) {
    sel = picker->Get(n);
    if (sheet->picked != sel) {
      sheet->picked = sel;
      picker->show->winlist->SelectSheet(picker, n);
    }
  }
}

StuntSheetPicker::StuntSheetPicker(CC_show *shw, CC_WinList *lst,
				   Bool multi, wxFrame *frame,
				   char *title,
				   int x, int y, int width, int height):
wxFrame(frame, title, x, y, width, height, wxSDI | wxDEFAULT_FRAME),
show(shw) {
  panel = new wxPanel(this);

  wxButton *closeBut = new wxButton(panel, (wxFunction)SheetPickerClose,
				    "Close");
  if (multi) {
    (void)new wxButton(panel, (wxFunction)SheetPickerAll,
		       "All");
    (void)new wxButton(panel, (wxFunction)SheetPickerNone,
		       "None");
  }
  panel->NewLine();
  panel->NewLine();
  closeBut->SetDefault();

  list = new wxListBox(panel, (wxFunction)SheetPickerClick, "",
		       multi ? wxMULTIPLE:wxSINGLE,
		       -1, -1, -1, -1);
  SetListBoxEntries();

  node = new CC_WinNodePicker(lst, this);

  OnSize(-1,-1);

  Show(TRUE);
}

StuntSheetPicker::~StuntSheetPicker()
{
  if (node) {
    node->Remove();
    delete node;
  }
}

Bool StuntSheetPicker::OnClose(void) {
  return TRUE;
}

void StuntSheetPicker::OnSize(int, int) {
  int width, height;
  int list_x, list_y;

  GetClientSize(&width, &height);
  panel->SetSize(0, 0, width, height);
  list->GetPosition(&list_x, &list_y);
  list->SetSize(0, list_y, width, height-list_y);
}

void StuntSheetPicker::Update() {
  list->Clear();
  SetListBoxEntries();
}

void StuntSheetPicker::SetListBoxEntries() {
  CC_sheet *sheet;
  char **text;
  unsigned n;

  text = new char*[show->GetNumSheets()];
  for (n = 0, sheet = show->GetSheet(); sheet!=NULL; n++, sheet=sheet->next) {
    text[n] = (char *)sheet->GetName();
  }
  list->Set(show->GetNumSheets(), text);
  for (n = 0, sheet = show->GetSheet(); sheet!=NULL; n++, sheet=sheet->next) {
    list->SetSelection(n, sheet->picked);
  }
  delete text;
}

static void ShowInfoClose(wxButton& button, wxEvent&) {
  ((ShowInfoReq*)button.GetParent()->GetParent())->Close();
}

static void ShowInfoSetNum(wxText& text, wxEvent& ev) {
  unsigned num;
  ShowInfoReq *req = (ShowInfoReq *)text.GetParent()->GetParent();

  if (ev.eventType != wxEVENT_TYPE_TEXT_ENTER_COMMAND) return;
  num = req->GetNumPoints();
  if (num != req->show->GetNumPoints()) {
    if(wxMessageBox("Changing the number of points is not undoable.\nProceed?",
		    "Change number of points", wxOK|wxCANCEL, req) == wxOK) {
      if (num > req->show->GetNumPoints())
	req->show->SetNumPoints(num, req->GetColumns());
      else
	req->show->SetNumPoints(num, 1);
      req->SetLabels();
      req->show->winlist->ChangeNumPoints(req);
    }
  }
}

static void ShowInfoSetLabels(wxButton& button, wxEvent&) {
  ShowInfoReq *req = (ShowInfoReq*)button.GetParent()->GetParent();

  req->SetLabels();
  req->show->winlist->ChangePointLabels(req);
}

static void ShowInfoModeChoice(wxChoice& choice, wxCommandEvent&) {
  ShowMode *newmode;
  ShowInfoReq *req = (ShowInfoReq *)choice.GetParent()->GetParent();

  newmode = modelist->Find(req->GetChoiceStrSelection());
  if (newmode) {
    req->show->mode = newmode;
    req->show->winlist->ChangeShowMode(req);
  }
}

static void CalculateLabels(CC_show *show, Bool letters[26],
			    Bool& use_letters, int& maxnum) {
  unsigned i;
  char *tmp;

  for (i = 0; i < 26; i++) letters[i] = FALSE;
  use_letters = FALSE;
  maxnum = 1;
  for (i = 0; i < show->GetNumPoints(); i++) {
    tmp = show->GetPointLabel(i);
    if (!isdigit(tmp[0])) {
      letters[tmp[0]-'A'] = TRUE;
      if ((tmp[1]-'0') >= maxnum)
	maxnum = tmp[1]-'0'+1;
      use_letters = TRUE;
    }
  }
  if (use_letters == FALSE) {
    maxnum = 10;
    for (i = 0; i < 26; i++) letters[i] = TRUE;
  }

}

ShowInfoReq::ShowInfoReq(CC_show *shw, CC_WinList *lst,
			 wxFrame *frame, char *title,
			 int x, int y, int width, int height):
wxFrame(frame, title, x, y, width, height, wxSDI | wxDEFAULT_FRAME),
show(shw) {
  unsigned i;
  char buf[16];
  char *strs[2];
  Bool letters[26];
  Bool use_letters;
  int maxnum;

  CalculateLabels(show, letters, use_letters, maxnum);

  panel = new wxPanel(this);

  wxButton *closeBut = new wxButton(panel, (wxFunction)ShowInfoClose,
				    "Close");
  (void)new wxButton(panel, (wxFunction)ShowInfoSetLabels,
		     "Set Labels");
  panel->NewLine();
  panel->NewLine();
  closeBut->SetDefault();

  sprintf(buf, "%u", show->GetNumPoints());
  numpnts = new wxText(panel, (wxFunction)ShowInfoSetNum, "Points", buf,
		       -1, -1, 100, -1, wxPROCESS_ENTER);
  strs[0] = "Numbers";
  strs[1] = "Letters";
  label_type = new wxRadioBox(panel, (wxFunction)NULL, "Labels",
			      -1, -1, -1, -1, 2, strs, 2, wxHORIZONTAL|wxFLAT);
  label_type->SetSelection(use_letters);
  panel->NewLine();
  lettersize = new wxSlider(panel, (wxFunction)NULL, "Points per letter",
			    maxnum, 1, 10, 150);
  panel->NewLine();
  labels = new wxListBox(panel, (wxFunction)NULL, "Letters",
			 wxMULTIPLE | wxALWAYS_SB, -1, -1, 150, 100);
  buf[1] = '\0';
  for (i = 0; i < 26; i++) {
    buf[0] = i + 'A';
    labels->Append(buf);
    labels->SetSelection(i, letters[i]);
  }
  panel->NewLine();
  panel->NewLine();

  choice = new wxChoice(panel, (wxFunction)ShowInfoModeChoice, "Show mode");
  ShowMode *mode = modelist->First();
  while (mode != NULL) {
    choice->Append(mode->Name());
    mode = mode->next;
  }
  UpdateMode();

  text = new FancyTextWin(this, -1, -1, -1, -1, wxNATIVE_IMPL);

  node = new CC_WinNodeInfo(lst, this);

  OnSize(-1,-1);

  Show(TRUE);
}

ShowInfoReq::~ShowInfoReq()
{
  if (node) {
    node->Remove();
    delete node;
  }
}

Bool ShowInfoReq::OnClose(void) {
  return TRUE;
}

void ShowInfoReq::OnSize(int, int) {
  int width, height;
  int text_x, text_y;

  GetClientSize(&width, &height);
  panel->Fit();
  panel->GetSize(&text_x, &text_y);
  panel->SetSize(0, 0, width, text_y);
  text->SetSize(0, text_y, width, height-text_y);
}

void ShowInfoReq::UpdateLabels() {
  int i;
  Bool letters[26];
  Bool use_letters;
  int maxnum;

  CalculateLabels(show, letters, use_letters, maxnum);

  label_type->SetSelection(use_letters);
  lettersize->SetValue(maxnum);
  for (i = 0; i < 26; i++) {
    labels->SetSelection(i, letters[i]);
  }
}

void ShowInfoReq::UpdateNumPoints() {
  char buf[16];

  sprintf(buf, "%u", show->GetNumPoints());
  numpnts->SetValue(buf);
  UpdateLabels();
}

void ShowInfoReq::UpdateMode() {
  choice->SetStringSelection(show->mode->Name());
}

void ShowInfoReq::Update(CC_show *shw) {
  if (shw != NULL) show = shw;
  UpdateMode();
  UpdateNumPoints();
}

unsigned ShowInfoReq::GetNumPoints() {
  int i;
  char buf[16];

  StringToInt(numpnts->GetValue(), &i);
  if (i < 0) {
    numpnts->SetValue("0");
    i = 0;
  }
  if (i > MAX_POINTS) {
    sprintf(buf, "%u", MAX_POINTS);
    numpnts->SetValue(buf);
    i = MAX_POINTS;
  }
  return (unsigned)i;
}

unsigned ShowInfoReq::GetColumns() {
  char *s;
  int i;

  s = wxGetTextFromUser("Enter the number of columns for new points",
			"New points", "10", this);
  if (s) {
    StringToInt(s, &i);
  } else {
    i = 10;
  }
  if (i < 1) {
    i = 1;
  }
  return (unsigned)i;
}

void ShowInfoReq::SetLabels() {
  unsigned i, j, num, numlabels, letr;
  Bool letters[26];

  if (GetLabelType() == 0) {
    // Numbers
    for (i = 0; i < show->GetNumPoints(); i++) {
      // Begin with 1, not 0
      sprintf(show->GetPointLabel(i), "%u", i+1);
    }
  } else {
    // Letters
    num = GetLetterSize();
    numlabels = 0;
    for (i = 0; i < 26; i++) {
      if (GetLetter(i)) {
	letters[i] = TRUE;
	numlabels++;
      } else {
	letters[i] = FALSE;
      }
    }
    if (num*numlabels < show->GetNumPoints()) {
      wxMessageBox("There are not enough labels defined.", "Set labels");
    } else {
      j = 0;
      letr = 0;
      while (!letters[letr]) letr++;
      for (i = 0; i < show->GetNumPoints(); i++, j++) {
	if (j >= num) {
	  j = 0;
	  do {
	    letr++;
	  } while (!letters[letr]);
	}
	sprintf(show->GetPointLabel(i), "%c%u", letr+'A', j);
      }
    }
  }
}
