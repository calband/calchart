/* main_ui.cc
 * Handle wxWindows interface
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "main_ui.h"
#include "print_ui.h"
#include "show_ui.h"
#include "anim_ui.h"
#include "cont_ui.h"
#include "undo.h"
#include "modes.h"
#include "confgr.h"

#include <wx_help.h>

#ifdef wx_msw
#include <direct.h>
#else
#include <dirent.h>
#endif

#ifdef wx_x
#include "tb_left.xbm"
#include "tb_right.xbm"
#include "tb_box.xbm"
#include "tb_poly.xbm"
#include "tb_lasso.xbm"
#include "tb_line.xbm"
#include "tb_lbl_l.xbm"
#include "tb_lbl_r.xbm"
#include "tb_lbl_f.xbm"
#include "tb_sym0.xbm"
#include "tb_sym1.xbm"
#include "tb_sym2.xbm"
#include "tb_sym3.xbm"
#include "tb_sym4.xbm"
#include "tb_sym5.xbm"
#include "tb_sym6.xbm"
#include "tb_sym7.xbm"
#include "tb_stop.xbm"
#include "tb_play.xbm"
#include "tb_pbeat.xbm"
#include "tb_nbeat.xbm"
#include "tb_pshet.xbm"
#include "tb_nshet.xbm"
#endif

static void toolbar_prev_ss(CoolToolBar *tb);
static void toolbar_next_ss(CoolToolBar *tb);
static void toolbar_box(CoolToolBar *tb);
static void toolbar_poly(CoolToolBar *tb);
static void toolbar_lasso(CoolToolBar *tb);
static void toolbar_line(CoolToolBar *tb);
static void toolbar_label_left(CoolToolBar *tb);
static void toolbar_label_right(CoolToolBar *tb);
static void toolbar_label_flip(CoolToolBar *tb);
static void toolbar_setsym0(CoolToolBar *tb);
static void toolbar_setsym1(CoolToolBar *tb);
static void toolbar_setsym2(CoolToolBar *tb);
static void toolbar_setsym3(CoolToolBar *tb);
static void toolbar_setsym4(CoolToolBar *tb);
static void toolbar_setsym5(CoolToolBar *tb);
static void toolbar_setsym6(CoolToolBar *tb);
static void toolbar_setsym7(CoolToolBar *tb);
static void refnum_callback(wxObject &obj, wxEvent &ev);
static void slider_sheet_callback(wxObject &obj, wxEvent &ev);
static void slider_zoom_callback(wxObject &obj, wxEvent &ev);

static ToolBarEntry main_tb[] = {
  { 0, NULL, "Previous stuntsheet", toolbar_prev_ss },
  { TOOLBAR_SPACE, NULL, "Next stuntsheet", toolbar_next_ss },
  { 0, NULL, "Select points with box", toolbar_box },
  { 0, NULL, "Select points with polygon", toolbar_poly },
  { TOOLBAR_SPACE, NULL, "Select points with lasso", toolbar_lasso },
  { TOOLBAR_SPACE, NULL, "Move points into line", toolbar_line },
  { 0, NULL, "Label on left", toolbar_label_left },
  { 0, NULL, "Flip label", toolbar_label_flip },
  { TOOLBAR_SPACE, NULL, "Label on right", toolbar_label_right },
  { 0, NULL, "Change to plainmen", toolbar_setsym0 },
  { 0, NULL, "Change to solidmen", toolbar_setsym1 },
  { 0, NULL, "Change to backslash men", toolbar_setsym2 },
  { 0, NULL, "Change to slash men", toolbar_setsym3 },
  { 0, NULL, "Change to x men", toolbar_setsym4 },
  { 0, NULL, "Change to solid backslash men", toolbar_setsym5 },
  { 0, NULL, "Change to solid slash men", toolbar_setsym6 },
  { 0, NULL, "Change to solid x men", toolbar_setsym7 }
};

extern ToolBarEntry anim_tb[];
extern ToolBarEntry printcont_tb[];

char *gridtext[] = {
  "None",
  "1",
  "2",
  "4"
};

static char *file_wild = FILE_WILDCARDS;
static char *file_save_wild = FILE_SAVE_WILDCARDS;

Coord gridvalue[] = {
  1,
  INT2COORD(1),
  INT2COORD(2),
  INT2COORD(4)
};

// This statement initializes the whole application and calls OnInit
CalChartApp theApp;

static MainFrameList *window_list;

wxHelpInstance *help_inst = NULL;

wxFont *contPlainFont;
wxFont *contBoldFont;
wxFont *contItalFont;
wxFont *contBoldItalFont;
wxFont *pointLabelFont;
wxFont *yardLabelFont;

ShowModeList *modelist;

CC_WinNodeMain::CC_WinNodeMain(CC_WinList *lst, MainFrame *frm)
: CC_WinNode(lst), frame(frm) {}

void CC_WinNodeMain::SetShow(CC_show *shw) {
  Remove();
  list = shw->winlist;
  shw->winlist->Add(this);
  frame->field->show_descr.show = shw;
  winlist.SetShow(shw); // Must set new show before redrawing
  frame->field->UpdateBars();
  frame->field->GotoSS(0);
  ChangeName();
}
void CC_WinNodeMain::ChangeName() {
  frame->SetTitle((char *)frame->field->show_descr.show->UserGetName());
  winlist.ChangeName();
}
void CC_WinNodeMain::UpdateSelections(wxWindow* win, int point) {
  if (frame->field->GetDC()->Colour) {
    frame->field->RefreshShow(FALSE, point);
  } else {
    // In mono we use different line widths, so must redraw everything
    frame->field->RefreshShow();
  }
  winlist.UpdateSelections(win, point);
}
void CC_WinNodeMain::UpdatePoints() {
  frame->field->RefreshShow();
  winlist.UpdatePoints();
}
void CC_WinNodeMain::UpdatePointsOnSheet(unsigned sht, int ref) {
  if (sht == frame->field->show_descr.curr_ss) {
    // ref = 0 means that any points could move
    if ((ref <= 0) || (ref == (int)frame->field->curr_ref)) {
      frame->field->RefreshShow();
    }
  }
  winlist.UpdatePointsOnSheet(sht, ref);
}
void CC_WinNodeMain::ChangeNumPoints(wxWindow *win) {
  frame->field->UpdateSS();
  winlist.ChangeNumPoints(win);
}
void CC_WinNodeMain::ChangePointLabels(wxWindow *win) {
  frame->field->UpdateSS();
  winlist.ChangePointLabels(win);
}
void CC_WinNodeMain::ChangeShowMode(wxWindow *win) {
  frame->field->UpdateBars();
  frame->field->UpdateSS();
  winlist.ChangeShowMode(win);
}
void CC_WinNodeMain::UpdateStatusBar() {
  frame->UpdatePanel();
  winlist.UpdateStatusBar();
}
void CC_WinNodeMain::GotoSheet(unsigned sht) {
  winlist.GotoSheet(sht);
}
void CC_WinNodeMain::AddSheet(unsigned sht) {
  if (sht <= frame->field->show_descr.curr_ss) {
    frame->field->show_descr.curr_ss++;
  }
  frame->UpdatePanel();
  winlist.AddSheet(sht);
}
void CC_WinNodeMain::DeleteSheet(unsigned sht) {
  winlist.DeleteSheet(sht);
  if (sht < frame->field->show_descr.curr_ss) {
    frame->field->show_descr.curr_ss--;
  }
  if (frame->field->show_descr.curr_ss >=
      frame->field->show_descr.show->GetNumSheets()) {
    frame->field->PrevSS();
  } else {
    if (sht == frame->field->show_descr.curr_ss) {
      frame->field->GotoThisSS();
    } else {
      frame->UpdatePanel();
    }
  }
}
void CC_WinNodeMain::AppendSheets() {
  frame->UpdatePanel();
  winlist.AppendSheets();
}
void CC_WinNodeMain::RemoveSheets(unsigned num) {
  winlist.RemoveSheets(num);
  if (frame->field->show_descr.curr_ss >=
      frame->field->show_descr.show->GetNumSheets()) {
    frame->field->GotoSS(num-1);
  }
}
void CC_WinNodeMain::ChangeTitle(unsigned sht) {
  if (sht == frame->field->show_descr.curr_ss) frame->UpdatePanel();
  winlist.ChangeTitle(sht);
}
void CC_WinNodeMain::SelectSheet(wxWindow* win, unsigned sht) {
  winlist.SelectSheet(win, sht);
}
void CC_WinNodeMain::AddContinuity(unsigned sht, unsigned cont) {
  winlist.AddContinuity(sht, cont);
}
void CC_WinNodeMain::DeleteContinuity(unsigned sht, unsigned cont) {
  winlist.DeleteContinuity(sht, cont);
}
void CC_WinNodeMain::FlushContinuity() {
  winlist.FlushContinuity();
}
void CC_WinNodeMain::SetContinuity(wxWindow* win,
				   unsigned sht, unsigned cont) {
  winlist.SetContinuity(win, sht, cont);
}
void CC_WinNodeMain::ChangePrint(wxWindow* win) {
  winlist.ChangePrint(win);
}
void CC_WinNodeMain::FlushDescr() {
  winlist.FlushDescr();
}
void CC_WinNodeMain::SetDescr(wxWindow* win) {
  winlist.SetDescr(win);
}

// Create windows and return main app frame
wxFrame *CalChartApp::OnInit(void)
{
  char *runtimepath = "runtime";
  int realargc = argc;

  modelist = new ShowModeList();

  if (argc > 1) {
    DIR *d = opendir(argv[argc-1]);
    if (d != NULL) {
      runtimepath = argv[argc-1];
      closedir(d);
      realargc--;
    }
  }

  char *s = ReadConfig(runtimepath);
  if (s) {
    (void)wxMessageBox(s, "CalChart");
  }

  //Create toolbar bitmaps
  int i = 0;

  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_left));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_right));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_box));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_poly));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lasso));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_line));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_l));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_f));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lbl_r));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym0));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym1));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym2));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym3));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym4));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym5));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym6));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym7));

  i = 0;

  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_stop));
  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_play));
  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_pbeat));
  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_nbeat));
  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_pshet));
  anim_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_nshet));

  i = 0;

  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym0));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym1));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym2));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym3));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym4));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym5));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym6));
  printcont_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_sym7));

  contPlainFont = new wxFont(11, wxMODERN, wxNORMAL, wxNORMAL);
  contBoldFont = new wxFont(11, wxMODERN, wxNORMAL, wxBOLD);
  contItalFont = new wxFont(11, wxMODERN, wxITALIC, wxNORMAL);
  contBoldItalFont = new wxFont(11, wxMODERN, wxITALIC, wxBOLD);
  pointLabelFont = new wxFont((int)FLOAT2NUM(dot_ratio * num_ratio),
			      wxSWISS, wxNORMAL, wxNORMAL);
  yardLabelFont = new wxFont((int)FLOAT2NUM(yards_size),
			      wxSWISS, wxNORMAL, wxNORMAL);

  window_list = new MainFrameList();

  {
    wxString helpfile(program_dir);
    helpfile.Append(PATH_SEPARATOR "charthlp");
    help_inst = new wxHelpInstance(TRUE);
    help_inst->Initialize(helpfile.GetData());
  }

  for (i = 1; i < realargc; i++) {
    CC_show *shw;

    shw = new CC_show(argv[i]);
    if (shw->Ok()) {
      new MainFrame(NULL, 50, 50,
		    window_default_width, window_default_height, shw);
    } else {
      (void)wxMessageBox(shw->GetError(), "Load Error");
      delete shw;
    }
  }
  if (!shows_dir.Empty()) {
    wxSetWorkingDirectory(shows_dir.GetData());
  }

  return new TopFrame();
}

int CalChartApp::OnExit(void) {
  if (modelist) delete modelist;
  if (help_inst) delete help_inst;
  if (window_list) delete window_list;

  return 0;
}

CC_lasso::CC_lasso() {}

CC_lasso::~CC_lasso() {
  Clear();
}

void CC_lasso::Clear() {
  for (wxNode *n = pntlist.First(); n != NULL; n = n->Next()) {
    delete n->Data();
  }
  pntlist.Clear();
}

void CC_lasso::Start(const CC_coord& p) {
  Clear();
  Append(p);
}

// Closes polygon
void CC_lasso::End() {
  wxNode *n = pntlist.Last();
  wxPoint *first;
  if (n) {
    first = (wxPoint*)n->Data();
    wxPoint *p = new wxPoint(first->x, first->y);
    pntlist.Insert(p);
  }
}

void CC_lasso::Append(const CC_coord& p) {
  pntlist.Insert(new wxPoint(p.x, p.y));
}

// Test if inside polygon using odd-even rule
Bool CC_lasso::Inside(const CC_coord& p) {
  Bool parity = FALSE;
  wxNode *last;
  wxNode *n = pntlist.First();
  if (n != NULL) {
    last = n;
    n = n->Next();
    while (n != NULL) {
      if (CrossesLine((wxPoint*)last->Data(), (wxPoint*)n->Data(), p)) {
	parity ^= TRUE;
      }
      last = n;
      n = n->Next();
    }
  }
  return parity;
}

void CC_lasso::Draw(wxDC *dc, float x, float y) {
  SetXOR(dc);
  dc->DrawLines(&pntlist, x, y);
}

/*
void CC_lasso::Drag(wxDC *dc, const CC_coord& p) {
  wxNode *n1, *n2;
  wxPoint *p1, *p2;

  n1 = pntlist.First();
  if (n1 != NULL) {
    n2 = n1->Next();
    if (n2 != NULL) {
      SetXOR(dc);
      p1 = (wxPoint*)n1->Data();
      p2 = (wxPoint*)n2->Data();
      dc->DrawLine(p1->x, p1->y, p2->x, p2->y);
      p1->x = p.x;
      p1->y = p.y;
      dc->DrawLine(p1->x, p1->y, p2->x, p2->y);
    }
  }
}
*/
void CC_lasso::Drag(const CC_coord& p) {
  wxNode *n1;
  wxPoint *p1;

  n1 = pntlist.First();
  if (n1 != NULL) {
    p1 = (wxPoint*)n1->Data();
    p1->x = p.x;
    p1->y = p.y;
  }
}

