/* anim_ui.cc
 * Animation user interface
 *
 * Modification history:
 * 1-4-96     Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "anim_ui.h"
#include "modes.h"
#include "confgr.h"

static void toolbar_anim_stop(CoolToolBar *tb);
static void toolbar_anim_play(CoolToolBar *tb);
static void toolbar_anim_prev_beat(CoolToolBar *tb);
static void toolbar_anim_next_beat(CoolToolBar *tb);
static void toolbar_anim_prev_sheet(CoolToolBar *tb);
static void toolbar_anim_next_sheet(CoolToolBar *tb);
static void slider_anim_tempo(wxObject &obj, wxEvent &ev);

ToolBarEntry anim_tb[] = {
  { 0, NULL, "Stop", toolbar_anim_stop },
  { 0, NULL, "Play", toolbar_anim_play },
  { 0, NULL, "Previous beat", toolbar_anim_prev_beat },
  { 0, NULL, "Next beat", toolbar_anim_next_beat },
  { 0, NULL, "Previous stuntsheet", toolbar_anim_prev_sheet },
  { 0, NULL, "Next stuntsheet", toolbar_anim_next_sheet }
};

CC_WinNodeAnim::CC_WinNodeAnim(CC_WinList *lst, AnimationFrame *frm)
: CC_WinNode(lst), frame(frm) {}

void CC_WinNodeAnim::SetShow(CC_show *) {
  frame->canvas->FreeAnim();
}

void CC_WinNodeAnim::UpdateSelections(wxWindow*, int) {
  frame->canvas->OnPaint();
}

void CC_WinNodeAnim::ChangeNumPoints(wxWindow *) {
  frame->canvas->FreeAnim();
}

CC_WinNodeAnimErrors::CC_WinNodeAnimErrors(CC_WinList *lst, AnimErrorList *err)
: CC_WinNode(lst), errlist(err) {}

void CC_WinNodeAnimErrors::SetShow(CC_show *) {
  errlist->Close();
}

void CC_WinNodeAnimErrors::UpdateSelections(wxWindow *win, int) {
  if (win != errlist) {
    errlist->Unselect();
  }
}

void CC_WinNodeAnimErrors::ChangeNumPoints(wxWindow *) {
  errlist->Close();
}

void AnimationTimer::Notify() {
  if (!canvas->NextBeat()) Stop();
}

AnimationCanvas::AnimationCanvas(wxFrame *frame, CC_descr *dcr)
: AutoScrollCanvas(frame, -1, -1,
	   COORD2INT(dcr->show->mode->Size().x)*DEFAULT_ANIM_SIZE,
	   COORD2INT(dcr->show->mode->Size().y)*DEFAULT_ANIM_SIZE),
  anim(NULL), show_descr(dcr), ourframe(frame), tempo(120) {
  float f;

  SetColourMap(CalChartColorMap);

  timer = new AnimationTimer(this);

  SetBackground(CalChartBrushes[COLOR_FIELD]);
  f = DEFAULT_ANIM_SIZE * (COORD2INT(1 << 16)/65536.0);
  SetUserScale(f, f);
}

AnimationCanvas::~AnimationCanvas() {
  if (timer) delete timer;
  if (anim) delete anim;
}

void AnimationCanvas::OnPaint() {
  unsigned long x, y;
  unsigned i;
  wxDC *dc = GetMemDC();

  dc->BeginDrawing();

  dc->Clear();
  dc->SetPen(CalChartPens[COLOR_FIELD_DETAIL]);
  show_descr->show->mode->DrawAnim(dc);
  if (anim)
  for (i = 0; i < anim->numpts; i++) {
    if (show_descr->show->IsSelected(i)) {
      if (anim->curr_cmds[i]) {
	switch (anim->curr_cmds[i]->Direction()) {
	case ANIMDIR_SW:
	case ANIMDIR_W:
	case ANIMDIR_NW:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_HILIT_BACK]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_HILIT_BACK]);
	  break;
	case ANIMDIR_SE:
	case ANIMDIR_E:
	case ANIMDIR_NE:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_HILIT_FRONT]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_HILIT_FRONT]);
	  break;
	default:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_HILIT_SIDE]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_HILIT_SIDE]);
	}
      } else {
	dc->SetPen(CalChartPens[COLOR_POINT_ANIM_HILIT_FRONT]);
	dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_HILIT_FRONT]);
      }
    } else {
      if (anim->curr_cmds[i]) {
	switch (anim->curr_cmds[i]->Direction()) {
	case ANIMDIR_SW:
	case ANIMDIR_W:
	case ANIMDIR_NW:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_BACK]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_BACK]);
	  break;
	case ANIMDIR_SE:
	case ANIMDIR_E:
	case ANIMDIR_NE:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_FRONT]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_FRONT]);
	  break;
	default:
	  dc->SetPen(CalChartPens[COLOR_POINT_ANIM_SIDE]);
	  dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_SIDE]);
	}
      } else {
	dc->SetPen(CalChartPens[COLOR_POINT_ANIM_FRONT]);
	dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_FRONT]);
      }
    }
    x = anim->pts[i].pos.x+show_descr->show->mode->Offset().x;
    y = anim->pts[i].pos.y+show_descr->show->mode->Offset().y;
    dc->DrawRectangle(x - INT2COORD(1)/2, y - INT2COORD(1)/2,
		      INT2COORD(1), INT2COORD(1));
  }

  dc->EndDrawing();
  Blit();
}

void AnimationCanvas::OnEvent(wxMouseEvent& event) {
  if (event.LeftDown()) PrevBeat();
  if (event.RightDown()) NextBeat();
}

void AnimationCanvas::OnChar(wxKeyEvent&) {
}

void AnimationCanvas::SetTempo(unsigned t) {
  tempo = t;
}

void AnimationCanvas::UpdateText() {
  char tempbuf[100];

  if (anim) {
    sprintf(tempbuf, "Beat %u of %u  Sheet %d of %d \"%.32s\"",
	    anim->curr_beat, anim->curr_sheet->numbeats,
	    anim->curr_sheetnum+1, anim->numsheets,
	    anim->curr_sheet->name);
    ourframe->SetStatusText(tempbuf, 1);
  } else {
    ourframe->SetStatusText("No animation available", 1);
  }
}

void AnimationCanvas::Generate() {
  timer->Stop();
  ourframe->SetStatusText("Compiling...");
  if (anim) {
    delete anim;
    anim = NULL;
    Refresh();
  }
  anim = new Animation(show_descr->show, ourframe,
		       ((AnimationFrame*)ourframe)->node->GetList());
  if (anim) {
    anim->GotoSheet(show_descr->curr_ss);
  }
  Refresh();
  ourframe->SetStatusText("Ready");
}

void AnimationCanvas::FreeAnim() {
  if (anim) {
    delete anim;
    anim = NULL;
    Refresh();
  }
}

AnimationFrame::AnimationFrame(wxFrame *frame, CC_descr *dcr,
			       CC_WinList *lst)
: wxFrameWithStuffSized(frame, "Animation") {
  // Give it an icon
  SetBandIcon(this);

  CreateStatusLine(2);

  // Make a menubar
  wxMenu *anim_menu = new wxMenu;
  anim_menu->Append(CALCHART__ANIM_REANIMATE, "&Reanimate Show");
  anim_menu->Append(CALCHART__ANIM_CLOSE, "&Close Animation");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(anim_menu, "&Animate");
  SetMenuBar(menu_bar);

  // Add a toolbar
  CoolToolBar *ribbon = new CoolToolBar(this, 0, 0, -1, -1, 0,
					wxHORIZONTAL, 20);
  ribbon->SetupBar(anim_tb, sizeof(anim_tb)/sizeof(ToolBarEntry));
  SetToolBar(ribbon);

  // Add the field canvas
  canvas = new AnimationCanvas(this, dcr);
  SetCanvas(canvas);
  canvas->UpdateText();

  // Add the controls
  SetPanel(new wxPanel(this));
  (void)new wxCheckBox(framePanel, (wxFunction)NULL, "&Collisions");
  AnimationSlider *sldr =
    new AnimationSlider(framePanel, slider_anim_tempo, "&Tempo",
			canvas->GetTempo(), 10, 300, 150);
  sldr->canvas = canvas;

  node = new CC_WinNodeAnim(lst, this);

  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  Fit();
  Show(TRUE);
}

AnimationFrame::~AnimationFrame() {
  if (node) {
    node->Remove();
    delete node;
  }
}

void AnimationFrame::OnMenuCommand(int id) {
  switch (id) {
  case CALCHART__ANIM_REANIMATE:
    canvas->Generate();
    break;
  case CALCHART__ANIM_CLOSE:
    Close();
    break;
  }
}

void AnimationFrame::OnMenuSelect(int id) {
  char *msg = NULL;

  switch (id) {
  case CALCHART__ANIM_REANIMATE:
    msg = "Regenerate animation";
    break;
  case CALCHART__ANIM_CLOSE:
    msg = "Close window";
    break;
  }
  if (msg) SetStatusText(msg);
}

static void toolbar_anim_stop(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->timer->Stop();
}

static void toolbar_anim_play(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  if (!af->canvas->timer->Start(60000/af->canvas->GetTempo())) {
    af->SetStatusText("Could not get timer!");
  }
}

static void toolbar_anim_prev_beat(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->PrevBeat();
}

static void toolbar_anim_next_beat(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->NextBeat();
}

static void toolbar_anim_prev_sheet(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->PrevSheet();
}

static void toolbar_anim_next_sheet(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->NextSheet();
}

static void slider_anim_tempo(wxObject &obj, wxEvent &) {
  AnimationSlider *slider = (AnimationSlider*)&obj;
  slider->canvas->SetTempo(slider->GetValue());
}

static void AnimErrorClose(wxButton& button, wxEvent&) {
  ((AnimErrorList*)button.GetParent()->GetParent())->Close();
}

static void AnimErrorClick(wxListBox& list, wxCommandEvent&) {
  AnimErrorList *err = (AnimErrorList*)list.GetParent()->GetParent();

  err->Update();
}

AnimErrorList::AnimErrorList(AnimateCompile *comp, CC_WinList *lst,
			     wxFrame *frame, char *title,
			     int x, int y, int width, int height)
: wxFrame(frame, title, x, y, width, height, wxSDI | wxDEFAULT_FRAME) {
  unsigned i, j;

  for (i = 0; i < NUM_ANIMERR; i++) {
    pointsels[i] = NULL;
  }

  // Give it an icon
  SetBandIcon(this);

  SetAutoLayout(TRUE);

  show = comp->show;

  panel = new wxPanel(this);

  wxButton *closeBut = new wxButton(panel, (wxFunction)AnimErrorClose,
				    "Close");
  wxLayoutConstraints *bt0 = new wxLayoutConstraints;
  bt0->left.SameAs(panel, wxLeft, 5);
  bt0->top.SameAs(panel, wxTop, 5);
  bt0->width.AsIs();
  bt0->height.AsIs();
  closeBut->SetConstraints(bt0);

  closeBut->SetDefault();

  list = new GoodListBox(panel, (wxFunction)AnimErrorClick, "",
			 wxSINGLE, -1, -1, -1, -1);

  for (i = 0, j = 0; i < NUM_ANIMERR; i++) {
    if (comp->error_markers[i]) {
      list->Append((char*)animate_err_msgs[i]);
      pointsels[j++] = comp->StealErrorMarker(i);
    }
  }

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

  node = new CC_WinNodeAnimErrors(lst, this);

  OnSize(-1,-1);

  Show(TRUE);
}

AnimErrorList::~AnimErrorList() {
  unsigned i;

  if (node) {
    node->Remove();
    delete node;
  }
  for (i = 0; i < NUM_ANIMERR; i++) {
    if (pointsels[i]) delete [] pointsels[i];
  }
}

void AnimErrorList::OnSize(int, int) {
  Layout();
}

Bool AnimErrorList::OnClose(void) {
  return TRUE;
}

void AnimErrorList::Unselect() {
  int i = list->GetSelection();

  if (i >= 0) {
    list->Deselect(i);
  }
}

void AnimErrorList::Update() {
  int i = list->GetSelection();

  if (i >= 0) {
    for (unsigned j = 0; j < show->GetNumPoints(); j++) {
      show->Select(j, pointsels[i][j]);
    }
    show->winlist->UpdateSelections(this);
  }
}
