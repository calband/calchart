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

#ifdef wx_x
#include "tb_left.xbm"
#include "tb_right.xbm"
#include "tb_box.xbm"
#include "tb_poly.xbm"
#include "tb_lasso.xbm"
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
static void slider_zoom_callback(wxObject &obj, wxEvent &ev);

static ToolBarEntry main_tb[] = {
  { 0, NULL, "Previous stuntsheet", toolbar_prev_ss },
  { TOOLBAR_SPACE, NULL, "Next stuntsheet", toolbar_next_ss },
  { 0, NULL, "Select points with box", toolbar_box },
  { 0, NULL, "Select points with polygon", toolbar_poly },
  { TOOLBAR_SPACE, NULL, "Select points with lasso", toolbar_lasso },
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

CC_WinNodeMain::CC_WinNodeMain(CC_WinList *lst, FieldCanvas *canv)
: CC_WinNode(lst), canvas(canv) {}

void CC_WinNodeMain::SetShow(CC_show *shw) {
  Remove();
  list = shw->winlist;
  shw->winlist->Add(this);
  canvas->show_descr.show = shw;
  winlist.SetShow(shw); // Must set new show before redrawing
  canvas->UpdateBars();
  canvas->GotoSS(0);
  ChangeName();
}
void CC_WinNodeMain::ChangeName() {
  canvas->ourframe->SetTitle((char *)canvas->show_descr.show->UserGetName());
  winlist.ChangeName();
}
void CC_WinNodeMain::UpdateSelections(wxWindow* win, int point) {
  if (canvas->GetDC()->Colour) {
    canvas->RefreshShow(FALSE, point);
  } else {
    // In mono we use different line widths, so must redraw everything
    canvas->RefreshShow();
  }
  winlist.UpdateSelections(win, point);
}
void CC_WinNodeMain::UpdatePoints() {
  canvas->RefreshShow();
  winlist.UpdatePoints();
}
void CC_WinNodeMain::UpdatePointsOnSheet(unsigned sht, int ref) {
  if (sht == canvas->show_descr.curr_ss) {
    if ((ref < 0) || (ref == (int)canvas->curr_ref)) {
      canvas->RefreshShow();
    }
  }
  winlist.UpdatePointsOnSheet(sht, ref);
}
void CC_WinNodeMain::ChangeNumPoints(wxWindow *win) {
  canvas->UpdateSS();
  winlist.ChangeNumPoints(win);
}
void CC_WinNodeMain::ChangePointLabels(wxWindow *win) {
  canvas->UpdateSS();
  winlist.ChangePointLabels(win);
}
void CC_WinNodeMain::ChangeShowMode(wxWindow *win) {
  canvas->UpdateBars();
  canvas->UpdateSS();
  winlist.ChangeShowMode(win);
}
void CC_WinNodeMain::UpdateStatusBar() {
  canvas->UpdatePanel();
  winlist.UpdateStatusBar();
}
void CC_WinNodeMain::GotoSheet(unsigned sht) {
  winlist.GotoSheet(sht);
}
void CC_WinNodeMain::AddSheet(unsigned sht) {
  if (sht <= canvas->show_descr.curr_ss) {
    canvas->show_descr.curr_ss++;
  }
  canvas->UpdatePanel();
  winlist.AddSheet(sht);
}
void CC_WinNodeMain::DeleteSheet(unsigned sht) {
  winlist.DeleteSheet(sht);
  if (sht < canvas->show_descr.curr_ss) {
    canvas->show_descr.curr_ss--;
  }
  if (canvas->show_descr.curr_ss >= canvas->show_descr.show->GetNumSheets()) {
    canvas->PrevSS();
  } else {
    if (sht == canvas->show_descr.curr_ss) {
      canvas->GotoThisSS();
    } else {
      canvas->UpdatePanel();
    }
  }
}
void CC_WinNodeMain::AppendSheets() {
  canvas->UpdatePanel();
  winlist.AppendSheets();
}
void CC_WinNodeMain::RemoveSheets(unsigned num) {
  winlist.RemoveSheets(num);
  if (canvas->show_descr.curr_ss >= canvas->show_descr.show->GetNumSheets()) {
    canvas->GotoSS(num-1);
  }
}
void CC_WinNodeMain::ChangeTitle(unsigned sht) {
  if (sht == canvas->show_descr.curr_ss) canvas->UpdatePanel();
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
  modelist = new ShowModeList();

  char *s = ReadConfig();
  if (s) {
    (void)wxMessageBox(s, "CalChart");
  }

  //Create toolbar bitmaps
  unsigned i = 0;

  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_left));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_right));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_box));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_poly));
  main_tb[i++].bm = new wxBitmap(BITMAP_NAME(tb_lasso));
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

  help_inst = new wxHelpInstance(TRUE);
  help_inst->Initialize("charthlp");

  return new TopFrame();
}