Bool CC_lasso::CrossesLine(const wxPoint* start, const wxPoint* end,
			   const CC_coord& p) {
  if (start->y > end->y) {
    if (!((p.y <= start->y) && (p.y > end->y))) {
      return FALSE;
    }
  } else {
    if (!((p.y <= end->y) && (p.y > start->y))) {
      return FALSE;
    }
  }
  if (p.x >=
      ((end->x-start->x) * (p.y-start->y) / (end->y-start->y) + start->x)) {
    return TRUE;
  }
  return FALSE;
}

static void new_window(wxButton&, wxEvent&) {
  (void)new MainFrame(NULL, 50, 50,
		      window_default_width, window_default_height);
}

static void open_window(wxButton&, wxEvent&) {
  const char *s;
  CC_show *shw;

  s = wxFileSelector("Load show", NULL, NULL, NULL, file_wild);
  if (s) {
    shw = new CC_show(s);
    if (shw->Ok()) {
      new MainFrame(NULL, 50, 50,
		    window_default_width, window_default_height, shw);
    } else {
      (void)wxMessageBox(shw->GetError(), "Load Error");
      delete shw;
    }
  }
}

static void quit_callback(wxButton &button, wxEvent &) {
  wxFrame *f = (wxFrame*)button.GetParent()->GetParent();
  f->Close();
}

