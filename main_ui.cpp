/* main_ui.cpp
 * Handle wxWindows interface
 *
 * Modification history:
 * 6-1-95     Garrick Meeker              Created
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

#include "main_ui.h"
#include "print_ui.h"
#include "show_ui.h"
#include "anim_ui.h"
#include "cont_ui.h"
#include "undo.h"
#include "modes.h"
#include "confgr.h"
#include "ccvers.h"

#include <wx/help.h>
#ifdef __WXMSW__
#include <wx/helpwin.h>
#endif

#ifdef __CC_INCLUDE_BITMAPS__
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
#if defined(__APPLE__) && (__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#endif // defined(__APPLE__) && (__APPLE__)

static ToolBarEntry main_tb[] = {
  { 0, NULL, wxT("Previous stuntsheet"), CALCHART__prev_ss },
  { TOOLBAR_SPACE, NULL, wxT("Next stuntsheet"), CALCHART__next_ss },
  { TOOLBAR_TOGGLE, NULL, wxT("Select points with box"), CALCHART__box },
  { TOOLBAR_TOGGLE, NULL, wxT("Select points with polygon"), CALCHART__poly },
  { TOOLBAR_SPACE | TOOLBAR_TOGGLE, NULL,
    wxT("Select points with lasso"), CALCHART__lasso },
  { TOOLBAR_TOGGLE, NULL, wxT("Translate points"), CALCHART__move },
  { TOOLBAR_TOGGLE, NULL, wxT("Move points into line"), CALCHART__line },
  { TOOLBAR_TOGGLE, NULL, wxT("Rotate block"), CALCHART__rot },
  { TOOLBAR_TOGGLE, NULL, wxT("Shear block"), CALCHART__shear },
  { TOOLBAR_TOGGLE, NULL, wxT("Reflect block"), CALCHART__reflect },
  { TOOLBAR_TOGGLE, NULL, wxT("Resize block"), CALCHART__size },
  { TOOLBAR_SPACE | TOOLBAR_TOGGLE, NULL, wxT("Genius move"), CALCHART__genius },
  { 0, NULL, wxT("Label on left"), CALCHART__label_left },
  { 0, NULL, wxT("Flip label"), CALCHART__label_flip },
  { TOOLBAR_SPACE, NULL, wxT("Label on right"), CALCHART__label_right },
  { 0, NULL, wxT("Change to plainmen"), CALCHART__setsym0 },
  { 0, NULL, wxT("Change to solidmen"), CALCHART__setsym1 },
  { 0, NULL, wxT("Change to backslash men"), CALCHART__setsym2 },
  { 0, NULL, wxT("Change to slash men"), CALCHART__setsym3 },
  { 0, NULL, wxT("Change to x men"), CALCHART__setsym4 },
  { 0, NULL, wxT("Change to solid backslash men"), CALCHART__setsym5 },
  { 0, NULL, wxT("Change to solid slash men"), CALCHART__setsym6 },
  { 0, NULL, wxT("Change to solid x men"), CALCHART__setsym7 }
};

#define TOOLBAR_BOX 2
#define TOOLBAR_TRANS 5

extern ToolBarEntry anim_tb[];
extern ToolBarEntry printcont_tb[];

const wxString gridtext[] = {
  wxT("None"),
  wxT("1"),
  wxT("2"),
  wxT("4"),
  wxT("Mil"),
  wxT("2-Mil"),
};

static const wxChar *file_wild = FILE_WILDCARDS;
static const wxChar *file_save_wild = FILE_SAVE_WILDCARDS;

struct GridValue {
  Coord num, sub;
};

GridValue gridvalue[] = {
  {1,0},
  {INT2COORD(1),0},
  {INT2COORD(2),0},
  {INT2COORD(4),0},
  {INT2COORD(4),INT2COORD(4)/3},
  {INT2COORD(8),INT2COORD(8)/3}
};

// This statement initializes the whole application and calls OnInit
IMPLEMENT_APP(CalChartApp)

static MainFrameList *window_list;

wxHelpControllerBase *help_inst = NULL;

TopFrame *topframe = NULL;

wxFont *contPlainFont;
wxFont *contBoldFont;
wxFont *contItalFont;
wxFont *contBoldItalFont;
wxFont *pointLabelFont;
wxFont *yardLabelFont;

ShowModeList *modelist;

BEGIN_EVENT_TABLE(TopFrame, CC_MDIChildFrame)
  EVT_CLOSE(TopFrame::OnCloseWindow)
#ifdef CC_USE_MDI
  EVT_MENU(wxID_NEW, TopFrame::OnCmdNew)
  EVT_MENU(wxID_OPEN, TopFrame::OnCmdLoad)
  EVT_MENU(wxID_EXIT, TopFrame::OnCmdExit)
  EVT_MENU(wxID_ABOUT, TopFrame::OnCmdAbout)
  EVT_MENU(wxID_HELP, TopFrame::OnCmdHelp)
#else
  EVT_BUTTON(wxID_NEW, TopFrame::OnCmdNew)
  EVT_BUTTON(wxID_OPEN, TopFrame::OnCmdLoad)
  EVT_BUTTON(wxID_QUIT, TopFrame::OnCmdExit)
#endif
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MainFrame, CC_MDIChildFrame)
  EVT_CLOSE(MainFrame::OnCloseWindow)
  EVT_MENU(wxID_NEW, MainFrame::OnCmdNew)
  EVT_MENU(CALCHART__NEW_WINDOW, MainFrame::OnCmdNewWindow)
  EVT_MENU(wxID_OPEN, MainFrame::OnCmdLoad)
  EVT_MENU(CALCHART__APPEND_FILE, MainFrame::OnCmdAppend)
  EVT_MENU(CALCHART__IMPORT_CONT_FILE, MainFrame::OnCmdImportCont)
  EVT_MENU(wxID_SAVE, MainFrame::OnCmdSave)
  EVT_MENU(wxID_SAVEAS, MainFrame::OnCmdSaveAs)
  EVT_MENU(wxID_PRINT, MainFrame::OnCmdPrint)
  EVT_MENU(CALCHART__PRINT_EPS, MainFrame::OnCmdPrintEPS)
  EVT_MENU(wxID_CLOSE, MainFrame::OnCmdClose)
  EVT_MENU(wxID_EXIT, MainFrame::OnCmdExit)
  EVT_MENU(wxID_UNDO, MainFrame::OnCmdUndo)
  EVT_MENU(wxID_REDO, MainFrame::OnCmdRedo)
  EVT_MENU(CALCHART__INSERT_BEFORE, MainFrame::OnCmdInsertBefore)
  EVT_MENU(CALCHART__INSERT_AFTER, MainFrame::OnCmdInsertAfter)
  EVT_MENU(wxID_DELETE, MainFrame::OnCmdDelete)
  EVT_MENU(CALCHART__RELABEL, MainFrame::OnCmdRelabel)
  EVT_MENU(CALCHART__CLEAR_REF, MainFrame::OnCmdClearRef)
  EVT_MENU(CALCHART__EDIT_CONTINUITY, MainFrame::OnCmdEditCont)
  EVT_MENU(CALCHART__EDIT_PRINTCONT, MainFrame::OnCmdEditPrintCont)
  EVT_MENU(CALCHART__SET_TITLE, MainFrame::OnCmdSetTitle)
  EVT_MENU(CALCHART__SET_BEATS, MainFrame::OnCmdSetBeats)
  EVT_MENU(CALCHART__SETUP, MainFrame::OnCmdSetup)
  EVT_MENU(CALCHART__POINTS, MainFrame::OnCmdPoints)
  EVT_MENU(CALCHART__ANIMATE, MainFrame::OnCmdAnimate)
  EVT_MENU(CALCHART__ROWS, MainFrame::OnCmdRows)
  EVT_MENU(CALCHART__COLUMNS, MainFrame::OnCmdColumns)
  EVT_MENU(CALCHART__NEAREST, MainFrame::OnCmdNearest)
  EVT_MENU(wxID_ABOUT, MainFrame::OnCmdAbout)
  EVT_MENU(wxID_HELP, MainFrame::OnCmdHelp)
  EVT_MENU_HIGHLIGHT(wxID_SAVE, MainFrame::OnMenuSelect)
  EVT_MENU_HIGHLIGHT(wxID_UNDO, MainFrame::OnMenuSelect)
  EVT_MENU_HIGHLIGHT(wxID_REDO, MainFrame::OnMenuSelect)
  EVT_MENU(CALCHART__prev_ss, MainFrame::OnCmd_prev_ss)
  EVT_MENU(CALCHART__next_ss, MainFrame::OnCmd_next_ss)
  EVT_MENU(CALCHART__box, MainFrame::OnCmd_box)
  EVT_MENU(CALCHART__poly, MainFrame::OnCmd_poly)
  EVT_MENU(CALCHART__lasso, MainFrame::OnCmd_lasso)
  EVT_MENU(CALCHART__move, MainFrame::OnCmd_move)
  EVT_MENU(CALCHART__line, MainFrame::OnCmd_line)
  EVT_MENU(CALCHART__rot, MainFrame::OnCmd_rot)
  EVT_MENU(CALCHART__shear, MainFrame::OnCmd_shear)
  EVT_MENU(CALCHART__reflect, MainFrame::OnCmd_reflect)
  EVT_MENU(CALCHART__size, MainFrame::OnCmd_size)
  EVT_MENU(CALCHART__genius, MainFrame::OnCmd_genius)
  EVT_MENU(CALCHART__label_left, MainFrame::OnCmd_label_left)
  EVT_MENU(CALCHART__label_right, MainFrame::OnCmd_label_right)
  EVT_MENU(CALCHART__label_flip, MainFrame::OnCmd_label_flip)
  EVT_MENU(CALCHART__setsym0, MainFrame::OnCmd_setsym0)
  EVT_MENU(CALCHART__setsym1, MainFrame::OnCmd_setsym1)
  EVT_MENU(CALCHART__setsym2, MainFrame::OnCmd_setsym2)
  EVT_MENU(CALCHART__setsym3, MainFrame::OnCmd_setsym3)
  EVT_MENU(CALCHART__setsym4, MainFrame::OnCmd_setsym4)
  EVT_MENU(CALCHART__setsym5, MainFrame::OnCmd_setsym5)
  EVT_MENU(CALCHART__setsym6, MainFrame::OnCmd_setsym6)
  EVT_MENU(CALCHART__setsym7, MainFrame::OnCmd_setsym7)
  EVT_COMMAND_SCROLL(CALCHART__slider_zoom, MainFrame::slider_zoom_callback)
  EVT_COMMAND_SCROLL(CALCHART__slider_sheet_callback, MainFrame::slider_sheet_callback)
  EVT_CHOICE(CALCHART__refnum_callback, MainFrame::refnum_callback)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(FieldCanvas, AutoScrollCanvas)
  EVT_CHAR(FieldCanvas::OnChar)
  EVT_MOUSE_EVENTS(FieldCanvas::OnMouseEvent)
  EVT_PAINT(FieldCanvas::OnPaint)
END_EVENT_TABLE()

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
  frame->SetTitle(frame->field->show_descr.show->UserGetName());
  winlist.ChangeName();
}
void CC_WinNodeMain::UpdateSelections(wxWindow* win, int point) {
  if (wxColourDisplay()) {
    frame->field->RefreshShow(false, point);
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
void CC_WinNodeMain::GotoContLocation(unsigned sht, unsigned contnum,
				      int line, int col) {
  frame->field->GotoSS(sht);
  winlist.GotoContLocation(sht,contnum,line,col);
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

// Create windows and initialize app
bool CalChartApp::OnInit()
{
#if defined(__APPLE__) && (__APPLE__)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif // defined(__APPLE__) && (__APPLE__)


  wxString runtimepath(wxT("runtime"));
  int realargc = argc;

  modelist = new ShowModeList();

  if (argc > 1) {
    wxString arg(argv[argc-1]);
    if (wxDirExists(arg)) {
      runtimepath = arg;
      realargc--;
    }
  }

  wxString s = ReadConfig(runtimepath);
  if (!s.empty()) {
    (void)wxMessageBox(s, wxT("CalChart"));
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

  topframe = new TopFrame(300, 100);
  topframe->Maximize(true);
  for (i = 1; i < realargc; i++) {
    CC_show *shw;

    shw = new CC_show(argv[i]);
    if (shw->Ok()) {
      topframe->NewShow(shw);
    } else {
      (void)wxMessageBox(shw->GetError(), wxT("Load Error"));
      delete shw;
    }
  }

  {
    wxString helpfile(program_dir);
    helpfile.Append(PATH_SEPARATOR wxT("charthlp"));
#ifdef __WXMSW__
    help_inst = new wxWinHelpController(topframe);
#else
    help_inst = new wxHelpController;
#endif
    help_inst->Initialize(helpfile);
  }

  if (!shows_dir.empty()) {
    wxSetWorkingDirectory(shows_dir);
  }

  SetAutoSave(autosave_interval);
  SetTopWindow(topframe);

  return true;
}

int CalChartApp::OnExit() {
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
  pntlist.clear();
}

void CC_lasso::Start(const CC_coord& p) {
  Clear();
  Append(p);
}

// Closes polygon
void CC_lasso::End() {
  if (!pntlist.empty()) {
    pntlist.push_back(pntlist[0]);
  }
}

void CC_lasso::Append(const CC_coord& p) {
  pntlist.push_back(wxPoint(p.x, p.y));
}

// Test if inside polygon using odd-even rule
bool CC_lasso::Inside(const CC_coord& p) const {
  bool parity = false;
  for (PointList::const_iterator i=pntlist.begin();
       i != pntlist.end();
       ) {
    PointList::const_iterator prev(i);
    ++i;
    if (CrossesLine(*prev, *i, p)) {
      parity = !parity;
    }
  }
  return parity;
}

void CC_lasso::Draw(wxDC *dc, float x, float y) const {
  dc->DrawLines(pntlist.size(), const_cast<wxPoint*>(&pntlist[0]), x, y);
}

void CC_lasso::Drag(const CC_coord& p) {
  if (!pntlist.empty()) {
    pntlist.back() = wxPoint(p.x, p.y);
  }
}

bool CC_lasso::CrossesLine(const wxPoint& start, const wxPoint& end,
			   const CC_coord& p) const {
  if (start.y > end.y) {
    if (!((p.y <= start.y) && (p.y > end.y))) {
      return false;
    }
  } else {
    if (!((p.y <= end.y) && (p.y > start.y))) {
      return false;
    }
  }
  if (p.x >=
      ((end.x-start.x) * (p.y-start.y) / (end.y-start.y) + start.x)) {
    return true;
  }
  return false;
}

CC_poly::CC_poly(const CC_coord &p)
  : CC_lasso(p) {
    // add end point
    Append(p);
}

void CC_poly::OnMove(const CC_coord& p, MainFrame *) {
  Drag(p);
}

TopFrame::TopFrame(int width, int height):
  CC_MDIParentFrame(NULL, wxID_ANY, wxT("CalChart"), wxDefaultPosition, wxSize(width, height), CC_FRAME_TOP) {
  // Give it an icon
  SetBandIcon(this);

#ifdef CC_USE_MDI
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(wxID_NEW, wxT("&New Show"), wxT("Create a new show"));
  file_menu->Append(wxID_OPEN, wxT("&Open..."), wxT("Load a saved show"));
  file_menu->Append(wxID_EXIT, wxT("&Quit"), wxT("Quit CalChart"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(wxID_ABOUT, wxT("&About CalChart..."), wxT("Information about the program"));
  help_menu->Append(wxID_HELP, wxT("&Help on CalChart..."), wxT("Help on using CalChart"));

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, wxT("&File"));
  menu_bar->Append(help_menu, wxT("&Help"));
  SetMenuBar(menu_bar);
#else
  int w, h;

  wxPanel *p = new wxPanel(this);
  p->SetHorizontalSpacing(0);
  p->SetVerticalSpacing(0);
  (void)new wxButton(p, wxID_NEW, wxT("&New"));
  (void)new wxButton(p, wxID_OPEN, wxT("&Open"));
  (void)new wxButton(p, wxID_QUIT, wxT("&Quit"));
  p->Fit();
  p->GetSize(&w, &h);
  SetClientSize(w, h);
#ifndef BUGGY_SIZE_HINTS
  GetSize(&w, &h);
  SetSizeHints(w, h, w, h);
#endif
#endif
  SetDropTarget(new TopFrameDropTarget(this));
  Show(true);
}

TopFrame::~TopFrame() {
}

void TopFrame::OnCloseWindow(wxCloseEvent& event) {
  window_list->CloseAllWindows();
  Destroy();
}

#ifdef CC_USE_MDI
void TopFrame::OnCmdNew(wxCommandEvent& event) {
  NewShow();
}

void TopFrame::OnCmdLoad(wxCommandEvent& event) {
  OpenShow();
}

void TopFrame::OnCmdExit(wxCommandEvent& event) {
  Quit();
}

void TopFrame::OnCmdAbout(wxCommandEvent& event) {
  About();
}

void TopFrame::OnCmdHelp(wxCommandEvent& event) {
  Help();
}
#endif

void TopFrame::NewShow(CC_show *shw) {
  (void)new MainFrame(this, 50, 50,
		      window_default_width, window_default_height, shw);
}

void TopFrame::OpenShow(const wxString& filename) {
  CC_show *shw;

  wxString s(filename);
  if (s.empty())
    s = wxFileSelector(wxT("Load show"), NULL, NULL, NULL, file_wild);
  if (!s.empty()) {
    shw = new CC_show(s);
    if (shw->Ok()) {
      NewShow(shw);
    } else {
      (void)wxMessageBox(shw->GetError(), wxT("Load Error"));
      delete shw;
    }
  }
}

void TopFrame::Quit() {
  Close();
}

void TopFrame::About() {
  (void)wxMessageBox(wxT("CalChart ") wxT(CC_VERSION) wxT("\nAuthor: Gurk Meeker\n")
		     wxT("http://calchart.sourceforge.net\n")
		     wxT("Copyright (c) 1994-2008 Garrick Meeker\n")
		     wxT("\n")
		     wxT("This program is free software: you can redistribute it and/or modify\n")
		     wxT("it under the terms of the GNU General Public License as published by\n")
		     wxT("the Free Software Foundation, either version 3 of the License, or\n")
		     wxT("(at your option) any later version.\n")
		     wxT("\n")
		     wxT("This program is distributed in the hope that it will be useful,\n")
		     wxT("but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
		     wxT("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
		     wxT("GNU General Public License for more details.\n")
		     wxT("\n")
		     wxT("You should have received a copy of the GNU General Public License\n")
		     wxT("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n")
		     wxT("\n")
		     wxT("Compiled on ") __TDATE__ wxT(" at ") __TTIME__,
		     wxT("About CalChart"));
}

void TopFrame::Help() {
  help_inst->LoadFile();
  help_inst->DisplayContents();
}

bool TopFrameDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
  for (size_t i = 0; i < filenames.Count(); i++) {
    frame->OpenShow(filenames[i]);
  }
  return true;
}

// Main frame constructor
MainFrame::MainFrame(wxMDIParentFrame *frame, int x, int y, int w, int h,
		     CC_show *show, MainFrame *other_frame):
  CC_MDIChildFrame(frame, -1, wxT("CalChart"), wxPoint(x, y), wxSize(w, h), CC_FRAME_CHILD),
  field(NULL)
{
  unsigned ss;
  unsigned def_zoom;
  unsigned def_grid;
  unsigned def_ref;
  bool setup;

  // Give it an icon
  SetBandIcon(this);

  // Give it a status line
  CreateStatusBar(2);
  SetStatusText(wxT("Welcome to Calchart 3.0"));

  // Make a menubar
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(wxID_NEW, wxT("&New Show"), wxT("Create a new show"));
  file_menu->Append(CALCHART__NEW_WINDOW, wxT("New &Window"), wxT("Open a new window"));
  file_menu->Append(wxID_OPEN, wxT("&Open..."), wxT("Load a saved show"));
  file_menu->Append(CALCHART__APPEND_FILE, wxT("&Append..."), wxT("Append a show to the end"));
  file_menu->Append(CALCHART__IMPORT_CONT_FILE, wxT("&Import Continuity..."), wxT("Import continuity text"));
  file_menu->Append(wxID_SAVE, wxT("&Save"), wxT("Save show"));
  file_menu->Append(wxID_SAVEAS, wxT("Save &As..."), wxT("Save show as a new name"));
  file_menu->Append(wxID_PRINT, wxT("&Print..."), wxT("Print this show"));
  file_menu->Append(CALCHART__PRINT_EPS, wxT("Print &EPS..."), wxT("Print a stuntsheet in EPS"));
  file_menu->Append(wxID_CLOSE, wxT("&Close Window"), wxT("Close this window"));
  file_menu->Append(wxID_EXIT, wxT("&Quit"), wxT("Quit CalChart"));

  wxMenu *edit_menu = new wxMenu;
  edit_menu->Append(wxID_UNDO, wxT("&Undo"));
  edit_menu->Append(wxID_REDO, wxT("&Redo"));
  edit_menu->Append(CALCHART__INSERT_BEFORE, wxT("&Insert Sheet Before"), wxT("Insert a new stuntsheet before this one"));
  edit_menu->Append(CALCHART__INSERT_AFTER, wxT("Insert Sheet &After"), wxT("Insert a new stuntsheet after this one"));
  edit_menu->Append(wxID_DELETE, wxT("&Delete Sheet"), wxT("Delete this stuntsheet"));
  edit_menu->Append(CALCHART__RELABEL, wxT("&Relabel Sheets"), wxT("Relabel all stuntsheets after this one"));
  edit_menu->Append(CALCHART__CLEAR_REF, wxT("&Clear Reference"), wxT("Clear selected reference points"));
  edit_menu->Append(CALCHART__SETUP, wxT("&Setup Show..."), wxT("Setup basic show information"));
  edit_menu->Append(CALCHART__POINTS, wxT("&Point Selections..."), wxT("Select Points"));
  edit_menu->Append(CALCHART__SET_TITLE, wxT("Set &Title..."), wxT("Change the title of this stuntsheet"));
  edit_menu->Append(CALCHART__SET_BEATS, wxT("Set &Beats..."), wxT("Change the number of beats for this stuntsheet"));

  wxMenu *anim_menu = new wxMenu;
  anim_menu->Append(CALCHART__EDIT_CONTINUITY, wxT("&Edit Continuity..."), wxT("Edit continuity for this stuntsheet"));
  anim_menu->Append(CALCHART__EDIT_PRINTCONT, wxT("Edit &Printed Continuity..."), wxT("Edit printed continuity for this stuntsheet"));
  anim_menu->Append(CALCHART__ANIMATE, wxT("&Animate..."), wxT("Open animation window"));

  wxMenu *select_menu = new wxMenu;
  // These items are a radio group
  select_menu->Append(CALCHART__ROWS, wxT("Rows first"), wxT("Select points by rows"), wxITEM_RADIO);
  select_menu->Append(CALCHART__COLUMNS, wxT("Columns first"), wxT("Select points by columns"), wxITEM_RADIO);
  select_menu->Append(CALCHART__NEAREST, wxT("Nearest"), wxT("Select points in nearest order"), wxITEM_RADIO);

  wxMenu *options_menu = new wxMenu;
  options_menu->Append(CALCHART__SELECTION, wxT("Selection Order"), select_menu);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(wxID_ABOUT, wxT("&About CalChart..."), wxT("Information about the program"));
  help_menu->Append(wxID_HELP, wxT("&Help on CalChart..."), wxT("Help on using CalChart"));

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, wxT("&File"));
  menu_bar->Append(edit_menu, wxT("&Edit"));
  menu_bar->Append(anim_menu, wxT("&Animation"));
  menu_bar->Append(options_menu, wxT("&Options"));
  menu_bar->Append(help_menu, wxT("&Help"));
  SetMenuBar(menu_bar);

  // Add a toolbar
  CoolToolBar ribbon(this, wxID_ANY);
  ribbon.SetupBar(main_tb, sizeof(main_tb)/sizeof(ToolBarEntry));

  // Add the field canvas
  setup = false;
  if (!other_frame) {
    if (!show) {
      show = new CC_show();
      setup = true;
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
	
  SetTitle(show->UserGetName());
  field->curr_ref = def_ref;
  node = new CC_WinNodeMain(show->winlist, this);
  switch(field->curr_select) {
  case CC_SELECT_ROWS:
    menu_bar->Check(CALCHART__ROWS, true);
    break;
  case CC_SELECT_COLUMNS:
    menu_bar->Check(CALCHART__COLUMNS, true);
    break;
  case CC_SELECT_NEAREST:
    menu_bar->Check(CALCHART__NEAREST, true);
    break;
  }

  // Add the controls
	wxBoxSizer* fullsizer = new wxBoxSizer(wxVERTICAL);

  // Grid choice
  grid_choice = new wxChoice(this, -1, wxPoint(-1, -1), wxSize(-1, -1),
			     sizeof(gridtext)/sizeof(const wxChar*),
			     gridtext);
  grid_choice->SetSelection(def_grid);

  // Zoom slider
  zoom_slider = new wxSlider(this, CALCHART__slider_zoom, def_zoom, 1, FIELD_MAXZOOM);

	// set up a sizer for the field panel
	wxBoxSizer* rowsizer = new wxBoxSizer(wxHORIZONTAL);
	rowsizer->Add(grid_choice, 1, wxALL);
	rowsizer->Add(zoom_slider, 1, wxEXPAND);
	fullsizer->Add(rowsizer);
  // Reference choice
  {
    wxString buf;
    unsigned i;

    ref_choice = new wxChoice(this, CALCHART__refnum_callback);
    ref_choice->Append(wxT("Off"));
    for (i = 1; i <= NUM_REF_PNTS; i++) {
      buf.sprintf(wxT("%u"), i);
      ref_choice->Append(buf);
    }
  }
  ref_choice->SetSelection(def_ref);

  // Sheet slider (will get set later with UpdatePanel())
  sheet_slider = new wxSlider(this, CALCHART__slider_sheet_callback, 1, 1, 2);

	rowsizer = new wxBoxSizer(wxHORIZONTAL);
	rowsizer->Add(ref_choice, 0, wxALL);
	rowsizer->Add(sheet_slider, 0, wxEXPAND);
	fullsizer->Add(rowsizer);

  // Update the tool bar
  SetCurrentLasso(field->curr_lasso);
  SetCurrentMove(field->curr_move);

  // Show the frame
  UpdatePanel();
  window_list->push_back(this);
  field->RefreshShow();

	fullsizer->Add(field, 1, wxEXPAND);
	SetSizer(fullsizer);
  Show(true);

  if (setup) Setup();
}

MainFrame::~MainFrame() {
  window_list->remove(this);
  if (node) {
    node->Remove();
    delete node;
  }
}

// Define the behaviour for the frame closing
void MainFrame::OnCloseWindow(wxCloseEvent& event)
{
  // Save changes first
  if (!OkayToClearShow()) {
    if (event.CanVeto()) {
      event.Veto(); // don't delete the frame
    } else {
      Destroy();
    }
  } else {
    Destroy();
  }
}

// Intercept menu commands

void MainFrame::OnCmdNew(wxCommandEvent& event) {
  topframe->NewShow();
}

void MainFrame::OnCmdNewWindow(wxCommandEvent& event) {
  MainFrame *frame;
  frame = new MainFrame(topframe, 50, 50, window_default_width,
			window_default_height, NULL, this);
}

void MainFrame::OnCmdLoad(wxCommandEvent& event) {
  topframe->OpenShow();
}

void MainFrame::OnCmdAppend(wxCommandEvent& event) {
  AppendShow();
}

void MainFrame::OnCmdImportCont(wxCommandEvent& event) {
  ImportContFile();
}

void MainFrame::OnCmdSave(wxCommandEvent& event) {
  SaveShow();
}

void MainFrame::OnCmdSaveAs(wxCommandEvent& event) {
  SaveShowAs();
}

void MainFrame::OnCmdPrint(wxCommandEvent& event) {
  if (field->show_descr.show) {
    (void)new ShowPrintDialog(&field->show_descr, &node->winlist,
			      false, this, wxT("Print show"), false);
  }
}

void MainFrame::OnCmdPrintEPS(wxCommandEvent& event) {
  if (field->show_descr.show) {
    (void)new ShowPrintDialog(&field->show_descr, &node->winlist,
			      true, this, wxT("Print stuntsheet as EPS"), false);
  }
}

void MainFrame::OnCmdClose(wxCommandEvent& event) {
  Close();
}

void MainFrame::OnCmdExit(wxCommandEvent& event) {
  topframe->Quit();
}

void MainFrame::OnCmdUndo(wxCommandEvent& event) {
  int sheetnum;
  sheetnum = field->show_descr.show->undolist->Undo(field->show_descr.show);
  if ((sheetnum >= 0) && ((unsigned)sheetnum != field->show_descr.curr_ss))
    field->GotoSS((unsigned)sheetnum);
}

void MainFrame::OnCmdRedo(wxCommandEvent& event) {
  int sheetnum;
  sheetnum = field->show_descr.show->undolist->Redo(field->show_descr.show);
  if ((sheetnum >= 0) && ((unsigned)sheetnum != field->show_descr.curr_ss))
    field->GotoSS((unsigned)sheetnum);
}

void MainFrame::OnCmdInsertBefore(wxCommandEvent& event) {
  CC_sheet *sht;
  sht = new CC_sheet(field->show_descr.CurrSheet());
  field->show_descr.show->UserInsertSheet(sht, field->show_descr.curr_ss);
  field->PrevSS();
}

void MainFrame::OnCmdInsertAfter(wxCommandEvent& event) {
  CC_sheet *sht;
  sht = new CC_sheet(field->show_descr.CurrSheet());
  field->show_descr.show->UserInsertSheet(sht, field->show_descr.curr_ss+1);
  field->NextSS();
}

void MainFrame::OnCmdDelete(wxCommandEvent& event) {
  if (field->show_descr.show->GetNumSheets() > 1) {
    field->show_descr.show->UserDeleteSheet(field->show_descr.curr_ss);
  }
}

void MainFrame::OnCmdRelabel(wxCommandEvent& event) {
  if (field->show_descr.curr_ss+1 < field->show_descr.show->GetNumSheets()) {
    if(wxMessageBox(wxT("Relabeling sheets is not undoable.\nProceed?"),
		    wxT("Relabel sheets"), wxYES_NO) == wxYES) {
      if (!field->show_descr.show->RelabelSheets(field->show_descr.curr_ss))
	(void)wxMessageBox(wxT("Stuntsheets don't match"),
			   wxT("Relabel sheets"));
      else {
	field->show_descr.show->undolist->EraseAll();
	field->show_descr.show->SetModified(true);
      }
    }
  } else {
    (void)wxMessageBox(wxT("This can't used on the last stuntsheet"),
		       wxT("Relabel sheets"));
  }
}

void MainFrame::OnCmdClearRef(wxCommandEvent& event) {
  if (field->curr_ref > 0) {
    if (field->show_descr.CurrSheet()->ClearRefPositions(field->curr_ref))
      field->show_descr.show->winlist->
	UpdatePointsOnSheet(field->show_descr.curr_ss, field->curr_ref);
  }
}

void MainFrame::OnCmdEditCont(wxCommandEvent& event) {
  if (field->show_descr.show) {
    (void)new ContinuityEditor(&field->show_descr, &node->winlist, this,
			       wxT("Animation Continuity"));
  }
}

void MainFrame::OnCmdEditPrintCont(wxCommandEvent& event) {
  if (field->show_descr.show) {
    (void)new PrintContEditor(&field->show_descr, &node->winlist, this,
			      wxT("Printed Continuity"));
  }
}

void MainFrame::OnCmdSetTitle(wxCommandEvent& event) {
  wxString s;
  if (field->show_descr.show) {
    s = wxGetTextFromUser(wxT("Enter the new title"),
			  field->show_descr.CurrSheet()->GetName(),
			  field->show_descr.CurrSheet()->GetName(),
			  this);
    if (s) {
      field->show_descr.CurrSheet()->UserSetName(s);
    }
  }
}

void MainFrame::OnCmdSetBeats(wxCommandEvent& event) {
  wxString s;
  if (field->show_descr.show) {
    wxString buf;
    buf.sprintf(wxT("%u"), field->show_descr.CurrSheet()->GetBeats());
    s = wxGetTextFromUser(wxT("Enter the number of beats"),
			  field->show_descr.CurrSheet()->GetName(),
			  buf, this);
    if (!s.empty()) {
      long val;
      if (s.ToLong(&val)) {
	field->show_descr.CurrSheet()->UserSetBeats(val);
      }
    }
  }
}

void MainFrame::OnCmdSetup(wxCommandEvent& event) {
  Setup();
}

void MainFrame::OnCmdPoints(wxCommandEvent& event) {
  if (field->show_descr.show)
    (void)new PointPicker(field->show_descr.show, &node->winlist,
			  true, this, wxT("Select points"));
}

void MainFrame::OnCmdAnimate(wxCommandEvent& event) {
  if (field->show_descr.show) {
    AnimationFrame *anim =
      new AnimationFrame(this, &field->show_descr, &node->winlist);
    anim->canvas->Generate();
  }
}

void MainFrame::OnCmdSelect(int id) {
  GetMenuBar()->Check(field->curr_select, false);
  field->curr_select = (CC_SELECT_TYPES)id;
  GetMenuBar()->Check(id, true);
}

void MainFrame::OnCmdRows(wxCommandEvent& event) {
  OnCmdSelect(CALCHART__ROWS);
}

void MainFrame::OnCmdColumns(wxCommandEvent& event) {
  OnCmdSelect(CALCHART__COLUMNS);
}

void MainFrame::OnCmdNearest(wxCommandEvent& event) {
  OnCmdSelect(CALCHART__NEAREST);
}

void MainFrame::OnCmdAbout(wxCommandEvent& event) {
  topframe->About();
}

void MainFrame::OnCmdHelp(wxCommandEvent& event) {
  topframe->Help();
}

// Intercept menu commands
void MainFrame::OnMenuSelect(wxMenuEvent& event)
{
  wxString msg;
  switch (event.GetMenuId()) {
  case wxID_SAVE:
    msg = field->show_descr.show->Modified() ?
      wxT("Save show (needed)") :
	wxT("Save show (not needed)");
    break;
  case wxID_UNDO:
    msg = field->show_descr.show->undolist->UndoDescription();
    break;
  case wxID_REDO:
    msg = field->show_descr.show->undolist->RedoDescription();
    break;
  default:
    event.Skip();
    break;
  }
  if (!msg.empty())
    SetStatusText(msg);
}

void MainFrame::OnCmd_prev_ss(wxCommandEvent& event) {
  field->PrevSS();
}

void MainFrame::OnCmd_next_ss(wxCommandEvent& event) {
  field->NextSS();
}

void MainFrame::OnCmd_box(wxCommandEvent& event) {
  SetCurrentLasso(CC_DRAG_BOX);
}

void MainFrame::OnCmd_poly(wxCommandEvent& event) {
  SetCurrentLasso(CC_DRAG_POLY);
}

void MainFrame::OnCmd_lasso(wxCommandEvent& event) {
  SetCurrentLasso(CC_DRAG_LASSO);
}

void MainFrame::OnCmd_move(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_NORMAL);
}

void MainFrame::OnCmd_line(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_LINE);
}

void MainFrame::OnCmd_rot(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_ROTATE);
}

void MainFrame::OnCmd_shear(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_SHEAR);
}

void MainFrame::OnCmd_reflect(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_REFL);
}

void MainFrame::OnCmd_size(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_SIZE);
}

void MainFrame::OnCmd_genius(wxCommandEvent& event) {
  SetCurrentMove(CC_MOVE_GENIUS);
}

void MainFrame::OnCmd_label_left(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsLabel(false))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_label_right(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsLabel(true))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_label_flip(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsLabelFlip())
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym0(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_PLAIN))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym1(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOL))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym2(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_BKSL))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym3(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SL))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym4(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_X))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym5(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLBKSL))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym6(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLSL))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

void MainFrame::OnCmd_setsym7(wxCommandEvent& event) {
  if (field->show_descr.CurrSheet()->SetPointsSym(SYMBOL_SOLX))
    field->show_descr.show->winlist->
      UpdatePointsOnSheet(field->show_descr.curr_ss);
}

// Give the use a chance to save the current show
bool MainFrame::OkayToClearShow() {
  wxString buf;

  if (field->show_descr.show->Modified()) {
    if (!field->show_descr.show->winlist->MultipleWindows()) {
      buf.sprintf(wxT("Save changes to '%s'?"),
		  field->show_descr.show->UserGetName().c_str());
      switch (wxMessageBox(buf, wxT("Unsaved changes"),
			   wxYES_NO | wxCANCEL, this)) {
      case wxYES:
	SaveShowAs();
	break;
      case wxNO:
	break;
      case wxCANCEL:
	return false;
	break;
      }
    }
    field->show_descr.show->ClearAutosave();
  }
  return true;
}

// Load a show with file selector
void MainFrame::LoadShow() {
  wxString s;
  CC_show *shw;

  if (OkayToClearShow()) {
    s = wxFileSelector(wxT("Load show"), NULL, NULL, NULL, file_wild);
    if (!s.empty()) {
      shw = new CC_show(s);
      if (shw->Ok()) {
	node->SetShow(shw);
      } else {
	(void)wxMessageBox(shw->GetError(), wxT("Load Error"));
	delete shw;
      }
    }
  }
}

// Append a show with file selector
void MainFrame::AppendShow() {
  wxString s;
  CC_show *shw;
  unsigned currend;

  s = wxFileSelector(wxT("Append show"), NULL, NULL, NULL, file_wild);
  if (!s.empty()) {
    shw = new CC_show(s);
    if (shw->Ok()) {
      if (shw->GetNumPoints() == field->show_descr.show->GetNumPoints()) {
	currend = field->show_descr.show->GetNumSheets();
	field->show_descr.show->undolist->Add(new ShowUndoAppendSheets(currend));
	field->show_descr.show->Append(shw);
	if (!field->show_descr.show->RelabelSheets(currend-1))
	  (void)wxMessageBox(wxT("Stuntsheets don't match"),
			     wxT("Append Error"));
      } else {
	(void)wxMessageBox(wxT("The blocksize doesn't match"), wxT("Append Error"));
	delete shw;
      }
    } else {
      (void)wxMessageBox(shw->GetError(), wxT("Load Error"));
      delete shw;
    }
  }
}

// Append a show with file selector
void MainFrame::ImportContFile() {
  wxString s;
  wxString err;

  s = wxFileSelector(wxT("Import Continuity"), NULL, NULL, NULL, wxT("*.txt"));
  if (!s.empty()) {
    err = field->show_descr.show->ImportContinuity(s);
    if (!err.empty()) {
      (void)wxMessageBox(err, wxT("Load Error"));
      delete err;
    }
  }
}

// Save this show without file selector
void MainFrame::SaveShow() {
  wxString s;

  s = field->show_descr.show->GetName();
  if (s.empty()) {
    // No file name; use SaveAs instead
    SaveShowAs();
  } else {
    s = field->show_descr.show->Save(s);
    if (!s.empty()) {
      (void)wxMessageBox(s, wxT("Save Error"));
    }
  }
}

// Save this show with file selector
void MainFrame::SaveShowAs() {
  wxString s;
  wxString err;

  s = wxFileSelector(wxT("Save show"), NULL, NULL, NULL, file_save_wild,
		     wxSAVE | wxOVERWRITE_PROMPT);
  if (!s.empty()) {
    wxString str(s);
    unsigned int i = str.Length();
    if ((i < 4) ||
	(str.SubString(i-4,i-1).CompareTo(wxT(".shw"), wxString::ignoreCase)!=0)) {
      str.Append(wxT(".shw"));
    }
    err = field->show_descr.show->Save(str);
    if (!err.empty()) {
      (void)wxMessageBox(err, wxT("Save Error"));
    }
  }    
}

static inline Coord SNAPGRID(Coord a, Coord n, Coord s) {
  Coord a2 = (a+(n>>1)) & (~(n-1));
  Coord h = s>>1;
  if ((a-a2) >= h) return a2+s;
  else if ((a-a2) < -h) return a2-s;
  else return a2;
}

void MainFrame::SnapToGrid(CC_coord& c) {
  Coord gridn, grids;
  int n = grid_choice->GetSelection();

  gridn = gridvalue[n].num;
  grids = gridvalue[n].sub;

  c.x = SNAPGRID(c.x, gridn, grids);
  // Adjust so 4 step grid will be on visible grid
  c.y = SNAPGRID(c.y - INT2COORD(2), gridn, grids) + INT2COORD(2);
}

void MainFrame::SetCurrentLasso(CC_DRAG_TYPES type) {
#if 0
  if (field->curr_lasso != CC_DRAG_NONE) {
    frameToolBar->ToggleTool(TOOLBAR_BOX+field->curr_lasso-CC_DRAG_BOX, false);
  }
#endif
  field->curr_lasso = type;
#if 0
  if (field->curr_lasso != CC_DRAG_NONE) {
    frameToolBar->ToggleTool(TOOLBAR_BOX + type - CC_DRAG_BOX, true);
  }
#endif
}

void MainFrame::SetCurrentMove(CC_MOVE_MODES type) {
#if 0
  frameToolBar->ToggleTool(TOOLBAR_TRANS + field->curr_move - CC_MOVE_NORMAL,
			   false);
#endif
  field->curr_move = type;
#if 0
  frameToolBar->ToggleTool(TOOLBAR_TRANS + type - CC_MOVE_NORMAL, true);
#endif
  field->EndDrag();
}

void MainFrame::Setup() {
  if (field->show_descr.show)
    (void)new ShowInfoReq(field->show_descr.show, &node->winlist, this,
			  wxT("Setup Show"));
}

// Define a constructor for field canvas
FieldCanvas::FieldCanvas(CC_show *show, unsigned ss, MainFrame *frame,
			 int def_zoom, FieldCanvas *from_canvas,
			 int x, int y, int w, int h):
 AutoScrollCanvas(frame, -1, wxPoint(x, y), wxSize(w, h)), ourframe(frame), curr_lasso(CC_DRAG_BOX),
 curr_move(CC_MOVE_NORMAL), curr_select(CC_SELECT_ROWS),
 curr_ref(0), drag(CC_DRAG_NONE), curr_shape(NULL), dragon(false)
{
  if (from_canvas) {
    curr_lasso = from_canvas->curr_lasso;
    curr_move = from_canvas->curr_move;
    curr_select = from_canvas->curr_select;
  }

  SetPalette(CalChartPalette);

  show_descr.show = show;
  show_descr.curr_ss = ss;

  SetZoomQuick(def_zoom);

  UpdateBars();
}

FieldCanvas::~FieldCanvas(void) {
  ClearShapes();
}

void FieldCanvas::ClearShapes() {
  for (ShapeList::iterator i=shape_list.begin();
       i != shape_list.end();
       ++i) {
    delete *i;
  }
  shape_list.clear();
  curr_shape = NULL;
}

// Draw the current drag feedback
void FieldCanvas::DrawDrag(bool on)
{
  wxDC *dc = GetMemDC();
  CC_coord origin;

  if ((on != dragon) && curr_shape) {
    dragon = on;
    if (on) {
      SetXOR(dc);
      origin = show_descr.show->mode->Offset();
      for (ShapeList::const_iterator i=shape_list.begin();
	   i != shape_list.end();
	   ++i) {
	(*i)->Draw(dc, origin.x+GetPositionX(),
		   origin.y+GetPositionY());
      }
    } else {
      Blit(*dc);
    }
  }
}

// Define the repainting behaviour
void FieldCanvas::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);
  dc.SetBackground(*CalChartBrushes[COLOR_FIELD]);
  dc.Clear();
  Blit(dc);
  dragon = false; // since the canvas gets cleared
  DrawDrag(true);
}

// Allow clicking within pixels to close polygons
#define CLOSE_ENOUGH_TO_CLOSE 10
void FieldCanvas::OnMouseEvent(wxMouseEvent& event)
{
  long x,y;
  int i;
  CC_coord pos;

  if (show_descr.show) {
    CC_sheet *sheet = show_descr.CurrSheet();
    if (sheet) {
      event.GetPosition(&x, &y);
      if (event.ControlDown()) {
	Move(x, y);
	wxPaintDC dc(this);
	Blit(dc);
	dragon = false; // since the canvas gets cleared
      } else {
	Move(x, y, 1);
      }

      pos = show_descr.show->mode->Offset();
      pos.x = Coord((x/GetScaleX()) - pos.x);
      pos.y = Coord((y/GetScaleY()) - pos.y);

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
		  (Coord)GetMemDC()->DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
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
	    bool changed = false;
	    if (!event.ShiftDown()) changed = show_descr.show->UnselectAll();
	    i = sheet->FindPoint(pos.x, pos.y, curr_ref);
	    if (i >= 0) {
	      if (!(show_descr.show->IsSelected(i))) {
		show_descr.show->Select(i);
		changed = true;
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
	  if (shape_list.size() > 1) {
	    origin = (CC_shape_1point*)shape_list[0];
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
	  if (shape_list.size() > 1) {
	    origin = (CC_shape_1point*)shape_list[0];
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
	  if (shape_list.size() > 1) {
	    origin = (CC_shape_1point*)shape_list[0];
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
	  if (shape_list.size() >= 3) {
	    CC_shape_2point* v1 = (CC_shape_2point*)shape_list[0];
	    CC_shape_2point* v2 = (CC_shape_2point*)shape_list[1];
	    CC_shape_2point* v3 = (CC_shape_2point*)shape_list[2];
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
	      (void)wxMessageBox(wxT("Invalid genius move definition"),
				 wxT("Genius Move"));
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
      RefreshShow();
    }
  }
  Refresh();
}

void FieldCanvas::OnScroll(wxScrollEvent& event)
{
  event.Skip();
}

// Intercept character input
void FieldCanvas::OnChar(wxKeyEvent& event)
{
  // Process the default behaviour
  event.Skip();
}

void FieldCanvas::RefreshShow(bool drawall, int point) {
  wxPaintDC dc(this);
  if (show_descr.show) {
    CC_sheet *sheet = show_descr.CurrSheet();
    if (sheet) {
      if (curr_ref > 0) {
	sheet->Draw(GetMemDC(), 0, false, drawall, point);
	sheet->Draw(GetMemDC(), curr_ref, true, false, point);
      } else {
	sheet->Draw(GetMemDC(), curr_ref, true, drawall, point);
      }
      Blit(dc);
      dragon = false; // since the canvas gets cleared
      DrawDrag(true);
    }
  }
}

void MainFrame::UpdatePanel() {
  wxString tempbuf;
  CC_sheet *sht = field->show_descr.CurrSheet();
  unsigned num = field->show_descr.show->GetNumSheets();
  unsigned curr = field->show_descr.curr_ss+1;

  tempbuf.sprintf(wxT("%s%d of %d \"%.32s\" %d beats"),
		  field->show_descr.show->Modified() ? wxT("* "):wxT(""), curr,
		  num, sht->GetName().c_str(), sht->GetBeats());
  SetStatusText(tempbuf, 1);

  if (num > 1) {
    sheet_slider->Enable(true);
    if ((unsigned)sheet_slider->GetMax() != num)
      sheet_slider->SetValue(1); // So Motif doesn't complain about value
      sheet_slider->SetRange(1, num);
    if ((unsigned)sheet_slider->GetValue() != curr)
      sheet_slider->SetValue(curr);
  } else {
    sheet_slider->Enable(false);
  }
}

void FieldCanvas::UpdateBars() {
  if (show_descr.show) {
    SetSize(wxSize(COORD2INT(show_descr.show->mode->Size().x) * zoomf,
	    COORD2INT(show_descr.show->mode->Size().y) * zoomf));
  }
}

void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, CC_coord start) {
  DrawDrag(false);
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
  DrawDrag(false);
  drag = type;
  ClearShapes();
  curr_shape = NULL;
  AddDrag(type, shape);
}

void FieldCanvas::AddDrag(CC_DRAG_TYPES type, CC_shape *shape) {
  DrawDrag(false);
  drag = type;
  shape_list.push_back(shape);
  curr_shape = shape;
  DrawDrag(true);
}

void FieldCanvas::MoveDrag(CC_coord end) {
  if (curr_shape) {
    DrawDrag(false);
    curr_shape->OnMove(end, ourframe);
    DrawDrag(true);
  }
}

void FieldCanvas::EndDrag() {
  DrawDrag(false);
  ClearShapes();
  drag = CC_DRAG_NONE;
}

void FieldCanvas::SelectOrdered(PointList& pointlist,
				const CC_coord& start) {
  CC_coord c1, c2, last;
  Coord v1, v2;
  float f1, f2, fx, fy;
  CC_sheet* sheet = show_descr.CurrSheet();

  last = start;
  while (!pointlist.empty()) {
    PointList::iterator pnt(pointlist.begin());
    c1 = sheet->GetPosition(*pnt, curr_ref);
    for (PointList::iterator n=pnt+1; n != pointlist.end(); ++n) {
      switch (curr_select) {
      case CC_SELECT_ROWS:
	v1 = ABS(start.y - c1.y);
	c2 = sheet->GetPosition(*n, curr_ref);
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
	c2 = sheet->GetPosition(*n, curr_ref);
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
	c2 = sheet->GetPosition(*n, curr_ref);
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
    show_descr.show->Select(*pnt);
    last = c1;
    pointlist.erase(pnt);
  }
}

bool FieldCanvas::SelectWithLasso(const CC_lasso* lasso) {
  bool changed = false;
  CC_sheet* sheet = show_descr.CurrSheet();
  PointList pointlist;
  const wxPoint *pnt;

  for (unsigned i = 0; i < show_descr.show->GetNumPoints(); i++) {
    if (lasso->Inside(sheet->GetPosition(i, curr_ref))) {
      changed = true;
      pointlist.push_back(i);
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
bool FieldCanvas::SelectPointsInRect(const CC_coord& c1, const CC_coord& c2,
				     unsigned ref) {
  unsigned i;
  bool changed = false;
  CC_sheet* sheet = show_descr.CurrSheet();
  CC_coord top_left, bottom_right;
  const CC_coord *pos;
  PointList pointlist;

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
	pointlist.push_back(i);
	changed = true;
      }
    }
  }
  if (changed) {
    SelectOrdered(pointlist, c1);
    show_descr.show->winlist->UpdateSelections();
  }

  return changed;
}

bool MainFrameList::CloseAllWindows() {
  MainFrame *mf;

  for (MainFrameList::iterator i = begin(); i != end(); ) {
    mf = *i;
    // This node will be deleted by the window's deconstructor
    MainFrameList::iterator i_tmp(i);
    ++i_tmp;
    if (!mf->Close()) return false;
    i = i_tmp;
  }
  return true;
}

void MainFrame::refnum_callback(wxCommandEvent &) {
  field->curr_ref = ref_choice->GetSelection();
  field->RefreshShow();
}

void MainFrame::slider_sheet_callback(wxScrollEvent &) {
  field->GotoSS(sheet_slider->GetValue()-1);
}

void MainFrame::slider_zoom_callback(wxScrollEvent &) {
  field->SetZoom(zoom_slider->GetValue());
}
