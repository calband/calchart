/* cont_ui.cc
 * Continuity editors
 *
 * Modification history:
 * 1-10-96    Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "cont_ui.h"

extern wxFont *contPlainFont;
extern wxFont *contBoldFont;
extern wxFont *contItalFont;
extern wxFont *contBoldItalFont;

static void toolbar_printcont_sym0(CoolToolBar *tb);
static void toolbar_printcont_sym1(CoolToolBar *tb);
static void toolbar_printcont_sym2(CoolToolBar *tb);
static void toolbar_printcont_sym3(CoolToolBar *tb);
static void toolbar_printcont_sym4(CoolToolBar *tb);
static void toolbar_printcont_sym5(CoolToolBar *tb);
static void toolbar_printcont_sym6(CoolToolBar *tb);
static void toolbar_printcont_sym7(CoolToolBar *tb);

ToolBarEntry printcont_tb[] = {
  { NULL, "Insert plainmen", toolbar_printcont_sym0 },
  { NULL, "Insert solidmen", toolbar_printcont_sym1 },
  { NULL, "Insert plain backslash men", toolbar_printcont_sym2 },
  { NULL, "Insert plain slash men", toolbar_printcont_sym3 },
  { NULL, "Insert plain x men", toolbar_printcont_sym4 },
  { NULL, "Insert solid backslash men", toolbar_printcont_sym5 },
  { NULL, "Insert solid slash men", toolbar_printcont_sym6 },
  { NULL, "Insert solid x men", toolbar_printcont_sym7 }
};

CC_WinNodeCont::CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodeCont::SetShow(CC_show *) {
  editor->Update();
}

void CC_WinNodeCont::GotoSheet(unsigned) {
  editor->Update();
}

void CC_WinNodeCont::AddContinuity(unsigned sht, unsigned cont) {
  editor->Update();
}

void CC_WinNodeCont::DeleteContinuity(unsigned sht, unsigned cont) {
  editor->Update();
}

CC_WinNodePrintCont::CC_WinNodePrintCont(CC_WinList *lst,
					 PrintContEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodePrintCont::SetShow(CC_show *) {
  editor->canvas->Update();
}

void CC_WinNodePrintCont::GotoSheet(unsigned) {
  editor->canvas->Update();
}

static void ContEditSet(wxButton& button, wxEvent&) {
  ContinuityEditor* editor =
    (ContinuityEditor*)(button.GetParent()->GetParent());

  editor->SetPoints();
}

static void ContEditSelect(wxButton& button, wxEvent&) {
  ContinuityEditor* editor =
    (ContinuityEditor*)(button.GetParent()->GetParent());

  editor->SelectPoints();
}

static void ContEditCurrent(wxChoice& choice, wxEvent&) {
  ContinuityEditor *editor =
    (ContinuityEditor*)(choice.GetParent()->GetParent());
  editor->SetCurrent(choice.GetSelection());
}

ContinuityEditor::ContinuityEditor(CC_descr *dcr, CC_WinList *lst,
				   wxFrame *parent, char *title,
				   int x, int y, int width, int height):
wxFrame(parent, title, x, y, width, height, wxSDI | wxDEFAULT_FRAME),
descr(dcr), curr_cont(0) {
  CreateStatusLine();

  panel = new wxPanel(this);

  (void)new wxButton(panel, (wxFunction)ContEditSet, "Set");
  (void)new wxButton(panel, (wxFunction)ContEditSelect, "Select");

  conts = new wxChoice(panel, (wxFunction)ContEditCurrent, "");

  text = new FancyTextWin(this, -1, -1, -1, -1, wxNATIVE_IMPL);
  
  wxMenu *cont_menu = new wxMenu;
  cont_menu->Append(CALCHART__CONT_NEW, "New");
  cont_menu->Append(CALCHART__CONT_DELETE, "Delete");
  cont_menu->Append(CALCHART__CONT_CLOSE, "Close window");
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(cont_menu, "Continuity");
  SetMenuBar(menu_bar);

  node = new CC_WinNodeCont(lst, this);

  OnSize(-1, -1);
  Show(TRUE);

  Update();
}

ContinuityEditor::~ContinuityEditor() {
  if (node) {
    node->Remove();
    delete node;
  }
}

void ContinuityEditor::OnSize(int, int) {
  int width, height;
  int text_x, text_y;

  GetClientSize(&width, &height);
  panel->Fit();
  panel->GetSize(&text_x, &text_y);
  panel->SetSize(0, 0, width, text_y);
  text->SetSize(0, text_y, width, height-text_y);
}

Bool ContinuityEditor::OnClose(void) {
  return TRUE;
}

void ContinuityEditor::OnMenuCommand(int id) {
  switch(id) {
  case CALCHART__CONT_NEW:
    break;
  case CALCHART__CONT_DELETE:
    break;
  case CALCHART__CONT_CLOSE:
    Close();
  }
}

void ContinuityEditor::OnMenuSelect(int id) {
  char *msg = NULL;

  switch (id) {
  case CALCHART__CONT_NEW:
    msg = "Add new continuity";
    break;
  case CALCHART__CONT_DELETE:
    msg = "Delete this continuity";
    break;
  case CALCHART__CONT_CLOSE:
    msg = "Close window";
    break;
  }
  if (msg) SetStatusText(msg);
}

void ContinuityEditor::Update() {
  CC_sheet *sht = descr->CurrSheet();
  unsigned i;

  conts->Clear();
  for (i = 0; i < sht->numanimcont; i++) {
    conts->Append((char *)((const char *)sht->animcont[i].name));
  }
  conts->SetSelection(0);
  curr_cont = 0;

  UpdateText();
}

void ContinuityEditor::UpdateText() {
  CC_sheet *sht = descr->CurrSheet();

  text->Clear();
  if (sht->numanimcont > 0) {
    if (sht->animcont[curr_cont].text)
      text->WriteText((char *)((const char *)sht->animcont[curr_cont].text));
  }
}

void ContinuityEditor::SelectPoints() {
  CC_sheet *sht = descr->CurrSheet();

  if (curr_cont < sht->numanimcont) {
    if (sht->SelectContinuity(sht->animcont[curr_cont].num)) {
      descr->show->winlist->UpdateSelections();
    }
  }
}

void ContinuityEditor::SetPoints() {
  CC_sheet *sht = descr->CurrSheet();

  if (curr_cont < sht->numanimcont) {
    sht->SetContinuity(sht->animcont[curr_cont].num);
  }
}

PrintContCanvas::PrintContCanvas(wxFrame *frame, CC_descr *dcr):
wxCanvas(frame, -1, -1, -1, -1, 0), show_descr(dcr), ourframe(frame),
topline(0) {
  GetDC()->SetMapMode(MM_TEXT);
  GetDC()->SetBackground(wxWHITE_BRUSH);
}

PrintContCanvas::~PrintContCanvas() {}

void PrintContCanvas::OnPaint() {
  cc_text *cont, *c;
  CC_sheet *sht = show_descr->CurrSheet();
  Bool do_tab;
  unsigned i;
  float x, y;
  float textw, texth, textd, maxtexth;
  int devx, devy;
  unsigned tabnum;
  float tabw;

  BeginDrawing();
  SetTextForeground(wxBLACK);
  Clear();

  cont = sht->continuity;
  for (i = 0; (i < topline) && (cont != NULL); i++, cont = cont->next);
  y = 0.0;
  SetFont(contPlainFont);
  GetTextExtent("012345", &tabw, &texth, &textd); // Get size of tab
  while (cont) {
    x = 0.0;
    if (cont->center) {
      for (c = cont; c != NULL; c = c->more) {
	do_tab = FALSE;
	switch (c->font) {
	case PSFONT_NORM:
	  SetFont(contPlainFont);
	  break;
	case PSFONT_BOLD:
	  SetFont(contBoldFont);
	  break;
	case PSFONT_ITAL:
	  SetFont(contItalFont);
	  break;
	case PSFONT_BOLDITAL:
	  SetFont(contBoldItalFont);
	  break;
	case PSFONT_TAB:
	  do_tab = TRUE;
	  break;
	}
	if (!do_tab) {
	  GetTextExtent(c->text, &textw, &texth, &textd);
	  x += textw;
	}
      }
      GetVirtualSize(&devx, &devy);
      x = (GetDC()->DeviceToLogicalX(devx) - x) / 2;
      if (x < 0.0) x = 0.0;
    }
    maxtexth = 0.0;
    tabnum = 0;
    for (c = cont; c != NULL; c = c->more) {
      do_tab = FALSE;
      switch (c->font) {
      case PSFONT_NORM:
	SetFont(contPlainFont);
	break;
      case PSFONT_BOLD:
	SetFont(contBoldFont);
	break;
      case PSFONT_ITAL:
	SetFont(contItalFont);
	break;
      case PSFONT_BOLDITAL:
	SetFont(contBoldItalFont);
	break;
      case PSFONT_TAB:
	tabnum++;
	textw = tabnum * tabw;
	if (textw >= x) x = textw;
	else x += tabw/6;
	do_tab = TRUE;
	break;
      default:
	break;
      }
      if (!do_tab) {
	GetTextExtent(c->text, &textw, &texth, &textd);
	if (texth > maxtexth) maxtexth = texth;
	DrawText(c->text, x, y);
	x += textw;
      }
    }
    y += maxtexth;
    cont = cont->next;
  }

  SetFont(NULL);
  EndDrawing();
}

void PrintContCanvas::OnEvent(wxMouseEvent& event) {
}

void PrintContCanvas::OnChar(wxKeyEvent& event) {
}

void PrintContClose(wxButton& button, wxEvent&) {
  ((PrintContEditor*)(button.GetParent()->GetParent()))->Close();
}

PrintContEditor::PrintContEditor(CC_descr *dcr, CC_WinList *lst,
				 wxFrame *parent, char *title,
				 int x, int y, int width, int height)
: wxFrameWithStuff(parent, title, x, y, width, height) {
  CreateStatusLine(1);

  // Add a toolbar
  CoolToolBar *ribbon = new CoolToolBar(this, 0, 0, -1, -1, 0,
					wxHORIZONTAL, 20);
  ribbon->SetupBar(printcont_tb, sizeof(printcont_tb)/sizeof(ToolBarEntry));
  SetToolBar(ribbon);

  // Add the canvas
  canvas = new PrintContCanvas(this, dcr);
  SetCanvas(canvas);

  // Add the buttons
  SetPanel(new wxPanel(this));
  (void)new wxButton(framePanel, (wxFunction)PrintContClose, "Close");

  node = new CC_WinNodePrintCont(lst, this);

  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  OnSize(-1, -1);
  Show(TRUE);
}

PrintContEditor::~PrintContEditor() {
  if (node) {
    node->Remove();
    delete node;
  }
}

static void toolbar_printcont_sym0(CoolToolBar *tb) {
}
static void toolbar_printcont_sym1(CoolToolBar *tb) {
}
static void toolbar_printcont_sym2(CoolToolBar *tb) {
}
static void toolbar_printcont_sym3(CoolToolBar *tb) {
}
static void toolbar_printcont_sym4(CoolToolBar *tb) {
}
static void toolbar_printcont_sym5(CoolToolBar *tb) {
}
static void toolbar_printcont_sym6(CoolToolBar *tb) {
}
static void toolbar_printcont_sym7(CoolToolBar *tb) {
}