TopFrame::TopFrame(wxFrame *frame):
  wxFrame(frame, "CalChart")
{
  int w, h;

  // Give it an icon
  SetBandIcon(this);

  wxPanel *p = new wxPanel(this);
  p->SetHorizontalSpacing(0);
  p->SetVerticalSpacing(0);
  (void)new wxButton(p, (wxFunction)new_window, "&New");
  (void)new wxButton(p, (wxFunction)open_window, "&Open");
  (void)new wxButton(p, (wxFunction)quit_callback, "&Quit");
  p->Fit();
  p->GetSize(&w, &h);
  SetClientSize(w, h);
#ifndef BUGGY_SIZE_HINTS
  GetSize(&w, &h);
  SetSizeHints(w, h, w, h);
#endif
  Show(TRUE);
}

Bool TopFrame::OnClose(void) {
  return window_list->CloseAllWindows();
}

// Main frame constructor
MainFrame::MainFrame(wxFrame *frame, int x, int y, int w, int h,
		     CC_show *show, MainFrame *other_frame):
  wxFrameWithStuff(frame, "CalChart", x, y, w, h, wxSDI|wxDEFAULT_FRAME),
  field(NULL)
{
  unsigned ss;
  unsigned def_zoom;
  unsigned def_grid;
  unsigned def_ref;

  // Give it an icon
  SetBandIcon(this);

  // Give it a status line
  CreateStatusLine(2);
  SetStatusText("Welcome to Calchart 3.0");

  // Make a menubar
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(CALCHART__NEW, "&New Show");
  file_menu->Append(CALCHART__NEW_WINDOW, "New &Window");
  file_menu->Append(CALCHART__LOAD_FILE, "&Open...");
  file_menu->Append(CALCHART__APPEND_FILE, "&Append...");
  file_menu->Append(CALCHART__SAVE, "&Save");
  file_menu->Append(CALCHART__SAVE_AS, "Save &As...");
  file_menu->Append(CALCHART__PRINT, "&Print...");
  file_menu->Append(CALCHART__PRINT_EPS, "Print &EPS...");
  file_menu->Append(CALCHART__CLOSE, "&Close Window");

  wxMenu *edit_menu = new wxMenu;
  edit_menu->Append(CALCHART__UNDO, "&Undo");
  edit_menu->Append(CALCHART__REDO, "&Redo");
  edit_menu->Append(CALCHART__INSERT_BEFORE, "&Insert Sheet Before");
  edit_menu->Append(CALCHART__INSERT_AFTER, "Insert Sheet &After");
  edit_menu->Append(CALCHART__DELETE, "&Delete Sheet");
  edit_menu->Append(CALCHART__RELABEL, "&Relabel Sheets");
  edit_menu->Append(CALCHART__EDIT_CONTINUITY, "&Edit Continuity...");
  edit_menu->Append(CALCHART__EDIT_PRINTCONT, "Edit &Printed Continuity...");
  edit_menu->Append(CALCHART__SET_TITLE, "Set &Title...");
  edit_menu->Append(CALCHART__SET_BEATS, "Set &Beats...");

  wxMenu *win_menu = new wxMenu;
  win_menu->Append(CALCHART__INFO, "&Information...");
  win_menu->Append(CALCHART__POINTS, "Point &Selections...");
  win_menu->Append(CALCHART__ANIMATE, "&Animate...");

  wxMenu *select_menu = new wxMenu;
  // These items are checkable
  select_menu->Append(CALCHART__ROWS, "Rows first", NULL, TRUE);
  select_menu->Append(CALCHART__COLUMNS, "Columns first", NULL, TRUE);
  select_menu->Append(CALCHART__NEAREST, "Nearest", NULL, TRUE);

  wxMenu *options_menu = new wxMenu;
  options_menu->Append(CALCHART__SELECTION, "Selection Order", select_menu);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(CALCHART__ABOUT, "&About CalChart...");
  help_menu->Append(CALCHART__HELP, "&Help on CalChart...");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, "&File");
  menu_bar->Append(edit_menu, "&Edit");
  menu_bar->Append(win_menu, "&Windows");
  menu_bar->Append(options_menu, "&Options");
  menu_bar->Append(help_menu, "&Help");
  SetMenuBar(menu_bar);

  // Add a toolbar
  CoolToolBar *ribbon = new CoolToolBar(this, 0, 0, -1, -1, 0,
					wxHORIZONTAL, 20);
  ribbon->SetupBar(main_tb, sizeof(main_tb)/sizeof(ToolBarEntry));
  SetToolBar(ribbon);

  // Add the field canvas
  if (!other_frame) {
    if (!show) show = new CC_show();
    ss = 0;
    def_zoom = FIELD_DEFAULT_ZOOM;
    def_grid = 2;
    def_ref = 0;
    field = new FieldCanvas(show, ss, this, def_zoom);
  } else {
    show = other_frame->field->show_descr.show;
    ss = other_frame->field->show_descr.curr_ss;
    def_zoom = other_frame->zoom_slider->GetValue();
    def_grid = other_frame->grid_choice->GetSelection();
    def_ref = other_frame->field->curr_ref;
    field = new FieldCanvas(show, ss, this, def_zoom, other_frame->field);
  }

  SetTitle((char *)show->UserGetName());
  field->curr_ref = def_ref;
  frameCanvas = field;
  node = new CC_WinNodeMain(show->winlist, this);
  switch(field->curr_select) {
  case CC_SELECT_ROWS:
    menu_bar->Check(CALCHART__ROWS, TRUE);
    break;
  case CC_SELECT_COLUMNS:
    menu_bar->Check(CALCHART__COLUMNS, TRUE);
    break;
  case CC_SELECT_NEAREST:
    menu_bar->Check(CALCHART__NEAREST, TRUE);
    break;
  }

  // Add the controls
  framePanel = new wxPanel(this);
  framePanel->SetAutoLayout(TRUE);

  // Grid choice
  grid_choice = new wxChoice(framePanel, (wxFunction)NULL,
			     "&Grid", -1, -1, -1, -1,
			     sizeof(gridtext)/sizeof(char*),
			     gridtext);
  grid_choice->SetSelection(def_grid);

  // Zoom slider
  SliderWithField *sldr = new SliderWithField(framePanel, slider_zoom_callback,
					      "&Zoom", def_zoom,
					      1, FIELD_MAXZOOM, 150);
  sldr->field = field;
  zoom_slider = sldr;
  wxLayoutConstraints *sl0 = new wxLayoutConstraints;
  sl0->left.AsIs();
  sl0->top.AsIs();
  sl0->right.SameAs(framePanel, wxRight, 5);
  sl0->height.AsIs();
  sldr->SetConstraints(sl0);

  framePanel->NewLine();

  // Reference choice
  {
    wxString buf;
    unsigned i;

    ref_choice = new ChoiceWithField(framePanel, (wxFunction)refnum_callback,
				     "&Ref Group");
    ref_choice->Append("Off");
    for (i = 1; i <= NUM_REF_PNTS; i++) {
      buf.sprintf("%u", i);
      ref_choice->Append(buf.GetData());
    }
  }
  ((ChoiceWithField*)ref_choice)->field = field;
  ref_choice->SetSelection(def_ref);

  // Sheet slider (will get set later with UpdatePanel())
  sldr = new SliderWithField(framePanel, slider_sheet_callback,
			     "&Sheet", 1, 1, 2, 150);
  sldr->field = field;
  sheet_slider = sldr;
  wxLayoutConstraints *sl1 = new wxLayoutConstraints;
  sl1->left.AsIs();
  sl1->top.AsIs();
  sl1->right.SameAs(framePanel, wxRight, 5);
  sl1->height.AsIs();
  sldr->SetConstraints(sl1);

  // Show the frame
  UpdatePanel();
  window_list->Insert(this);
  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  OnSize(-1, -1);
  field->RefreshShow();
  Show(TRUE);
}

