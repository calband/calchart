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
#include "confgr.h"

#include <wx_help.h>

extern wxFont *contPlainFont;
extern wxFont *contBoldFont;
extern wxFont *contItalFont;
extern wxFont *contBoldItalFont;

extern wxHelpInstance *help_inst;

static void toolbar_printcont_sym0(CoolToolBar *tb);
static void toolbar_printcont_sym1(CoolToolBar *tb);
static void toolbar_printcont_sym2(CoolToolBar *tb);
static void toolbar_printcont_sym3(CoolToolBar *tb);
static void toolbar_printcont_sym4(CoolToolBar *tb);
static void toolbar_printcont_sym5(CoolToolBar *tb);
static void toolbar_printcont_sym6(CoolToolBar *tb);
static void toolbar_printcont_sym7(CoolToolBar *tb);

ToolBarEntry printcont_tb[] = {
  { 0, NULL, "Insert plainman", toolbar_printcont_sym0 },
  { 0, NULL, "Insert solidman", toolbar_printcont_sym1 },
  { 0, NULL, "Insert backslash man", toolbar_printcont_sym2 },
  { 0, NULL, "Insert slash man", toolbar_printcont_sym3 },
  { 0, NULL, "Insert x man", toolbar_printcont_sym4 },
  { 0, NULL, "Insert solid backslash man", toolbar_printcont_sym5 },
  { 0, NULL, "Insert solid slash man", toolbar_printcont_sym6 },
  { 0, NULL, "Insert solid x man", toolbar_printcont_sym7 }
};

CC_WinNodeCont::CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodeCont::SetShow(CC_show *) {
  editor->DetachText();
  editor->Update(TRUE);
}

void CC_WinNodeCont::GotoSheet(unsigned) {
  editor->Update();
}

void CC_WinNodeCont::DeleteSheet(unsigned sht) {
  if (sht == editor->GetShowDescr()->curr_ss) {
    editor->DetachText();
  }
}

void CC_WinNodeCont::RemoveSheets(unsigned num) {
  if (num <= editor->GetShowDescr()->curr_ss) {
    editor->DetachText();
  }
}

void CC_WinNodeCont::AddContinuity(unsigned sht, unsigned cont) {
  if (sht == editor->GetShowDescr()->curr_ss) {
    if (cont <= editor->GetCurrent()) {
      editor->IncCurrent();
    }
    editor->Update();
  }
}

void CC_WinNodeCont::DeleteContinuity(unsigned sht, unsigned cont) {
  if (sht == editor->GetShowDescr()->curr_ss) {
    if (cont == editor->GetCurrent()) editor->DetachText();
    if (editor->GetCurrent() > 0) {
      if (cont <= editor->GetCurrent()) {
	editor->DecCurrent();
      }
    }
    editor->Update();
  }
}

void CC_WinNodeCont::FlushContinuity() {
  editor->FlushText();
}

