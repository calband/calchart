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
#include "ccvers.h"

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
#include "tb_mv.xbm"
#include "tb_line.xbm"
#include "tb_rot.xbm"
#include "tb_shr.xbm"
#include "tb_ref.xbm"
#include "tb_siz.xbm"
#include "tb_gen.xbm"
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
static void toolbar_move(CoolToolBar *tb);
static void toolbar_line(CoolToolBar *tb);
static void toolbar_rot(CoolToolBar *tb);
static void toolbar_shear(CoolToolBar *tb);
static void toolbar_reflect(CoolToolBar *tb);
static void toolbar_size(CoolToolBar *tb);
static void toolbar_genius(CoolToolBar *tb);
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
  { TOOLBAR_TOGGLE, NULL, "Select points with box", toolbar_box },
  { TOOLBAR_TOGGLE, NULL, "Select points with polygon", toolbar_poly },
  { TOOLBAR_SPACE | TOOLBAR_TOGGLE, NULL,
    "Select points with lasso", toolbar_lasso },
  { TOOLBAR_TOGGLE, NULL, "Translate points", toolbar_move },
  { TOOLBAR_TOGGLE, NULL, "Move points into line", toolbar_line },
  { TOOLBAR_TOGGLE, NULL, "Rotate block", toolbar_rot },
  { TOOLBAR_TOGGLE, NULL, "Shear block", toolbar_shear },
  { TOOLBAR_TOGGLE, NULL, "Reflect block", toolbar_reflect },
  { TOOLBAR_TOGGLE, NULL, "Resize block", toolbar_size },
  { TOOLBAR_SPACE | TOOLBAR_TOGGLE, NULL, "Genius move", toolbar_genius },
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

#define TOOLBAR_BOX 2
#define TOOLBAR_TRANS 5

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

TopFrame *topframe = NULL;

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
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_mv));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_line));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_rot));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_shr));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_ref));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_siz));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_gen));
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

  topframe = new TopFrame(300, 100);
  topframe->Maximize(TRUE);
  for (i = 1; i < realargc; i++) {
    CC_show *shw;

    shw = new CC_show(argv[i]);
    if (shw->Ok()) {
      topframe->NewShow(shw);
    } else {
      (void)wxMessageBox((char*)shw->GetError(), "Load Error");
      delete shw;
    }
  }
  if (!shows_dir.Empty()) {
    wxSetWorkingDirectory(shows_dir.GetData());
  }

  SetAutoSave(autosave_interval);

  return topframe;
}

int CalChartApp::OnExit(void) {
  if (modelist) delete modelist;
  if (help_inst) delete help_inst;
  if (window_list) delete window_list;

  return 0;
}

CC_shape::CC_shape() {}
CC_shape::~CC_shape() {}

CC_shape_1point::CC_shape_1point(const CC_coord& p)
  : origin(p.x, p.y) {}

void CC_shape_1point::OnMove(const CC_coord& p, MainFrame *) {
  MoveOrigin(p);
}

void CC_shape_1point::MoveOrigin(const CC_coord& p) {
  origin.x = p.x;
  origin.y = p.y;
}

CC_coord CC_shape_1point::GetOrigin() const {
  return CC_coord(Coord(origin.x), Coord(origin.y));
}

CC_shape_cross::CC_shape_cross(const CC_coord& p, Coord width)
  : CC_shape_1point(p), cross_width(width) {}

void CC_shape_cross::OnMove(const CC_coord& p, MainFrame *frame) {
  CC_coord p1 = p;

  frame->SnapToGrid(p1);
  MoveOrigin(p1);
}

void CC_shape_cross::Draw(wxDC *dc, float x, float y) const {
  dc->DrawLine(origin.x + x - cross_width, origin.y + y - cross_width,
	       origin.x + x + cross_width, origin.y + y + cross_width);
  dc->DrawLine(origin.x + x + cross_width, origin.y + y - cross_width,
	       origin.x + x - cross_width, origin.y + y + cross_width);
}

CC_shape_2point::CC_shape_2point(const CC_coord& p)
  : CC_shape_1point(p), point(p.x, p.y) {}

CC_shape_2point::CC_shape_2point(const CC_coord& p1, const CC_coord& p2)
  : CC_shape_1point(p1), point(p2.x, p2.y) {}

void CC_shape_2point::OnMove(const CC_coord& p, MainFrame *) {
  MovePoint(p);
}

void CC_shape_2point::MovePoint(const CC_coord& p) {
  point.x = p.x;
  point.y = p.y;
}