MainFrame::~MainFrame() {
  window_list->DeleteObject(this);
  if (node) {
    node->Remove();
    delete node;
  }
}

// Define the behaviour for the frame closing
Bool MainFrame::OnClose(void)
{
  // Save changes first
  if (!OkayToClearShow()) return FALSE;

  return TRUE;
}

// Intercept menu commands
void MainFrame::OnMenuCommand(int id)
{
  char *s;
  MainFrame *frame;
  CC_sheet *sht;
  int sheetnum;

  switch (id) {
  case CALCHART__NEW:
    if (OkayToClearShow()) {
      node->SetShow(new CC_show());
    }
    break;
  case CALCHART__NEW_WINDOW:
    frame = new MainFrame(NULL, 50, 50, window_default_width,
			  window_default_height, NULL, this);
    break;
  case CALCHART__LOAD_FILE:
    LoadShow();
    break;
  case CALCHART__APPEND_FILE:
    AppendShow();
    break;
  case CALCHART__SAVE:
    SaveShow();
    break;
  case CALCHART__SAVE_AS:
    SaveShowAs();
    break;
  case CALCHART__PRINT:
    if (field->show_descr.show) {
      (void)new ShowPrintDialog(&field->show_descr, &node->winlist,
				FALSE, this, "Print show", FALSE);
    }
    break;
  case CALCHART__PRINT_EPS:
    if (field->show_descr.show) {
      (void)new ShowPrintDialog(&field->show_descr, &node->winlist,
				TRUE, this, "Print stuntsheet as EPS", FALSE);
    }
    break;
  case CALCHART__CLOSE:
    Close();
    break;
  case CALCHART__UNDO:
    sheetnum = field->show_descr.show->undolist->Undo(field->show_descr.show);
    if ((sheetnum >= 0) && ((unsigned)sheetnum != field->show_descr.curr_ss))
      field->GotoSS((unsigned)sheetnum);
    break;
  case CALCHART__REDO:
    sheetnum = field->show_descr.show->undolist->Redo(field->show_descr.show);
    if ((sheetnum >= 0) && ((unsigned)sheetnum != field->show_descr.curr_ss))
      field->GotoSS((unsigned)sheetnum);
    break;
  case CALCHART__INSERT_BEFORE:
    sht = new CC_sheet(field->show_descr.CurrSheet());
    field->show_descr.show->UserInsertSheet(sht, field->show_descr.curr_ss);
    field->PrevSS();
    break;
  case CALCHART__INSERT_AFTER:
    sht = new CC_sheet(field->show_descr.CurrSheet());
    field->show_descr.show->UserInsertSheet(sht, field->show_descr.curr_ss+1);
    field->NextSS();
    break;
  case CALCHART__DELETE:
    if (field->show_descr.show->GetNumSheets() > 1) {
      field->show_descr.show->UserDeleteSheet(field->show_descr.curr_ss);
    }
    break;
  case CALCHART__RELABEL:
    if (field->show_descr.curr_ss+1 < field->show_descr.show->GetNumSheets()) {
      if(wxMessageBox("Relabeling sheets is not undoable.\nProceed?",
		      "Relabel sheets", wxYES_NO) == wxYES) {
	if (!field->show_descr.show->RelabelSheets(field->show_descr.curr_ss))
	  (void)wxMessageBox("Stuntsheets don't match",
			     "Relabel sheets");
	else {
	  field->show_descr.show->undolist->EraseAll();
	  field->show_descr.show->SetModified(TRUE);
	}
      }
    } else {
      (void)wxMessageBox("This can't used on the last stuntsheet",
			 "Relabel sheets");
    }
    break;
  case CALCHART__EDIT_CONTINUITY:
    if (field->show_descr.show) {
      (void)new ContinuityEditor(&field->show_descr, &node->winlist, this,
				      "Animation Continuity");
    }
    break;
  case CALCHART__EDIT_PRINTCONT:
    if (field->show_descr.show) {
      (void)new PrintContEditor(&field->show_descr, &node->winlist, this,
				"Printed Continuity");
    }
    break;
  case CALCHART__SET_TITLE:
    if (field->show_descr.show) {
      s = wxGetTextFromUser("Enter the new title",
			    (char *)field->show_descr.CurrSheet()->GetName(),
			    (char *)field->show_descr.CurrSheet()->GetName(),
			    this);
      if (s) {
	field->show_descr.CurrSheet()->UserSetName(s);
      }
    }
    break;
  case CALCHART__SET_BEATS:
    if (field->show_descr.show) {
      wxString buf;
      buf.sprintf("%u", field->show_descr.CurrSheet()->beats);
      s = wxGetTextFromUser("Enter the number of beats",
			    (char *)field->show_descr.CurrSheet()->GetName(),
			    buf.GetData(), this);
      if (s) {
	field->show_descr.CurrSheet()->UserSetBeats(atoi(s));
      }
    }
    break;
  case CALCHART__INFO:
    if (field->show_descr.show)
      (void)new ShowInfoReq(field->show_descr.show, &node->winlist, this,
			    "Information");
    break;
  case CALCHART__POINTS:
    if (field->show_descr.show)
      (void)new PointPicker(field->show_descr.show, &node->winlist,
			     TRUE, this, "Select points");
    break;
  case CALCHART__ANIMATE:
    if (field->show_descr.show) {
      AnimationFrame *anim =
	new AnimationFrame(this, &field->show_descr, &node->winlist);
      anim->canvas->Generate();
    }
    break;
  case CALCHART__ROWS:
  case CALCHART__COLUMNS:
  case CALCHART__NEAREST:
    GetMenuBar()->Check(field->curr_select, FALSE);
    field->curr_select = (CC_SELECT_TYPES)id;
    GetMenuBar()->Check(id, TRUE);
    break;
  case CALCHART__ABOUT:
    (void)wxMessageBox("CalChart v3.0\nAuthor: Gurk Meeker\nhttp://www.calband.berkeley.edu/calchart\n(c) 1994-1996\nCompiled on " __DATE__ " at " __TIME__, "About CalChart");
    break;
  case CALCHART__HELP:
    help_inst->LoadFile();
    help_inst->DisplayContents();
    break;
  }
}