void CC_WinNodeCont::SetContinuity(wxWindow *win,
				   unsigned sht, unsigned cont) {
  if ((win != editor) && (sht == editor->GetShowDescr()->curr_ss) &&
      (cont == editor->GetCurrent())) {
    editor->UpdateText(TRUE);
  }
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
descr(dcr), curr_cont(0), text_sheet(NULL), text_contnum(0) {
  // Give it an icon
  SetBandIcon(this);

  CreateStatusLine();

  panel = new wxPanel(this);

  (void)new wxButton(panel, (wxFunction)ContEditSet, "&Set Points");
  (void)new wxButton(panel, (wxFunction)ContEditSelect, "Select &Points");

  conts = new wxChoice(panel, (wxFunction)ContEditCurrent, "");

  text = new FancyTextWin(this);
  
  wxMenu *cont_menu = new wxMenu;
  cont_menu->Append(CALCHART__CONT_NEW, "&New");
  cont_menu->Append(CALCHART__CONT_DELETE, "&Delete");
  cont_menu->Append(CALCHART__CONT_CLOSE, "&Close window");
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(CALCHART__CONT_HELP, "&Help on Continuity...");
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(cont_menu, "&Continuity");
  menu_bar->Append(help_menu, "&Help");
  SetMenuBar(menu_bar);

  OnSize(-1, -1);
  Show(TRUE);

  Update();

  node = new CC_WinNodeCont(lst, this);
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
  FlushText();
  return TRUE;
}

void ContinuityEditor::OnMenuCommand(int id) {
  CC_sheet *sht;
  char *contname;

  switch(id) {
  case CALCHART__CONT_NEW:
    sht = descr->CurrSheet();
    contname = wxGetTextFromUser("Enter the new continuity's name",
				 "New Continuity",
				 NULL, this);
    if (contname) {
      sht->UserNewContinuity(contname);
    }
    break;
  case CALCHART__CONT_DELETE:
    sht = descr->CurrSheet();
    if (sht->ContinuityInUse(curr_cont)) {
      (void)wxMessageBox("This continuity is being used.\nSet these points to a different continuity first.", "Delete continuity");
    } else {
      sht->UserDeleteContinuity(curr_cont);
    }
    break;
  case CALCHART__CONT_CLOSE:
    Close();
    break;
  case CALCHART__CONT_HELP:
    help_inst->LoadFile();
    help_inst->KeywordSearch("Animation Commands");
    break;
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
  case CALCHART__CONT_HELP:
    msg = "Help on continuity commands";
    break;
  }
  if (msg) SetStatusText(msg);
}

void ContinuityEditor::Update(Bool quick) {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *curranimcont;

  conts->Clear();
  for (curranimcont = sht->animcont; curranimcont != NULL;
       curranimcont = curranimcont->next) {
    conts->Append((char *)((const char *)curranimcont->name));
  }
  if (curr_cont >= sht->numanimcont && sht->numanimcont > 0)
    curr_cont = sht->numanimcont-1;
  conts->SetSelection(curr_cont);

  UpdateText(quick);
}

void ContinuityEditor::UpdateText(Bool quick) {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  if (quick) {
    c = sht->GetNthContinuity(curr_cont);
  } else {
    c = sht->UserGetNthContinuity(curr_cont);
  }
  text_sheet = sht;
  text_contnum = curr_cont;
  text->Clear();
  if (c != NULL) {
    if (c->text) {
      text->WriteText((char *)((const char *)c->text));
      text->SetInsertionPoint(0);
    }
  }
}

void ContinuityEditor::FlushText() {
  char *conttext;
  CC_continuity *cont;

  if (text_sheet) {
    cont = text_sheet->GetNthContinuity(text_contnum);
    if (cont != NULL) {
      conttext = text->GetContents();
      if (strcmp(conttext, cont->text) != 0) {
	text_sheet->UserSetNthContinuity(conttext, text_contnum, this);
      }
      delete conttext;
    }
  }
}

void ContinuityEditor::SelectPoints() {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  c = sht->GetNthContinuity(curr_cont);
  if (c != NULL) {
    if (sht->SelectContinuity(c->num)) {
      descr->show->winlist->UpdateSelections();
    }
  }
}

void ContinuityEditor::SetPoints() {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  c = sht->GetNthContinuity(curr_cont);
  if (c != NULL) {
    sht->SetContinuity(c->num);
  }
}

PrintContCanvas::PrintContCanvas(wxFrame *frame, CC_descr *dcr):
wxCanvas(frame, -1, -1, -1, -1, 0), show_descr(dcr), ourframe(frame),
topline(0), width(0), height(0) {
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

  width = 0;
  cont = sht->continuity;
  for (i = 0; (i < topline) && (cont != NULL); i++, cont = cont->next);
  y = 0.0;
  SetFont(contPlainFont);
  tabw = GetCharWidth() * 6; // Size of tab
  while (cont) {
    x = 0.0;
    if (cont->center) {
      for (c = cont; c != NULL; c = c->more) {
	do_tab = FALSE;
	switch (c->font) {
	case PSFONT_SYMBOL:
	  GetTextExtent("O", &textw, &texth, &textd);
	  x += textw * strlen(c->text);
	  break;
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
	if (!do_tab && (c->font != PSFONT_SYMBOL)) {
	  GetTextExtent(c->text, &textw, &texth, &textd);
	  x += textw;
	}
      }
      GetVirtualSize(&devx, &devy);
      x = (GetDC()->DeviceToLogicalX(devx) - x) / 2;
      if (x < 0.0) x = 0.0;
    }
    maxtexth = contPlainFont->GetPointSize();
    tabnum = 0;
    for (c = cont; c != NULL; c = c->more) {
      do_tab = FALSE;
      switch (c->font) {
      case PSFONT_NORM:
      case PSFONT_SYMBOL:
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
      if (c->font == PSFONT_SYMBOL) {
	GetTextExtent("O", &textw, &texth, &textd);
	float d = textw;
	SYMBOL_TYPE sym;

	float topline = y + texth - textd - d;

	for (const char *s = c->text; *s; s++) {
	  sym = (SYMBOL_TYPE)(*s - 'A');
	  switch (sym) {
	  case SYMBOL_SOL:
	  case SYMBOL_SOLBKSL:
	  case SYMBOL_SOLSL:
	  case SYMBOL_SOLX:
	    SetBrush(wxBLACK_BRUSH);
	    break;
	  default:
	    SetBrush(wxTRANSPARENT_BRUSH);
	  }
	  DrawEllipse(x, topline, d, d);
	  switch (sym) {
	  case SYMBOL_SL:
	  case SYMBOL_X:
	  case SYMBOL_SOLSL:
	  case SYMBOL_SOLX:
	    DrawLine(x, topline + d, x + d, topline);
	    break;
	  default:
	    break;
	  }
	  switch (sym) {
	  case SYMBOL_BKSL:
	  case SYMBOL_X:
	  case SYMBOL_SOLBKSL:
	  case SYMBOL_SOLX:
	    DrawLine(x, topline, x + d, topline + d);
	    break;
	  default:
	    break;
	  }
	  x += d;
	}
	if (texth > maxtexth) maxtexth = texth;
      } else {
	if (!do_tab) {
	  GetTextExtent(c->text, &textw, &texth, &textd);
	  if (texth > maxtexth) maxtexth = texth;
	  DrawText(c->text, x, y);
	  x += textw;
	}
      }
    }
    if (x > width) width = x;
    y += maxtexth;
    cont = cont->next;
  }
  height = y;

  SetFont(NULL);
  EndDrawing();
}

void PrintContCanvas::OnEvent(wxMouseEvent& /*event*/) {
}

void PrintContCanvas::OnChar(wxKeyEvent& /*event*/) {
}

void PrintContCanvas::UpdateBars() {
  SetScrollbars(1, 1, (int)width, (int)height, 1, 1);
}

void PrintContClose(wxButton& button, wxEvent&) {
  ((PrintContEditor*)(button.GetParent()->GetParent()))->Close();
}

PrintContEditor::PrintContEditor(CC_descr *dcr, CC_WinList *lst,
				 wxFrame *parent, char *title,
				 int x, int y, int width, int height)
: wxFrameWithStuff(parent, title, x, y, width, height) {
  // Give it an icon
  SetBandIcon(this);

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
  (void)new wxButton(framePanel, (wxFunction)PrintContClose, "&Close");

  node = new CC_WinNodePrintCont(lst, this);

  canvas->Update();

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

static void toolbar_printcont_sym0(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym1(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym2(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym3(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym4(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym5(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym6(CoolToolBar */*tb*/) {
}
static void toolbar_printcont_sym7(CoolToolBar */*tb*/) {
}