CC_coord CC_shape_2point::GetPoint() const {
  return CC_coord(Coord(point.x), Coord(point.y));
}

CC_shape_line::CC_shape_line(const CC_coord& p)
  : CC_shape_2point(p) {}

CC_shape_line::CC_shape_line(const CC_coord& p1, const CC_coord& p2)
  : CC_shape_2point(p1, p2) {}

void CC_shape_line::OnMove(const CC_coord& p, MainFrame *frame) {
  CC_coord p1 = p;

  frame->SnapToGrid(p1);
  MovePoint(p1);
}

void CC_shape_line::Draw(wxDC *dc, float x, float y) const {
  dc->DrawLine(origin.x + x, origin.y + y,
	       point.x + x, point.y + y);
}

CC_shape_angline::CC_shape_angline(const CC_coord& p, const CC_coord& refvect)
  : CC_shape_line(p), vect(refvect), mag(refvect.Magnitude()) {
}

void CC_shape_angline::OnMove(const CC_coord& p, MainFrame *frame) {
  CC_coord o = GetOrigin();
  CC_coord p1 = p;
  float d;

  frame->SnapToGrid(p1);
  p1 -= o;
  d = (COORD2FLOAT(p1.x) * COORD2FLOAT(vect.x) +
       COORD2FLOAT(p1.y) * COORD2FLOAT(vect.y)) / (mag*mag);
  p1.x = (Coord)(o.x + vect.x * d);
  p1.y = (Coord)(o.y + vect.y * d);
  MovePoint(p1);
}

CC_shape_arc::CC_shape_arc(const CC_coord& c, const CC_coord& p)
  : CC_shape_2point(c, p) {
    CC_coord p1 = p-c;

    r = r0 = p1.Direction()*PI/180.0;
    d = p1.Magnitude()*COORD_DECIMAL;
}

CC_shape_arc::CC_shape_arc(const CC_coord& c,
			   const CC_coord& p1, const CC_coord& p2)
  : CC_shape_2point(c, p2) {
    CC_coord p = p1-c;

    r0 = p.Direction();
    d = p.Magnitude()*COORD_DECIMAL;
    r = (p2-c).Direction();
}

void CC_shape_arc::OnMove(const CC_coord& p, MainFrame *frame) {
  CC_coord p1 = p;

  frame->SnapToGrid(p1);
  r = GetOrigin().Direction(p1)*PI/180.0;
  p1.x = Coord(origin.x + d*cos(r));
  p1.y = Coord(origin.y + -d*sin(r));
  MovePoint(p1);
}

void CC_shape_arc::Draw(wxDC *dc, float x, float y) const {
  // DrawArc always goes counterclockwise
  if (GetAngle() < 0.0 || GetAngle() > 180.0) {
    dc->DrawArc(origin.x + x + d*cos(r), origin.y + y + -d*sin(r),
		origin.x + x + d*cos(r0), origin.y + y + -d*sin(r0),
		origin.x + x, origin.y + y);
  } else {
    dc->DrawArc(origin.x + x + d*cos(r0), origin.y + y + -d*sin(r0),
		origin.x + x + d*cos(r), origin.y + y + -d*sin(r),
		origin.x + x, origin.y + y);
  }
}

CC_shape_rect::CC_shape_rect(const CC_coord& p)
  : CC_shape_2point(p) {}

CC_shape_rect::CC_shape_rect(const CC_coord& p1, const CC_coord& p2)
  : CC_shape_2point(p1, p2) {}

void CC_shape_rect::Draw(wxDC *dc, float x, float y) const {
  float w, h;

  if (origin.x < point.x) {
    x += origin.x;
    w = point.x - origin.x + 1;
  } else {
    x += point.x;
    w = origin.x - point.x + 1;
  }
  if (origin.y < point.y) {
    y += origin.y;
    h = point.y - origin.y + 1;
  } else {
    y += point.y;
    h = origin.y - point.y + 1;
  }
  if ((w > 1) && (h > 1)) {
    dc->DrawRectangle(x, y, w, h);
  }
}

CC_lasso::CC_lasso(const CC_coord &p) {
  Append(p);
}

CC_lasso::~CC_lasso() {
  Clear();
}