// Intercept menu commands
void MainFrame::OnMenuSelect(int id)
{
  char *msg = NULL;
  switch (id) {
  case CALCHART__NEW:
    msg = "Create a new show";
    break;
  case CALCHART__NEW_WINDOW:
    msg = "Open a new window";
    break;
  case CALCHART__LOAD_FILE:
    msg = "Load a saved show";
    break;
  case CALCHART__APPEND_FILE:
    msg = "Append a show to the end";
    break;
  case CALCHART__SAVE:
    msg = field->show_descr.show->Modified() ?
      "Save show (needed)" :
	"Save show (not needed)";
    break;
  case CALCHART__SAVE_AS:
    msg = "Save show as a new name";
    break;
  case CALCHART__PRINT:
    msg = "Print this show";
    break;
  case CALCHART__PRINT_EPS:
    msg = "Print a stuntsheet in EPS";
    break;
  case CALCHART__CLOSE:
    msg = "Close this window";
    break;
  case CALCHART__UNDO:
    msg = field->show_descr.show->undolist->UndoDescription();
    break;
  case CALCHART__REDO:
    msg = field->show_descr.show->undolist->RedoDescription();
    break;
  case CALCHART__INSERT_BEFORE:
    msg = "Insert a new stuntsheet before this one";
    break;
  case CALCHART__INSERT_AFTER:
    msg = "Insert a new stuntsheet after this one";
    break;
  case CALCHART__DELETE:
    msg = "Delete this stuntsheet";
    break;
  case CALCHART__RELABEL:
    msg = "Relabel all stuntsheets after this one";
    break;
  case CALCHART__EDIT_CONTINUITY:
    msg = "Edit continuity for this stuntsheet";
    break;
  case CALCHART__EDIT_PRINTCONT:
    msg = "Edit printed continuity for this stuntsheet";
    break;
  case CALCHART__SET_TITLE:
    msg = "Change the title of this stuntsheet";
    break;
  case CALCHART__SET_BEATS:
    msg = "Change the number of beats for this stuntsheet";
    break;
  case CALCHART__INFO:
    msg = "Change basic information";
    break;
  case CALCHART__ANIMATE:
    msg = "Open animation window";
    break;
  case CALCHART__ROWS:
    msg = "Select points by rows";
    break;
  case CALCHART__COLUMNS:
    msg = "Select points by columns";
    break;
  case CALCHART__NEAREST:
    msg = "Select points in nearest order";
    break;
  case CALCHART__ABOUT:
    msg = "Information about the program";
    break;
  case CALCHART__HELP:
    msg = "Help on using CalChart";
    break;
  case -1:
    msg = "";
    break;
  }
  if (msg)
    SetStatusText(msg);
}

