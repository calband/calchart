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

void CC_WinNodePicker::RemoveSheets(unsigned) {
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

CC_WinNodePointPicker::CC_WinNodePointPicker(CC_WinList *lst,
					     PointPicker *req)
: CC_WinNode(lst), picker(req) {}

void CC_WinNodePointPicker::SetShow(CC_show *shw) {
  picker->show = shw;
  picker->Update();
}

void CC_WinNodePointPicker::ChangeNumPoints(wxWindow*) {
  picker->Update();
}

void CC_WinNodePointPicker::ChangePointLabels(wxWindow*) {
  picker->Update();
}

void CC_WinNodePointPicker::UpdateSelections(wxWindow *win, int point) {
  if (win != picker) {
    if (point < 0) {
      picker->UpdateSelections();
    } else {
      picker->Set(point, picker->show->IsSelected(point));
    }
  }
}

CC_WinNodeInfo::CC_WinNodeInfo(CC_WinList *lst, ShowInfoReq *req)
: CC_WinNode(lst), inforeq(req) {}

void CC_WinNodeInfo::SetShow(CC_show *shw) {
  inforeq->Update(TRUE, shw);
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

void CC_WinNodeInfo::FlushDescr() {
  inforeq->FlushDescr();
}

void CC_WinNodeInfo::SetDescr(wxWindow* win) {
  if (win != inforeq) {
    inforeq->UpdateDescr(TRUE);
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
wxFrame(frame, title, x, y, width, height, CC_FRAME_OTHER),
show(shw) {
  // Give it an icon
  SetBandIcon(this);

  SetAutoLayout(TRUE);

  panel = new wxPanel(this);

  wxButton *closeBut = new wxButton(panel, (wxFunction)SheetPickerClose,
				    "&Close");
  wxLayoutConstraints *bt0 = new wxLayoutConstraints;
  bt0->left.SameAs(panel, wxLeft, 5);
  bt0->top.SameAs(panel, wxTop, 5);
  bt0->width.AsIs();
  bt0->height.AsIs();
  closeBut->SetConstraints(bt0);

  if (multi) {
    wxButton *allBut = new wxButton(panel, (wxFunction)SheetPickerAll,
				    "&All");
    wxLayoutConstraints *bt1 = new wxLayoutConstraints;
    bt1->left.RightOf(closeBut, 5);
    bt1->top.SameAs(closeBut, wxTop);
    bt1->width.AsIs();
    bt1->height.AsIs();
    allBut->SetConstraints(bt1);

    wxButton *noneBut = new wxButton(panel, (wxFunction)SheetPickerNone,
				     "&None");
    wxLayoutConstraints *bt2 = new wxLayoutConstraints;
    bt2->left.RightOf(allBut, 5);
    bt2->top.SameAs(closeBut, wxTop);
    bt2->width.AsIs();
    bt2->height.AsIs();
    noneBut->SetConstraints(bt2);
  }

  closeBut->SetDefault();

  list = new GoodListBox(panel, (wxFunction)SheetPickerClick, "",
			 multi ? wxMULTIPLE:wxSINGLE,
			 -1, -1, -1, -1);
  SetListBoxEntries();
  wxLayoutConstraints *b1 = new wxLayoutConstraints;
  b1->left.SameAs(panel, wxLeft, 5);
  b1->top.Below(closeBut, 5);
  b1->right.SameAs(panel, wxRight, 5);
  b1->bottom.SameAs(panel, wxBottom, 5);
  list->SetConstraints(b1);

  wxLayoutConstraints *c1 = new wxLayoutConstraints;
  c1->left.SameAs(this, wxLeft);
  c1->top.SameAs(this, wxTop);
  c1->right.SameAs(this, wxRight);
  c1->bottom.SameAs(this, wxBottom);
  panel->SetConstraints(c1);

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

void StuntSheetPicker::OnSize(int, int) {
  Layout();
}

Bool StuntSheetPicker::OnClose(void) {
  return TRUE;
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

static void PointPickerClose(wxButton& button, wxEvent&) {
  ((PointPicker*)button.GetParent()->GetParent())->Close();
}

static void PointPickerAll(wxButton& button, wxEvent&) {
  PointPicker* picker =
    (PointPicker*)button.GetParent()->GetParent();

  for (unsigned i=0; i < picker->show->GetNumPoints(); i++) {
    picker->Set(i, TRUE);
  }
  picker->show->winlist->UpdateSelections(picker);
}

static void PointPickerNone(wxButton& button, wxEvent&) {
  PointPicker* picker =
    (PointPicker*)button.GetParent()->GetParent();

  for (unsigned i=0; i < picker->show->GetNumPoints(); i++) {
    picker->Set(i, FALSE);
  }
  picker->show->winlist->UpdateSelections(picker);
}

static void PointPickerClick(wxListBox& list, wxCommandEvent&) {
  unsigned n;
  Bool sel;

  PointPicker *picker = (PointPicker*)list.GetParent()->GetParent();
  for (n = 0; n < picker->show->GetNumPoints(); n++) {
    sel = picker->Get(n);
    if (picker->show->IsSelected(n) != sel) {
      picker->show->Select(n, sel);
      picker->show->winlist->UpdateSelections(picker, n);
    }
  }
}

PointPicker::PointPicker(CC_show *shw, CC_WinList *lst,
			 Bool multi, wxFrame *frame,
			 char *title,
			 int x, int y, int width, int height):
wxFrame(frame, title, x, y, width, height, CC_FRAME_OTHER),
show(shw) {
  // Give it an icon
  SetBandIcon(this);

  SetAutoLayout(TRUE);

  panel = new wxPanel(this);

  wxButton *closeBut = new wxButton(panel, (wxFunction)PointPickerClose,
				    "&Close");
  wxLayoutConstraints *bt0 = new wxLayoutConstraints;
  bt0->left.SameAs(panel, wxLeft, 5);
  bt0->top.SameAs(panel, wxTop, 5);
  bt0->width.AsIs();
  bt0->height.AsIs();
  closeBut->SetConstraints(bt0);

  if (multi) {
    wxButton *allBut = new wxButton(panel, (wxFunction)PointPickerAll,
				    "&All");
    wxLayoutConstraints *bt1 = new wxLayoutConstraints;
    bt1->left.RightOf(closeBut, 5);
    bt1->top.SameAs(closeBut, wxTop);
    bt1->width.AsIs();
    bt1->height.AsIs();
    allBut->SetConstraints(bt1);

    wxButton *noneBut = new wxButton(panel, (wxFunction)PointPickerNone,
				     "&None");
    wxLayoutConstraints *bt2 = new wxLayoutConstraints;
    bt2->left.RightOf(allBut, 5);
    bt2->top.SameAs(closeBut, wxTop);
    bt2->width.AsIs();
    bt2->height.AsIs();
    noneBut->SetConstraints(bt2);
  }

  closeBut->SetDefault();

  list = new GoodListBox(panel, (wxFunction)PointPickerClick, "",
			 multi ? wxMULTIPLE:wxSINGLE,
			 -1, -1, -1, -1);
  SetListBoxEntries();
  wxLayoutConstraints *b1 = new wxLayoutConstraints;
  b1->left.SameAs(panel, wxLeft, 5);
  b1->top.Below(closeBut, 5);
  b1->right.SameAs(panel, wxRight, 5);
  b1->bottom.SameAs(panel, wxBottom, 5);
  list->SetConstraints(b1);

  wxLayoutConstraints *c1 = new wxLayoutConstraints;
  c1->left.SameAs(this, wxLeft);
  c1->top.SameAs(this, wxTop);
  c1->right.SameAs(this, wxRight);
  c1->bottom.SameAs(this, wxBottom);
  panel->SetConstraints(c1);

  node = new CC_WinNodePointPicker(lst, this);

  OnSize(-1,-1);

  Show(TRUE);
}

PointPicker::~PointPicker()
{
  if (node) {
    node->Remove();
    delete node;
  }
}

void PointPicker::OnSize(int, int) {
  Layout();
}

Bool PointPicker::OnClose(void) {
  return TRUE;
}

void PointPicker::Update() {
  list->Clear();
  SetListBoxEntries();
}

void PointPicker::UpdateSelections() {
  unsigned n;

  for (n = 0; n < show->GetNumPoints(); n++) {
    list->SetSelection(n, show->IsSelected(n));
  }
}

void PointPicker::SetListBoxEntries() {
  char **text;
  unsigned n;

  text = new char*[show->GetNumPoints()];
  for (n = 0; n < show->GetNumPoints(); n++) {
    text[n] = (char *)show->GetPointLabel(n);
  }
  list->Set(show->GetNumPoints(), text);
  for (n = 0; n < show->GetNumPoints(); n++) {
    list->SetSelection(n, show->IsSelected(n));
  }
  delete text;
}

static void ShowInfoClose(wxButton& button, wxEvent&) {
  ((ShowInfoReq*)button.GetParent()->GetParent())->Close();
}

static void ShowInfoSetNum(wxButton& button, wxEvent&) {
  unsigned num;
  ShowInfoReq *req = (ShowInfoReq *)button.GetParent()->GetParent();

  num = req->GetNumPoints();
  if (num != req->show->GetNumPoints()) {
    if(wxMessageBox("Changing the number of points is not undoable.\nProceed?",
		    "Change number of points", wxYES_NO, req) == wxYES) {
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
wxFrame(frame, title, x, y, width, height, CC_FRAME_OTHER),
show(shw) {
  unsigned i;
  wxString buf;
  char *strs[2];
  Bool letters[26];
  Bool use_letters;
  int maxnum;

  // Give it an icon
  SetBandIcon(this);

  SetAutoLayout(TRUE);

  CalculateLabels(show, letters, use_letters, maxnum);

  panel = new wxPanel(this);
  panel->SetLabelPosition(wxVERTICAL);

  wxButton *closeBut = new wxButton(panel, (wxFunction)ShowInfoClose,
				    "&Close");
  wxLayoutConstraints *bt0 = new wxLayoutConstraints;
  bt0->left.SameAs(panel, wxLeft, 5);
  bt0->top.SameAs(panel, wxTop, 5);
  bt0->width.AsIs();
  bt0->height.AsIs();
  closeBut->SetConstraints(bt0);

  wxButton *setnumBut = new wxButton(panel, (wxFunction)ShowInfoSetNum,
				     "Set &Num Points");
  wxLayoutConstraints *bt1 = new wxLayoutConstraints;
  bt1->left.RightOf(closeBut, 5);
  bt1->top.SameAs(closeBut, wxTop);
  bt1->width.AsIs();
  bt1->height.AsIs();
  setnumBut->SetConstraints(bt1);

  wxButton *setlabBut = new wxButton(panel, (wxFunction)ShowInfoSetLabels,
				     "&Set Labels");
  wxLayoutConstraints *bt2 = new wxLayoutConstraints;
  bt2->left.RightOf(setnumBut, 5);
  bt2->top.SameAs(closeBut, wxTop);
  bt2->width.AsIs();
  bt2->height.AsIs();
  setlabBut->SetConstraints(bt2);

  closeBut->SetDefault();

  lettersize = new wxSlider(panel, (wxFunction)NULL, "P&oints per letter",
			    maxnum, 1, 10, -1);
  wxLayoutConstraints *b0 = new wxLayoutConstraints;
  b0->left.SameAs(panel, wxLeft, 5);
  b0->right.SameAs(panel, wxRight, 5);
  b0->bottom.SameAs(panel, wxBottom, 5);
  b0->height.AsIs();
  lettersize->SetConstraints(b0);

  labels = new GoodListBox(panel, (wxFunction)NULL, "&Letters",
			   wxMULTIPLE | wxALWAYS_SB, -1, -1, 150, 150);
  for (i = 0; i < 26; i++) {
    buf = (char)(i + 'A');
    labels->Append(buf.GetData());
    labels->SetSelection(i, letters[i]);
  }

  wxLayoutConstraints *b1 = new wxLayoutConstraints;
  b1->left.SameAs(panel, wxLeft, 5);
  b1->top.Below(closeBut, 5);
  b1->width.PercentOf(panel, wxWidth, 40);
  b1->bottom.Above(lettersize, 5);
  labels->SetConstraints(b1);

  buf.sprintf("%u", show->GetNumPoints());
  numpnts = new wxText(panel, (wxFunction)NULL, "&Points", buf.GetData());

  wxLayoutConstraints *b2 = new wxLayoutConstraints;
  b2->left.RightOf(labels, 5);
  b2->top.SameAs(labels, wxTop);
  b2->right.SameAs(panel, wxRight, 5);
  b2->height.AsIs();
  numpnts->SetConstraints(b2);

  strs[0] = "Numbers";
  strs[1] = "Letters";
  label_type = new wxRadioBox(panel, (wxFunction)NULL, "&Labels",
			      -1, -1, -1, -1, 2, strs, 2, wxHORIZONTAL|wxFLAT);
  label_type->SetSelection(use_letters);

  wxLayoutConstraints *b3 = new wxLayoutConstraints;
  b3->left.RightOf(labels, 5);
  b3->top.Below(numpnts, 5);
  b3->width.AsIs();
  b3->height.AsIs();
  label_type->SetConstraints(b3);

  choice = new wxChoice(panel, (wxFunction)ShowInfoModeChoice, "Show &mode");
  ShowMode *mode = modelist->First();
  while (mode != NULL) {
    choice->Append((char*)mode->GetName());
    mode = mode->next;
  }
  UpdateMode();

  wxLayoutConstraints *b4 = new wxLayoutConstraints;
  b4->left.RightOf(labels, 5);
  b4->top.Below(label_type, 5);
  b4->width.AsIs();
  b4->height.AsIs();
  choice->SetConstraints(b4);

  wxLayoutConstraints *c1 = new wxLayoutConstraints;
  c1->left.SameAs(this, wxLeft);
  c1->top.SameAs(this, wxTop);
  c1->right.SameAs(this, wxRight);
  c1->height.PercentOf(this, wxHeight, 70);
  panel->SetConstraints(c1);

  text = new FancyTextWin(this);
  wxLayoutConstraints *c2 = new wxLayoutConstraints;
  c2->left.SameAs(this, wxLeft);
  c2->top.Below(panel);
  c2->right.SameAs(this, wxRight);
  c2->bottom.SameAs(this, wxBottom);
  text->SetConstraints(c2);
  UpdateDescr();

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
  FlushDescr();
  return TRUE;
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
    if (!letters[i] || !labels->Selected(i)) { // so Motif version works
      labels->SetSelection(i, letters[i]);
    }
  }
}

void ShowInfoReq::UpdateNumPoints() {
  wxString buf;

  buf.sprintf("%u", show->GetNumPoints());
  numpnts->SetValue(buf.GetData());
  UpdateLabels();
}

void ShowInfoReq::UpdateMode() {
  choice->SetStringSelection((char*)show->mode->GetName());
}

void ShowInfoReq::UpdateDescr(Bool quick) {
  char *c;

  text->Clear();
  if (quick) {
    c = (char *)show->GetDescr();
  } else {
    c = (char *)show->UserGetDescr();
  }
  text->WriteText(c);
  text->SetInsertionPoint(0);
}

void ShowInfoReq::Update(Bool quick, CC_show *shw) {
  if (shw != NULL) show = shw;
  UpdateDescr(quick);
  UpdateMode();
  UpdateNumPoints();
}

void ShowInfoReq::FlushDescr() {
  char *descr;

  descr = text->GetContents();
  if (strcmp(descr,show->GetDescr()) != 0) {
    show->UserSetDescr(descr, this);
  }
  delete descr;
}

unsigned ShowInfoReq::GetNumPoints() {
  int i;
  wxString buf;

  StringToInt(numpnts->GetValue(), &i);
  if (i < 0) {
    numpnts->SetValue("0");
    i = 0;
  }
  if (i > MAX_POINTS) {
    buf.sprintf("%u", MAX_POINTS);
    numpnts->SetValue(buf.GetData());
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