void CC_lasso::OnMove(const CC_coord& p, MainFrame *) {
  Append(p);
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
Bool CC_lasso::Inside(const CC_coord& p) const {
  Bool parity = FALSE;
  wxNode *last;
  wxNode *n = ((wxList*)&pntlist)->First();
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

void CC_lasso::Draw(wxDC *dc, float x, float y) const {
  dc->DrawLines((wxList*)&pntlist, x, y);
}

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
			   const CC_coord& p) const {
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

CC_poly::CC_poly(const CC_coord &p)
  : CC_lasso(p) {
    // add end point
    Append(p);
}

void CC_poly::OnMove(const CC_coord& p, MainFrame *) {
  Drag(p);
}

#ifndef CC_USE_MDI
static void new_window(wxButton& button, wxEvent&) {
  TopFrame *f = (TopFrame*)button.GetParent()->GetParent();
  f->NewShow();
}

static void open_window(wxButton& button, wxEvent&) {
  TopFrame *f = (TopFrame*)button.GetParent()->GetParent();
  f->OpenShow();
}

static void quit_callback(wxButton &button, wxEvent &) {
  TopFrame *f = (TopFrame*)button.GetParent()->GetParent();
  f->Quit();
}
#endif

TopFrame::TopFrame(int width, int height):
  wxFrame(NULL, "CalChart", 0, 0, width, height, CC_FRAME_TOP) {
  // Give it an icon
  SetBandIcon(this);

#ifdef CC_USE_MDI
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(CALCHART__NEW, "&New Show");
  file_menu->Append(CALCHART__LOAD_FILE, "&Open...");
  file_menu->Append(CALCHART__QUIT, "&Quit");

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(CALCHART__ABOUT, "&About CalChart...");
  help_menu->Append(CALCHART__HELP, "&Help on CalChart...");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, "&File");
  menu_bar->Append(help_menu, "&Help");
  SetMenuBar(menu_bar);
#else
  int w, h;

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
#endif
  DragAcceptFiles(TRUE);
  Show(TRUE);
}

TopFrame::~TopFrame() {
}

Bool TopFrame::OnClose(void) {
  return window_list->CloseAllWindows();
}

#ifdef CC_USE_MDI
void TopFrame::OnMenuCommand(int id) {
  switch (id) {
  case CALCHART__NEW:
    NewShow();
    break;
  case CALCHART__LOAD_FILE:
    OpenShow();
    break;
  case CALCHART__QUIT:
    Quit();
    break;
  case CALCHART__ABOUT:
    About();
    break;
  case CALCHART__HELP:
    Help();
    break;
  }
}

void TopFrame::OnMenuSelect(int id) {
}
#endif

void TopFrame::OnDropFiles(int n, char *files[], int, int) {
  for (int i = 0; i < n; i++) {
    OpenShow(files[i]);
  }
}

void TopFrame::NewShow(CC_show *shw) {
  (void)new MainFrame(this, 50, 50,
		      window_default_width, window_default_height, shw);
}

void TopFrame::OpenShow(const char *filename) {
  CC_show *shw;

  if (filename == NULL)
    filename = wxFileSelector("Load show", NULL, NULL, NULL, file_wild);
  if (filename) {
    shw = new CC_show(filename);
    if (shw->Ok()) {
      NewShow(shw);
    } else {
      (void)wxMessageBox((char*)shw->GetError(), "Load Error");
      delete shw;
    }
  }
}

void TopFrame::Quit() {
  Close();
}

void TopFrame::About() {
  (void)wxMessageBox("CalChart " CC_VERSION "\nAuthor: Gurk Meeker\nhttp://www.calband.berkeley.edu/calchart\n(c) 1994-1998\nCompiled on " __DATE__ " at " __TIME__, "About CalChart");
}

void TopFrame::Help() {
  help_inst->LoadFile();
  help_inst->DisplayContents();
}

// Main frame constructor
MainFrame::MainFrame(wxFrame *frame, int x, int y, int w, int h,
		     CC_show *show, MainFrame *other_frame):
  wxFrameWithStuff(frame, "CalChart", x, y, w, h, CC_FRAME_CHILD),
  field(NULL)
{
  unsigned ss;
  unsigned def_zoom;
  unsigned def_grid;
  unsigned def_ref;
  Bool setup;

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
  file_menu->Append(CALCHART__IMPORT_CONT_FILE, "&Import Continuity...");
  file_menu->Append(CALCHART__SAVE, "&Save");
  file_menu->Append(CALCHART__SAVE_AS, "Save &As...");
  file_menu->Append(CALCHART__PRINT, "&Print...");
  file_menu->Append(CALCHART__PRINT_EPS, "Print &EPS...");
  file_menu->Append(CALCHART__CLOSE, "&Close Window");
  file_menu->Append(CALCHART__QUIT, "&Quit");

  wxMenu *edit_menu = new wxMenu;
  edit_menu->Append(CALCHART__UNDO, "&Undo");
  edit_menu->Append(CALCHART__REDO, "&Redo");
  edit_menu->Append(CALCHART__INSERT_BEFORE, "&Insert Sheet Before");
  edit_menu->Append(CALCHART__INSERT_AFTER, "Insert Sheet &After");
  edit_menu->Append(CALCHART__DELETE, "&Delete Sheet");
  edit_menu->Append(CALCHART__RELABEL, "&Relabel Sheets");
  edit_menu->Append(CALCHART__CLEAR_REF, "&Clear Reference");
  edit_menu->Append(CALCHART__SETUP, "&Setup Show...");
  edit_menu->Append(CALCHART__POINTS, "&Point Selections...");
  edit_menu->Append(CALCHART__SET_TITLE, "Set &Title...");
  edit_menu->Append(CALCHART__SET_BEATS, "Set &Beats...");

  wxMenu *anim_menu = new wxMenu;
  anim_menu->Append(CALCHART__EDIT_CONTINUITY, "&Edit Continuity...");
  anim_menu->Append(CALCHART__EDIT_PRINTCONT, "Edit &Printed Continuity...");
  anim_menu->Append(CALCHART__ANIMATE, "&Animate...");

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
  menu_bar->Append(anim_menu, "&Animation");
  menu_bar->Append(options_menu, "&Options");
  menu_bar->Append(help_menu, "&Help");
  SetMenuBar(menu_bar);

  // Add a toolbar
  CoolToolBar *ribbon = new CoolToolBar(this, 0, 0, -1, -1, 0,
					wxHORIZONTAL, 23);
  ribbon->SetupBar(main_tb, sizeof(main_tb)/sizeof(ToolBarEntry));
  SetToolBar(ribbon);

  // Add the field canvas
  setup = FALSE;
  if (!other_frame) {
    if (!show) {
      show = new CC_show();
      setup = TRUE;
    }
    ss = 0;
    def_zoom = default_zoom;
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

  // Update the tool bar
  SetCurrentLasso(field->curr_lasso);
  SetCurrentMove(field->curr_move);

  // Show the frame
  UpdatePanel();
  window_list->Insert(this);
  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  OnSize(-1, -1);
  field->RefreshShow();
  Show(TRUE);

  if (setup) Setup();
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
    topframe->NewShow();
    break;
  case CALCHART__NEW_WINDOW:
    frame = new MainFrame(topframe, 50, 50, window_default_width,
			  window_default_height, NULL, this);
    break;
  case CALCHART__LOAD_FILE:
    topframe->OpenShow();
    break;
  case CALCHART__APPEND_FILE:
    AppendShow();
    break;
  case CALCHART__IMPORT_CONT_FILE:
    ImportContFile();
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
  case CALCHART__QUIT:
    topframe->Quit();
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
  case CALCHART__CLEAR_REF:
    if (field->curr_ref > 0) {
      if (field->show_descr.CurrSheet()->ClearRefPositions(field->curr_ref))
	field->show_descr.show->winlist->
	  UpdatePointsOnSheet(field->show_descr.curr_ss, field->curr_ref);
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
      buf.sprintf("%u", field->show_descr.CurrSheet()->GetBeats());
      s = wxGetTextFromUser("Enter the number of beats",
			    (char *)field->show_descr.CurrSheet()->GetName(),
			    buf.GetData(), this);
      if (s) {
	field->show_descr.CurrSheet()->UserSetBeats(atoi(s));
      }
    }
    break;
  case CALCHART__SETUP:
    Setup();
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
    topframe->About();
    break;
  case CALCHART__HELP:
    topframe->Help();
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
  case CALCHART__IMPORT_CONT_FILE:
    msg = "Import continuity text";
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
  case CALCHART__QUIT:
    msg = "Quit CalChart";
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
  case CALCHART__CLEAR_REF:
    msg = "Clear selected reference points";
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
  case CALCHART__SETUP:
    msg = "Setup basic show information";
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
    field->show_descr.show->ClearAutosave();
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
	(void)wxMessageBox((char*)shw->GetError(), "Load Error");
	delete shw;
      }
    }
  }
}