// Give the use a chance to save the current show
Bool MainFrame::OkayToClearShow() {
  wxString buf;

  if (field->show_descr.show->Modified()) {
    if (!field->show_descr.show->winlist->MultipleWindows()) {
      buf.sprintf("Save changes to '%s'?",
		  field->show_descr.show->UserGetName());
      switch (wxMessageBox(buf.GetData(), "Unsaved changes",
			   wxYES_NO | wxCANCEL, this)) {
      case wxYES:
	SaveShowAs();
	break;
      case wxNO:
	break;
      case wxCANCEL:
	return FALSE;
	break;
      }
    }
  }
  return TRUE;
}

// Load a show with file selector
void MainFrame::LoadShow() {
  const char *s;
  CC_show *shw;

  if (OkayToClearShow()) {
    s = wxFileSelector("Load show", NULL, NULL, NULL, file_wild);
    if (s) {
      shw = new CC_show(s);
      if (shw->Ok()) {
	node->SetShow(shw);
      } else {
	(void)wxMessageBox(shw->GetError(), "Load Error");
	delete shw;
      }
    }
  }
}

// Append a show with file selector
void MainFrame::AppendShow() {
  const char *s;
  CC_show *shw;

  s = wxFileSelector("Append show", NULL, NULL, NULL, file_wild);
  if (s) {
    shw = new CC_show(s);
    if (shw->Ok()) {
      if (shw->GetNumPoints() == field->show_descr.show->GetNumPoints()) {
	field->show_descr.show->undolist->Add(new ShowUndoAppendSheets(field->show_descr.show->GetNumSheets()));
	field->show_descr.show->Append(shw);
      } else {
	(void)wxMessageBox("The blocksize doesn't match", "Append Error");
	delete shw;
      }
    } else {
      (void)wxMessageBox(shw->GetError(), "Load Error");
      delete shw;
    }
  }
}

// Save this show without file selector
void MainFrame::SaveShow() {
  const char *s;

  s = field->show_descr.show->GetName();
  if (strcmp(s, "") == 0) {
    // No file name; use SaveAs instead
    SaveShowAs();
  }
  s = field->show_descr.show->Save(s);
  if (s != NULL) {
    (void)wxMessageBox((char *)s, "Save Error"); // should be const
  } else {
    field->show_descr.show->SetModified(FALSE);
  }
}

// Save this show with file selector
void MainFrame::SaveShowAs() {
  const char *s;
  const char *err;

  s = wxFileSelector("Save show", NULL, NULL, NULL, file_save_wild,
		     wxSAVE | wxOVERWRITE_PROMPT);
  if (s) {
    err = field->show_descr.show->Save(s);
    if (err != NULL) {
      (void)wxMessageBox((char *)err, "Save Error"); // should be const
    } else {
      field->show_descr.show->SetModified(FALSE);
      field->show_descr.show->UserSetName(s);
    }
  }    
}

void MainFrame::SnapToGrid(CC_coord& c) {
  Coord gridc;
  Coord gridmask;
  Coord gridadjust;

  gridc = gridvalue[grid_choice->GetSelection()];
  gridadjust = gridc >> 1; // Half of grid value
  gridmask = ~(gridc-1); // Create mask to snap to this coord

  c.x = (c.x+gridadjust) & gridmask;
  // Adjust so 4 step grid will be on visible grid
  c.y = ((c.y + gridadjust - INT2COORD(2)) & gridmask) + INT2COORD(2);
}

// Define a constructor for field canvas
FieldCanvas::FieldCanvas(CC_show *show, unsigned ss, MainFrame *frame,
			 int def_zoom, FieldCanvas *from_canvas,
			 int x, int y, int w, int h):
 AutoScrollCanvas(frame, x, y, w, h), ourframe(frame), curr_lasso(CC_DRAG_BOX),
 curr_move(CC_MOVE_NORMAL), curr_select(CC_SELECT_ROWS),
 curr_ref(0), drag(CC_DRAG_NONE), dragon(FALSE)
{
  if (from_canvas) {
    curr_lasso = from_canvas->curr_lasso;
    curr_move = from_canvas->curr_move;
    curr_select = from_canvas->curr_select;
  }
  SetColourMap(CalChartColorMap);

  show_descr.show = show;
  show_descr.curr_ss = ss;

  SetZoomQuick(def_zoom);

  SetBackground(CalChartBrushes[COLOR_FIELD]);

  UpdateBars();
}

FieldCanvas::~FieldCanvas(void)
{
}

// Draw the current drag feedback
void FieldCanvas::DrawDrag(Bool on)
{
  wxDC *dc = GetDC();
  Coord w,h;
  CC_coord orig;
  CC_coord origin;

  if (on != dragon) {
    origin = show_descr.show->mode->Offset();
    switch (drag) {
    case CC_DRAG_BOX:
      if (drag_start.x < drag_end.x) {
	orig.x = drag_start.x;
	w = drag_end.x - drag_start.x + 1;
      } else {
	orig.x = drag_end.x;
	w = drag_start.x - drag_end.x + 1;
      }
      if (drag_start.y < drag_end.y) {
	orig.y = drag_start.y;
	h = drag_end.y - drag_start.y + 1;
      } else {
	orig.y = drag_end.y;
	h = drag_start.y - drag_end.y + 1;
      }
      if ((w > 1) && (h > 1)) {
	SetXOR(dc);
	dc->DrawRectangle(orig.x+origin.x+GetPositionX(),
			  orig.y+origin.y+GetPositionY(), w, h);
	dragon = on;
      }
      break;
    case CC_DRAG_LINE:
      SetXOR(dc);
      dc->DrawLine(drag_start.x+origin.x+GetPositionX(),
		   drag_start.y+origin.y+GetPositionY(),
		   drag_end.x+origin.x+GetPositionX(),
		   drag_end.y+origin.y+GetPositionY());
      dragon = on;
      break;
    case CC_DRAG_POLY:
    case CC_DRAG_LASSO:
      lasso.Draw(dc, origin.x+GetPositionX(), origin.y+GetPositionY());
      dragon = on;
    default:
      break;
    }
  }
}

// Define the repainting behaviour
void FieldCanvas::OnPaint(void)
{
  Blit();
  dragon = FALSE; // since the canvas gets cleared
  DrawDrag(TRUE);
}