int CalChartApp::OnExit(void) {
  if (modelist) delete modelist;
  if (help_inst) delete help_inst;
  if (window_list) delete window_list;

  return 0;
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

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(CALCHART__ABOUT, "&About CalChart...");
  help_menu->Append(CALCHART__HELP, "&Help on CalChart...");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, "&File");
  menu_bar->Append(edit_menu, "&Edit");
  menu_bar->Append(win_menu, "&Windows");
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
  } else {
    show = other_frame->field->show_descr.show;
    ss = other_frame->field->show_descr.curr_ss;
    def_zoom = other_frame->zoom_slider->GetValue();
    def_grid = other_frame->grid_choice->GetSelection();
    def_ref = other_frame->field->curr_ref;
  }

  SetTitle((char *)show->UserGetName());
  field = new FieldCanvas(show, ss, this, def_zoom);
  field->curr_ref = def_ref;
  frameCanvas = field;
  node = new CC_WinNodeMain(show->winlist, field);

  // Add the controls
  framePanel = new wxPanel(this);
  {
    char buf[4];
    unsigned i;

    ref_choice = new ChoiceWithField(framePanel, (wxFunction)refnum_callback,
				     "&Ref Group");
    ref_choice->Append("Off");
    for (i = 1; i <= NUM_REF_PNTS; i++) {
      sprintf(buf, "%u", i);
      ref_choice->Append(buf);
    }
  }
  ((ChoiceWithField*)ref_choice)->field = field;
  ref_choice->SetSelection(def_ref);
  grid_choice = new wxChoice(framePanel, (wxFunction)NULL,
			     "&Grid", -1, -1, -1, -1,
			     sizeof(gridtext)/sizeof(char*),
			     gridtext);
  grid_choice->SetSelection(def_grid);
  SliderWithField *sldr = new SliderWithField(framePanel, slider_zoom_callback,
					      "&Zoom", def_zoom,
					      1, FIELD_MAXZOOM, 150);
  sldr->field = field;
  zoom_slider = sldr;

  // Show the frame
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
      char buf[16];
      sprintf(buf, "%u", field->show_descr.CurrSheet()->beats);
      s = wxGetTextFromUser("Enter the number of beats",
			    (char *)field->show_descr.CurrSheet()->GetName(),
			    buf, this);
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
  case CALCHART__ABOUT:
    (void)wxMessageBox("CalChart v3.0\nAuthor: Garrick Meeker\nhttp://www.calband.berkeley.edu/calchart\n(c) 1995", "About CalChart");
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
  char buf[1024];

  if (field->show_descr.show->Modified()) {
    if (!field->show_descr.show->winlist->MultipleWindows()) {
      sprintf(buf, "Save changes to '%s'?",
	      field->show_descr.show->UserGetName());
      switch (wxMessageBox(buf, "Unsaved changes",
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
			 int def_zoom, int x, int y, int w, int h):
 AutoScrollCanvas(frame, x, y, w, h), ourframe(frame), curr_lasso(CC_DRAG_BOX),
 curr_ref(0), drag(CC_DRAG_NONE), dragon(FALSE)
{
  SetColourMap(CalChartColorMap);

  show_descr.show = show;
  show_descr.curr_ss = ss;

  SetZoomQuick(def_zoom);

  SetBackground(CalChartBrushes[COLOR_FIELD]);

  UpdateBars();
  UpdatePanel();
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
      } else {
	if (event.LeftUp()) {
	  CC_DRAG_TYPES drag_tmp = drag;
	  EndDrag();
	  switch (drag_tmp) {
	  case CC_DRAG_BOX:
	    if (sheet->SelectPointsInRect(drag_start, drag_end))
	      show_descr.show->winlist->UpdateSelections();
	    break;
	  case CC_DRAG_LINE:
	    pos.x = drag_end.x - drag_start.x;
	    pos.y = drag_end.y - drag_start.y;
	    if (sheet->TranslatePoints(pos, curr_ref))
	      show_descr.show->winlist->
		UpdatePointsOnSheet(show_descr.curr_ss, curr_ref);
	    break;
	  default:
	    break;
	  }
	} else {
	  if (event.Dragging() && event.LeftIsDown()) {
	    if (drag != CC_DRAG_BOX) {
	      ourframe->SnapToGrid(pos);
	    }
	    MoveDrag(pos);
	  }
	}
      }
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

void FieldCanvas::UpdatePanel() {
  char tempbuf[256];
  CC_sheet *sht = show_descr.CurrSheet();

  sprintf(tempbuf, "%s%d of %d \"%.32s\" %d beats",
	  show_descr.show->Modified() ? "* ":"", show_descr.curr_ss+1,
	  show_descr.show->GetNumSheets(), sht->GetName(), sht->beats);
  ourframe->SetStatusText(tempbuf, 1);
}

void FieldCanvas::UpdateBars() {
  if (show_descr.show) {
    SetSize(COORD2INT(show_descr.show->mode->Size().x) * zoomf,
	    COORD2INT(show_descr.show->mode->Size().y) * zoomf);
  }
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

static void slider_zoom_callback(wxObject &obj, wxEvent &) {
  SliderWithField *slider = (SliderWithField *)&obj;
  slider->field->SetZoom(slider->GetValue());
}