// Append a show with file selector
void MainFrame::AppendShow() {
  const char *s;
  CC_show *shw;
  unsigned currend;

  s = wxFileSelector("Append show", NULL, NULL, NULL, file_wild);
  if (s) {
    shw = new CC_show(s);
    if (shw->Ok()) {
      if (shw->GetNumPoints() == field->show_descr.show->GetNumPoints()) {
	currend = field->show_descr.show->GetNumSheets();
	field->show_descr.show->undolist->Add(new ShowUndoAppendSheets(currend));
	field->show_descr.show->Append(shw);
	if (!field->show_descr.show->RelabelSheets(currend-1))
	  (void)wxMessageBox("Stuntsheets don't match",
			     "Append Error");
      } else {
	(void)wxMessageBox("The blocksize doesn't match", "Append Error");
	delete shw;
      }
    } else {
      (void)wxMessageBox((char*)shw->GetError(), "Load Error");
      delete shw;
    }
  }
}

// Append a show with file selector
void MainFrame::ImportContFile() {
  const char *s;
  wxString *err;

  s = wxFileSelector("Import Continuity", NULL, NULL, NULL, "*.txt");
  if (s) {
    err = field->show_descr.show->ImportContinuity(s);
    if (err) {
      (void)wxMessageBox(err->GetData(), "Load Error");
      delete err;
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
  } else {
    s = field->show_descr.show->Save(s);
    if (s != NULL) {
      (void)wxMessageBox((char *)s, "Save Error"); // should be const
    }
  }
}

// Save this show with file selector
void MainFrame::SaveShowAs() {
  const char *s;
  const char *err;

  s = wxFileSelector("Save show", NULL, NULL, NULL, file_save_wild,
		     wxSAVE | wxOVERWRITE_PROMPT);
  if (s) {
    wxString str(s);
    unsigned int i = str.Length();
    if ((i < 4) ||
	(str.SubString(i-4,i-1).CompareTo(".shw", wxString::ignoreCase)!=0)) {
      str.Append(".shw");
    }
    err = field->show_descr.show->Save(str);
    if (err != NULL) {
      (void)wxMessageBox((char *)err, "Save Error"); // should be const
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

void MainFrame::SetCurrentLasso(CC_DRAG_TYPES type) {
  if (field->curr_lasso != CC_DRAG_NONE) {
    frameToolBar->ToggleTool(TOOLBAR_BOX+field->curr_lasso-CC_DRAG_BOX, FALSE);
  }
  field->curr_lasso = type;
  if (field->curr_lasso != CC_DRAG_NONE) {
    frameToolBar->ToggleTool(TOOLBAR_BOX + type - CC_DRAG_BOX, TRUE);
  }
}

void MainFrame::SetCurrentMove(CC_MOVE_MODES type) {
  frameToolBar->ToggleTool(TOOLBAR_TRANS + field->curr_move - CC_MOVE_NORMAL,
			   FALSE);
  field->curr_move = type;
  frameToolBar->ToggleTool(TOOLBAR_TRANS + type - CC_MOVE_NORMAL, TRUE);
  field->EndDrag();
}

void MainFrame::Setup() {
  if (field->show_descr.show)
    (void)new ShowInfoReq(field->show_descr.show, &node->winlist, this,
			  "Setup Show");
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

FieldCanvas::~FieldCanvas(void) {
  ClearShapes();
}

void FieldCanvas::ClearShapes() {
  for (wxNode *n = shape_list.First(); n != NULL; n = n->Next()) {
    delete (CC_shape*)(n->Data());
  }
  shape_list.Clear();
  curr_shape = NULL;
}

// Draw the current drag feedback
void FieldCanvas::DrawDrag(Bool on)
{
  wxDC *dc = GetDC();
  CC_coord origin;

  if ((on != dragon) && curr_shape) {
    dragon = on;
    if (on) {
      SetXOR(dc);
      origin = show_descr.show->mode->Offset();
      for (wxNode *n = shape_list.First(); n != NULL; n = n->Next()) {
	((CC_shape*)(n->Data()))->Draw(dc, origin.x+GetPositionX(),
				       origin.y+GetPositionY());
      }
    } else {
      Blit();
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
      if (event.ControlDown()) {
	Move(x, y);
	Blit();
	dragon = FALSE; // since the canvas gets cleared
      } else {
	Move(x, y, 1);
      }

      pos = show_descr.show->mode->Offset();
      pos.x = Coord(x - GetPositionX() - pos.x);
      pos.y = Coord(y - GetPositionY() - pos.y);

      if (event.LeftDown()) {
	switch (curr_move) {
	case CC_MOVE_LINE:
	  ourframe->SnapToGrid(pos);
	  BeginDrag(CC_DRAG_LINE, pos);
	  break;
	case CC_MOVE_ROTATE:
	  ourframe->SnapToGrid(pos);
	  if (curr_shape &&
	      (((CC_shape_1point*)curr_shape)->GetOrigin() != pos)) {
	    AddDrag(CC_DRAG_LINE,
		    new CC_shape_arc(((CC_shape_1point*)
				      curr_shape)->GetOrigin(), pos));
	  } else {
	    BeginDrag(CC_DRAG_CROSS, pos);
	  }
	  break;
	case CC_MOVE_SHEAR:
	  ourframe->SnapToGrid(pos);
	  if (curr_shape &&
	      (((CC_shape_1point*)curr_shape)->GetOrigin() != pos)) {
	    CC_coord vect(pos - ((CC_shape_1point*)curr_shape)->GetOrigin());
	    // rotate vect 90 degrees
	    AddDrag(CC_DRAG_LINE,
		    new CC_shape_angline(pos,CC_coord(-vect.y, vect.x)));
	  } else {
	    BeginDrag(CC_DRAG_CROSS, pos);
	  }
	  break;
	case CC_MOVE_REFL:
	  ourframe->SnapToGrid(pos);
	  BeginDrag(CC_DRAG_LINE, pos);
	  break;
	case CC_MOVE_SIZE:
	  ourframe->SnapToGrid(pos);
	  if (curr_shape &&
	      (((CC_shape_1point*)curr_shape)->GetOrigin() != pos)) {
	    AddDrag(CC_DRAG_LINE, new CC_shape_line(pos));
	  } else {
	    BeginDrag(CC_DRAG_CROSS, pos);
	  }
	  break;
	case CC_MOVE_GENIUS:
	  ourframe->SnapToGrid(pos);
	  AddDrag(CC_DRAG_LINE, new CC_shape_line(pos));
	  break;
	default:
	  switch (drag) {
	  case CC_DRAG_POLY:
	    {
	      const wxPoint *p = ((CC_lasso*)curr_shape)->FirstPoint();
	      float d;
	      if (p != NULL) {
		Coord polydist =
		  (Coord)GetDC()->DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
		d = p->x - pos.x;
		if (ABS(d) < polydist) {
		  d = p->y - pos.y;
		  if (ABS(d) < polydist) {
		    SelectWithLasso((CC_lasso*)curr_shape);
		    EndDrag();
		    break;
		  }
		}
	      }
	      ((CC_lasso*)curr_shape)->Append(pos);
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
      } else if (event.LeftUp() && curr_shape) {
	const CC_shape_2point *shape = (CC_shape_2point*)curr_shape;
	const CC_shape_1point *origin;
	switch (curr_move) {
	case CC_MOVE_LINE:
	  if (sheet->MovePointsInLine(shape->GetOrigin(), shape->GetPoint(),
				      curr_ref))
	    show_descr.show->winlist->
	      UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	  EndDrag();
	  ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	  break;
	case CC_MOVE_ROTATE:
	  if (shape_list.Number() > 1) {
	    origin = (CC_shape_1point*)shape_list.First()->Data();
	    if (shape->GetOrigin() == shape->GetPoint()) {
	      BeginDrag(CC_DRAG_CROSS, pos);
	    } else {
	      Matrix m;
	      CC_coord c1 = origin->GetOrigin();
	      float r = -((CC_shape_arc*)curr_shape)->GetAngle();

	      m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
		ZRotationMatrix(r) *
		TranslationMatrix(Vector(c1.x, c1.y, 0));
	      if (sheet->TransformPoints(m, curr_ref))
		show_descr.show->winlist->
		  UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	      EndDrag();
	      ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	    }
	  }
	  break;
	case CC_MOVE_SHEAR:
	  if (shape_list.Number() > 1) {
	    origin = (CC_shape_1point*)shape_list.First()->Data();
	    if (shape->GetOrigin() == shape->GetPoint()) {
	      BeginDrag(CC_DRAG_CROSS, pos);
	    } else {
	      Matrix m;
	      CC_coord o = origin->GetOrigin();
	      CC_coord c1 = shape->GetOrigin();
	      CC_coord c2 = shape->GetPoint();
	      CC_coord v1, v2;
	      float ang, amount;
	      
	      v1 = c1 - o;
	      v2 = c2 - c1;
	      amount = v2.Magnitude() / v1.Magnitude();
	      if (BoundDirectionSigned(v1.Direction() -
				       (c2-o).Direction()) < 0)
		amount = -amount;
	      ang = -v1.Direction()*PI/180.0;
	      m = TranslationMatrix(Vector(-o.x, -o.y, 0)) *
		ZRotationMatrix(-ang) *
		YXShearMatrix(amount) *
		ZRotationMatrix(ang) *
		TranslationMatrix(Vector(o.x, o.y, 0));
	      if (sheet->TransformPoints(m, curr_ref))
		show_descr.show->winlist->
		  UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	      EndDrag();
	      ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	    }
	  }
	  break;
	case CC_MOVE_REFL:
	  if (shape->GetOrigin() != shape->GetPoint()) {
	    Matrix m;
	    CC_coord c1 = shape->GetOrigin();
	    CC_coord c2;
	    float ang;
	    
	    c2 = shape->GetPoint() - c1;
	    ang = -c2.Direction()*PI/180.0;
	    m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
	      ZRotationMatrix(-ang) *
	      YReflectionMatrix() *
	      ZRotationMatrix(ang) *
	      TranslationMatrix(Vector(c1.x, c1.y, 0));
	    if (sheet->TransformPoints(m, curr_ref))
	      show_descr.show->winlist->
		UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	  }
	  EndDrag();
	  ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	  break;
	case CC_MOVE_SIZE:
	  if (shape_list.Number() > 1) {
	    origin = (CC_shape_1point*)shape_list.First()->Data();
	    if (shape->GetOrigin() == shape->GetPoint()) {
	      BeginDrag(CC_DRAG_CROSS, pos);
	    } else {
	      Matrix m;
	      CC_coord c1 = origin->GetOrigin();
	      CC_coord c2;
	      float sx, sy;

	      c2 = shape->GetPoint() - c1;
	      sx = c2.x;
	      sy = c2.y;
	      c2 = shape->GetOrigin() - c1;
	      if ((c2.x != 0) || (c2.y != 0)) {
		if (c2.x != 0) {
		  sx /= c2.x;
		} else {
		  sx = 1;
		}
		if (c2.y != 0) {
		  sy /= c2.y;
		} else {
		  sy = 1;
		}
		m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) *
		  ScaleMatrix(Vector(sx, sy, 0)) *
		  TranslationMatrix(Vector(c1.x, c1.y, 0));
		if (sheet->TransformPoints(m, curr_ref))
		  show_descr.show->winlist->
		    UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	      }
	      EndDrag();
	      ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	    }
	  }
	  break;
	case CC_MOVE_GENIUS:
	  if (shape_list.Number() >= 3) {
	    CC_shape_2point* v1 = (CC_shape_2point*)shape_list.First()->Data();
	    CC_shape_2point* v2 = (CC_shape_2point*)
	      shape_list.First()->Next()->Data();
	    CC_shape_2point* v3 = (CC_shape_2point*)
	      shape_list.First()->Next()->Next()->Data();
	    CC_coord s1, s2, s3;
	    CC_coord e1, e2, e3;
	    float d;
	    Matrix m;

	    s1 = v1->GetOrigin();
	    e1 = v1->GetPoint();
	    s2 = v2->GetOrigin();
	    e2 = v2->GetPoint();
	    s3 = v3->GetOrigin();
	    e3 = v3->GetPoint();
	    d = (float)s1.x*(float)s2.y - (float)s2.x*(float)s1.y +
	      (float)s3.x*(float)s1.y - (float)s1.x*(float)s3.y +
	      (float)s2.x*(float)s3.y - (float)s3.x*(float)s2.y;
	    if (IS_ZERO(d)) {
	      (void)wxMessageBox("Invalid genius move definition",
				 "Genius Move");
	    } else {
	      Matrix A = Matrix(Vector(e1.x,e2.x,0,e3.x),
				Vector(e1.y,e2.y,0,e3.y),
				Vector(0,0,0,0),
				Vector(1,1,0,1));
	      Matrix Binv = Matrix(Vector((float)s2.y-(float)s3.y,
					  (float)s3.x-(float)s2.x, 0,
					  (float)s2.x*(float)s3.y -
					  (float)s3.x*(float)s2.y),
				   Vector((float)s3.y-(float)s1.y,
					  (float)s1.x-(float)s3.x, 0,
					  (float)s3.x*(float)s1.y - 
					  (float)s1.x*(float)s3.y),
				   Vector(0, 0, 0, 0),
				   Vector((float)s1.y-(float)s2.y,
					  (float)s2.x-(float)s1.x, 0,
					  (float)s1.x*(float)s2.y -
					  (float)s2.x*(float)s1.y));
	      Binv /= d;
	      m = Binv*A;
	      if (sheet->TransformPoints(m, curr_ref))
		show_descr.show->winlist->
		  UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	    }
	    EndDrag();
	    ourframe->SetCurrentMove(CC_MOVE_NORMAL);
	  }
	  break;
	default:
	  switch (drag) {
	  case CC_DRAG_BOX:
	    SelectPointsInRect(shape->GetOrigin(), shape->GetPoint(),
			       curr_ref);
	    EndDrag();
	    break;
	  case CC_DRAG_LINE:
	    pos = shape->GetPoint() - shape->GetOrigin();
	    if (sheet->TranslatePoints(pos, curr_ref))
	      show_descr.show->winlist->
		UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	    EndDrag();
	    break;
	  case CC_DRAG_LASSO:
	    ((CC_lasso*)curr_shape)->End();
	    SelectWithLasso((CC_lasso*)curr_shape);
	    EndDrag();
	    break;
	  default:
	    break;
	  }
	  break;
	}
      } else if (event.RightDown() && curr_shape) {
	switch (drag) {
	case CC_DRAG_POLY:
	  SelectWithLasso((CC_lasso*)curr_shape);
	  EndDrag();
	  break;
	default:
	  break;
	}
      } else if (event.Dragging() && event.LeftIsDown() && curr_shape) {
	MoveDrag(pos);
      } else if (event.Moving() && curr_shape) {
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
		  num, sht->GetName(), sht->GetBeats());
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
  ClearShapes();
  curr_shape = NULL;
  switch (type) {
  case CC_DRAG_BOX:
    AddDrag(type, new CC_shape_rect(start));
    break;
  case CC_DRAG_POLY:
    AddDrag(type, new CC_poly(start));
    break;
  case CC_DRAG_LASSO:
    AddDrag(type, new CC_lasso(start));
    break;
  case CC_DRAG_LINE:
    AddDrag(type, new CC_shape_line(start));
    break;
  case CC_DRAG_CROSS:
    AddDrag(type, new CC_shape_cross(start, INT2COORD(2)));
  default:
    break;
  }
}

void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, CC_shape *shape) {
  DrawDrag(FALSE);
  drag = type;
  ClearShapes();
  curr_shape = NULL;
  AddDrag(type, shape);
}

void FieldCanvas::AddDrag(CC_DRAG_TYPES type, CC_shape *shape) {
  DrawDrag(FALSE);
  drag = type;
  shape_list.Append((wxObject*)shape);
  curr_shape = shape;
  DrawDrag(TRUE);
}

void FieldCanvas::MoveDrag(CC_coord end) {
  if (curr_shape) {
    DrawDrag(FALSE);
    curr_shape->OnMove(end, ourframe);
    DrawDrag(TRUE);
  }
}

void FieldCanvas::EndDrag() {
  DrawDrag(FALSE);
  ClearShapes();
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

Bool FieldCanvas::SelectWithLasso(const CC_lasso* lasso) {
  Bool changed = FALSE;
  CC_sheet* sheet = show_descr.CurrSheet();
  wxList pointlist;
  const wxPoint *pnt;

  for (unsigned i = 0; i < show_descr.show->GetNumPoints(); i++) {
    if (lasso->Inside(sheet->GetPosition(i, curr_ref))) {
      changed = TRUE;
      pointlist.Append(i, NULL);
    }
  }
  pnt = lasso->FirstPoint();
  if (changed && pnt) {
    SelectOrdered(pointlist, CC_coord((Coord)pnt->x, (Coord)pnt->y));
    show_descr.show->winlist->UpdateSelections();
  }

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
  mf->SetCurrentLasso(CC_DRAG_BOX);
}

static void toolbar_poly(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentLasso(CC_DRAG_POLY);
}

static void toolbar_lasso(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentLasso(CC_DRAG_LASSO);
}

static void toolbar_move(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_NORMAL);
}

static void toolbar_line(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_LINE);
}

static void toolbar_rot(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_ROTATE);
}

static void toolbar_shear(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_SHEAR);
}

static void toolbar_reflect(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_REFL);
}

static void toolbar_size(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_SIZE);
}

static void toolbar_genius(CoolToolBar *tb) {
  MainFrame *mf = (MainFrame *)tb->ourframe;
  mf->SetCurrentMove(CC_MOVE_GENIUS);
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