// Allow clicking within pixels to close polygons
#define CLOSE_ENOUGH_TO_CLOSE 10
void FieldCanvas::OnEvent(wxMouseEvent& event)
{
  float x,y;
  int i;
  CC_coord pos;

  if (show_descr.show) {
    CC_sheet *sheet = show_descr.CurrSheet();
    if (sheet) {
      event.Position(&x, &y);
      Move(x, y);
      Blit();
      dragon = FALSE; // since the canvas gets cleared

      pos = show_descr.show->mode->Offset();
      pos.x = Coord(x - GetPositionX() - pos.x);
      pos.y = Coord(y - GetPositionY() - pos.y);

      if (event.LeftDown()) {
	switch (curr_move) {
	case CC_MOVE_LINE:
	  ourframe->SnapToGrid(pos);
	  BeginDrag(CC_DRAG_LINE, pos);
	  break;
	default:
	  switch (drag) {
	  case CC_DRAG_POLY:
	    {
	      wxPoint *p = lasso.FirstPoint();
	      float d;
	      if (p != NULL) {
		Coord polydist =
		  (Coord)GetDC()->DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
		d = p->x - pos.x;
		if (ABS(d) < polydist) {
		  d = p->y - pos.y;
		  if (ABS(d) < polydist) {
		    EndDrag();
		    SelectWithLasso();
		    break;
		  }
		}
	      }
	      lasso.Append(pos);
	    }
	  break;
	  default:
	    Bool changed = FALSE;
	    if (!event.shiftDown) changed = show_descr.show->UnselectAll();
	    i = sheet->FindPoint(pos.x, pos.y, curr_ref);
	    if (i >= 0) {
	      if (!(show_descr.show->IsSelected(i))) {
		show_descr.show->Select(i);
		changed = TRUE;
	      }
	    }
	    if (changed) {
	      show_descr.show->winlist->UpdateSelections();
	    }
	    if (i < 0) {
	      BeginDrag(curr_lasso, pos);
	    } else {
	      BeginDrag(CC_DRAG_LINE, sheet->GetPosition(i, curr_ref));
	    }
	  }
	  break;
	}
      } else if (event.LeftUp()) {
	switch (curr_move) {
	case CC_MOVE_LINE:
	  EndDrag();
	  if (sheet->MovePointsInLine(drag_start, drag_end, curr_ref))
	    show_descr.show->winlist->
	      UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	  curr_move = CC_MOVE_NORMAL;
	  break;
	default:
	  switch (drag) {
	  case CC_DRAG_BOX:
	    EndDrag();
	    SelectPointsInRect(drag_start, drag_end, curr_ref);
	    break;
	  case CC_DRAG_LINE:
	    EndDrag();
	    pos.x = drag_end.x - drag_start.x;
	    pos.y = drag_end.y - drag_start.y;
	    if (sheet->TranslatePoints(pos, curr_ref))
	      show_descr.show->winlist->
		UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	    break;
	  case CC_DRAG_LASSO:
	    EndDrag();
	    SelectWithLasso();
	    break;
	  default:
	    break;
	  }
	  break;
	}
      } else if (event.RightDown()) {
	switch (drag) {
	case CC_DRAG_POLY:
	  EndDrag();
	  SelectWithLasso();
	  break;
	default:
	  break;
	}
      } else if (event.Dragging() && event.LeftIsDown()) {
	switch (drag) {
	case CC_DRAG_LINE:
	  ourframe->SnapToGrid(pos);
	default:
	  MoveDrag(pos);
	  break;
	}
      } else if (event.Moving()) {
	switch (drag) {
	case CC_DRAG_POLY:
	  MoveDrag(pos);
	  break;
	default:
	  break;
	}
      }
      DrawDrag(TRUE);
    }
  }
}

void FieldCanvas::OnScroll(wxCommandEvent& event)
{
  wxCanvas::OnScroll(event);
}

// Intercept character input
void FieldCanvas::OnChar(wxKeyEvent& event)
{
  // Process the default behaviour
  wxCanvas::OnChar(event);
}

void FieldCanvas::RefreshShow(Bool drawall, int point) {
  if (show_descr.show) {
    CC_sheet *sheet = show_descr.CurrSheet();
    if (sheet) {
      if (curr_ref > 0) {
	sheet->Draw(GetMemDC(), 0, FALSE, drawall, point);
	sheet->Draw(GetMemDC(), curr_ref, TRUE, FALSE, point);
      } else {
	sheet->Draw(GetMemDC(), curr_ref, TRUE, drawall, point);
      }
      Blit();
      dragon = FALSE; // since the canvas gets cleared
      DrawDrag(TRUE);
    }
  }
}

void MainFrame::UpdatePanel() {
  wxString tempbuf;
  CC_sheet *sht = field->show_descr.CurrSheet();
  unsigned num = field->show_descr.show->GetNumSheets();
  unsigned curr = field->show_descr.curr_ss+1;

  tempbuf.sprintf("%s%d of %d \"%.32s\" %d beats",
		  field->show_descr.show->Modified() ? "* ":"", curr,
		  num, sht->GetName(), sht->beats);
  SetStatusText(tempbuf.GetData(), 1);

  if (num > 1) {
    sheet_slider->Enable(TRUE);
    if ((unsigned)sheet_slider->GetMax() != num)
      sheet_slider->SetValue(1); // So Motif doesn't complain about value
      sheet_slider->SetRange(1, num);
    if ((unsigned)sheet_slider->GetValue() != curr)
      sheet_slider->SetValue(curr);
  } else {
    sheet_slider->Enable(FALSE);
  }
}

void FieldCanvas::UpdateBars() {
  if (show_descr.show) {
    SetSize(COORD2INT(show_descr.show->mode->Size().x) * zoomf,
	    COORD2INT(show_descr.show->mode->Size().y) * zoomf);
  }
}

void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, CC_coord start) {
  DrawDrag(FALSE);
  drag = type;
  switch (drag) {
  case CC_DRAG_POLY:
    lasso.Clear();
    lasso.Append(start);
    lasso.Append(start);
    break;
  case CC_DRAG_LASSO:
    lasso.Clear();
    lasso.Append(start);
    break;
  default:
    drag_start = drag_end = start;
  }
  DrawDrag(TRUE);
}

void FieldCanvas::MoveDrag(CC_coord end) {
  DrawDrag(FALSE);
  switch (drag) {
  case CC_DRAG_POLY:
    lasso.Drag(end);
    break;
  case CC_DRAG_LASSO:
    lasso.Append(end);
    break;
  default:
    drag_end = end;
  }
  DrawDrag(TRUE);
}

void FieldCanvas::EndDrag() {
  DrawDrag(FALSE);
  switch (drag) {
  case CC_DRAG_LASSO:
    lasso.End();
    break;
  default:
    break;
  }
  drag = CC_DRAG_NONE;
}

