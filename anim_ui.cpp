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

static void toolbar_anim_stop(CoolToolBar *tb);
static void toolbar_anim_play(CoolToolBar *tb);
static void toolbar_anim_prev_beat(CoolToolBar *tb);
static void toolbar_anim_next_beat(CoolToolBar *tb);
static void toolbar_anim_prev_sheet(CoolToolBar *tb);
static void toolbar_anim_next_sheet(CoolToolBar *tb);
static void slider_anim_tempo(wxObject &obj, wxEvent &ev);

ToolBarEntry anim_tb[] = {
  { NULL, "Stop", toolbar_anim_stop },
  { NULL, "Play", toolbar_anim_play },
  { NULL, "Previous beat", toolbar_anim_prev_beat },
  { NULL, "Next beat", toolbar_anim_next_beat },
  { NULL, "Previous stuntsheet", toolbar_anim_prev_sheet },
  { NULL, "Next stuntsheet", toolbar_anim_next_sheet }
};

extern wxBrush *grassBrush;
extern wxBrush *hilitBrush;
extern wxPen *hilitPen;
extern wxBrush *animhilitBrush;
extern wxPen *animhilitPen;
extern wxBrush *blueBrush;
extern wxPen *bluePen;

CC_WinNodeAnim::CC_WinNodeAnim(CC_WinList *lst, AnimationFrame *frm)
: CC_WinNode(lst), frame(frm) {}

void CC_WinNodeAnim::SetShow(CC_show *) {
  frame->canvas->FreeAnim();
}

void CC_WinNodeAnim::UpdateSelections() {
  frame->canvas->OnPaint();
}

void CC_WinNodeAnim::ChangeNumPoints(wxWindow *) {
  frame->canvas->FreeAnim();
}

void AnimationTimer::Notify() {
  if (!canvas->NextBeat()) Stop();
}

AnimationCanvas::AnimationCanvas(wxFrame *frame, CC_descr *dcr)
: wxCanvas(frame, -1, -1,
	   COORD2INT(dcr->show->mode->Size().x)*DEFAULT_ANIM_SIZE,
	   COORD2INT(dcr->show->mode->Size().y)*DEFAULT_ANIM_SIZE, 0),
  anim(NULL), show_descr(dcr), ourframe(frame), tempo(120) {
  float f;

  timer = new AnimationTimer(this);

  GetDC()->SetMapMode(MM_TEXT);

  if (GetDC()->Colour) {
    SetBackground(grassBrush);
  } else {
    SetBackground(wxWHITE_BRUSH);
  }
  f = DEFAULT_ANIM_SIZE * (COORD2INT(1 << 16)/65536.0);
  GetDC()->SetUserScale(f, f);
}

AnimationCanvas::~AnimationCanvas() {
  if (timer) delete timer;
  if (anim) delete anim;
}

void AnimationCanvas::OnPaint() {
  unsigned long x, y;
  unsigned i;
  wxDC *dc = GetDC();

  dc->BeginDrawing();

  dc->Clear();
  if (anim)
  for (i = 0; i < anim->numpts; i++) {
    if (dc->Colour) {
      if (show_descr->show->IsSelected(i)) {
	dc->SetPen(animhilitPen);
	dc->SetBrush(animhilitBrush);
      } else {
	if (anim->curr_cmds[i]) {
	  switch (anim->curr_cmds[i]->Direction()) {
	  case ANIMDIR_SW:
	  case ANIMDIR_W:
	  case ANIMDIR_NW:
	    dc->SetPen(hilitPen);
	    dc->SetBrush(hilitBrush);
	    break;
	  case ANIMDIR_SE:
	  case ANIMDIR_E:
	  case ANIMDIR_NE:
	    dc->SetPen(wxWHITE_PEN);
	    dc->SetBrush(wxWHITE_BRUSH);
	    break;
	  default:
	    dc->SetPen(bluePen);
	    dc->SetBrush(blueBrush);
	  }
	} else {
	  dc->SetPen(wxWHITE_PEN);
	  dc->SetBrush(wxWHITE_BRUSH);
	}
      }
    } else {
      dc->SetPen(wxBLACK_PEN);
      if (show_descr->show->IsSelected(i)) {
	dc->SetBrush(wxTRANSPARENT_BRUSH);
      } else {
	dc->SetBrush(wxBLACK_BRUSH);
      }
    }
    x = anim->pts[i].pos.x+show_descr->show->mode->Offset().x;
    y = anim->pts[i].pos.y+show_descr->show->mode->Offset().y;
    dc->DrawRectangle(x - INT2COORD(1)/2, y - INT2COORD(1)/2,
		      INT2COORD(1), INT2COORD(1));
  }

  dc->EndDrawing();
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
    sprintf(tempbuf, "Beat %u of %u  \"%s\"", anim->curr_beat,
	    anim->curr_sheet->numbeats, anim->curr_sheet->name);
    ourframe->SetStatusText(tempbuf, 1);
  } else {
    ourframe->SetStatusText("No animation available", 1);
  }
}

void AnimationCanvas::Generate() {
  timer->Stop();
  ourframe->SetStatusText("Compiling...");
  if (anim) delete anim;
  anim = new Animation(show_descr->show);
  if (anim) anim->GotoSheet(show_descr->curr_ss);
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
  CreateStatusLine(2);

  // Make a menubar
  wxMenu *anim_menu = new wxMenu;
  anim_menu->Append(CALCHART__ANIM_REANIMATE, "Reanimate Show");
  anim_menu->Append(CALCHART__ANIM_CLOSE, "Close Animation");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(anim_menu, "Animate");
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
  AnimationSlider *sldr =
    new AnimationSlider(framePanel, slider_anim_tempo, "Tempo",
			canvas->GetTempo(), 10, 300, 150);
  sldr->canvas = canvas;

  node = new CC_WinNodeAnim(lst, this);

  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  Fit();
  Show(TRUE);

  canvas->Generate();
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