void FieldCanvas::SelectOrdered(wxList& pointlist,
				const CC_coord& start) {
  wxNode *pnt, *n;
  CC_coord c1, c2, last;
  Coord v1, v2;
  float f1, f2, fx, fy;
  CC_sheet* sheet = show_descr.CurrSheet();

  last = start;
  while ((pnt = pointlist.First()) != NULL) {
    c1 = sheet->GetPosition((unsigned)pnt->key.integer, curr_ref);
    for (n = pnt->Next(); n != NULL; n = n->Next()) {
      switch (curr_select) {
      case CC_SELECT_ROWS:
	v1 = ABS(start.y - c1.y);
	c2 = sheet->GetPosition((unsigned)n->key.integer, curr_ref);
	v2 = ABS(start.y - c2.y);
	if (v2 < v1) {
	  pnt = n;
	  c1 = c2;
	} else if ((v2 == v1) && ((c1.y == c2.y) || (c2.y < start.y))) {
	  // make sure we keep rows together
	  v1 = ABS(start.x - c1.x);
	  v2 = ABS(start.x - c2.x);
	  if (v2 < v1) {
	    pnt = n;
	    c1 = c2;
	  }
	}
	break;
      case CC_SELECT_COLUMNS:
	v1 = ABS(start.x - c1.x);
	c2 = sheet->GetPosition((unsigned)n->key.integer, curr_ref);
	v2 = ABS(start.x - c2.x);
	if (v2 < v1) {
	  pnt = n;
	  c1 = c2;
	} else if ((v2 == v1) && ((c1.x == c2.x) || (c2.x < start.x))) {
	  // make sure we keep columns together
	  v1 = ABS(start.y - c1.y);
	  v2 = ABS(start.y - c2.y);
	  if (v2 < v1) {
	    pnt = n;
	    c1 = c2;
	  }
	}
	break;
      case CC_SELECT_NEAREST:
	fx = (float)(last.x - c1.x);
	fy = (float)(last.y - c1.y);
	f1 = fx*fx+fy*fy;
	c2 = sheet->GetPosition((unsigned)n->key.integer, curr_ref);
	fx = (float)(last.x - c2.x);
	fy = (float)(last.y - c2.y);
	f2 = fx*fx+fy*fy;
	if (f2 < f1) {
	  pnt = n;
	  c1 = c2;
	}
	break;
      }
    }
    show_descr.show->Select((unsigned)pnt->key.integer);
    last = c1;
    pointlist.DeleteNode(pnt);
  }
}

Bool FieldCanvas::SelectWithLasso() {
  Bool changed = FALSE;
  CC_sheet* sheet = show_descr.CurrSheet();
  wxList pointlist;
  wxPoint *pnt;

  for (unsigned i = 0; i < show_descr.show->GetNumPoints(); i++) {
    if (lasso.Inside(sheet->GetPosition(i, curr_ref))) {
      changed = TRUE;
      pointlist.Append(i, NULL);
    }
  }
  pnt = lasso.FirstPoint();
  if (changed && pnt) {
    SelectOrdered(pointlist, CC_coord((Coord)pnt->x, (Coord)pnt->y));
    show_descr.show->winlist->UpdateSelections();
  }
  lasso.Clear();

  return changed;
}

// Select points within rectangle
Bool FieldCanvas::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
				     unsigned ref) {
  unsigned i;
  Bool changed = FALSE;
  CC_sheet* sheet = show_descr.CurrSheet();
  CC_coord top_left, bottom_right;
  const CC_coord *pos;
  wxList pointlist;

  if (c1.x > c2.x) {
    top_left.x = c2.x;
    bottom_right.x = c1.x;
  } else {
    top_left.x = c1.x;
    bottom_right.x = c2.x;
  }
  if (c1.y > c2.y) {
    top_left.y = c2.y;
    bottom_right.y = c1.y;
  } else {
    top_left.y = c1.y;
    bottom_right.y = c2.y;
  }
  for (i = 0; i < show_descr.show->GetNumPoints(); i++) {
    pos = &sheet->GetPosition(i, ref);
    if ((pos->x >= top_left.x) && (pos->x <= bottom_right.x) &&
	(pos->y >= top_left.y) && (pos->y <= bottom_right.y)) {
      if (!show_descr.show->IsSelected(i)) {
	pointlist.Append(i, NULL);
	changed = TRUE;
      }
    }
  }
  if (changed) {
    SelectOrdered(pointlist, c1);
    show_descr.show->winlist->UpdateSelections();
  }

  return changed;
}

Bool MainFrameList::CloseAllWindows() {
  wxNode *node, *node_tmp;
  MainFrame *mf;

  for (node = First(); node != NULL; ) {
    mf = (MainFrame *)node->Data();
    // This node will be deleted by the window's deconstructor
    node_tmp = node->Next();
    if (!mf->Close()) return FALSE;
    node = node_tmp;
  }
  return TRUE;
}

static void toolbar_prev_ss(CoolToolBar *tb) {
  ((MainFrame *)tb->ourframe)->field->PrevSS();
}

static void toolbar_next_ss(CoolToolBar *tb) {
  ((MainFrame *)tb->ourframe)->field->NextSS();
}

static void toolbar_box(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->field->curr_lasso = CC_DRAG_BOX;
}

static void toolbar_poly(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->field->curr_lasso = CC_DRAG_POLY;
}

static void toolbar_lasso(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->field->curr_lasso = CC_DRAG_LASSO;
}

static void toolbar_line(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->field->curr_move = CC_MOVE_LINE;
}

static void toolbar_label_left(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsLabel(FALSE))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_label_right(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsLabel(TRUE))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_label_flip(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsLabelFlip())
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym0(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_PLAIN))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym1(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOL))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym2(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_BKSL))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym3(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SL))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym4(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_X))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym5(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLBKSL))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym6(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLSL))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void toolbar_setsym7(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  if (mf->field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLX))
    mf->field->show_descr.show->winlist->
      UpdatePointsOnSheet(mf->field->show_descr.curr_ss);
}

static void refnum_callback(wxObject &obj, wxEvent &) {
  ChoiceWithField *choice = (ChoiceWithField *)&obj;
  choice->field->curr_ref = (unsigned)choice->GetSelection();
  choice->field->RefreshShow();
}

static void slider_sheet_callback(wxObject &obj, wxEvent &) {
  SliderWithField *slider = (SliderWithField *)&obj;
  slider->field->GotoSS(slider->GetValue()-1);
}

static void slider_zoom_callback(wxObject &obj, wxEvent &) {
  SliderWithField *slider = (SliderWithField *)&obj;
  slider->field->SetZoom(slider->GetValue());
}
