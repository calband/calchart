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
#ifdef ANIM_OUTPUT_RIB
#include <ri.h>
#endif

static void toolbar_anim_stop(CoolToolBar *tb);
static void toolbar_anim_play(CoolToolBar *tb);
static void toolbar_anim_prev_beat(CoolToolBar *tb);
static void toolbar_anim_next_beat(CoolToolBar *tb);
static void toolbar_anim_prev_sheet(CoolToolBar *tb);
static void toolbar_anim_next_sheet(CoolToolBar *tb);
static void collision_callback(wxChoice &ch, wxEvent &ev);
static void slider_anim_tempo(wxObject &obj, wxEvent &ev);
static void slider_gotosheet(wxObject &obj, wxEvent &ev);
static void slider_gotobeat(wxObject &obj, wxEvent &ev);

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
  frame->canvas->Redraw();
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

AnimationCanvas::AnimationCanvas(AnimationFrame *frame, CC_descr *dcr)
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
  Blit();
}

void AnimationCanvas::RedrawBuffer() {
  unsigned long x, y;
  unsigned i;
  wxDC *dc = GetMemDC();

  dc->BeginDrawing();

  dc->Clear();
  dc->SetPen(CalChartPens[COLOR_FIELD_DETAIL]);
  show_descr->show->mode->DrawAnim(dc);
  if (anim)
  for (i = 0; i < anim->numpts; i++) {
    if (anim->collisions[i]) {
      dc->SetPen(CalChartPens[COLOR_POINT_ANIM_COLLISION]);
      dc->SetBrush(CalChartBrushes[COLOR_POINT_ANIM_COLLISION]);
    } else if (show_descr->show->IsSelected(i)) {
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
}

void AnimationCanvas::OnEvent(wxMouseEvent& event) {
  if (event.LeftDown()) PrevBeat();
  if (event.RightDown()) NextBeat();
}

void AnimationCanvas::OnChar(wxKeyEvent&) {
}

void AnimationCanvas::SetTempo(unsigned t) {
  tempo = t;
  if (timeron) {
    StartTimer();
  }
}

void AnimationCanvas::UpdateText() {
  wxString tempbuf;

  if (anim) {
    tempbuf.sprintf("Beat %u of %u  Sheet %d of %d \"%.32s\"",
		    anim->curr_beat, anim->curr_sheet->numbeats,
		    anim->curr_sheetnum+1, anim->numsheets,
		    anim->curr_sheet->name);
    ourframe->SetStatusText(tempbuf.GetData(), 1);
  } else {
    ourframe->SetStatusText("No animation available", 1);
  }
}

void AnimationCanvas::Refresh() {
  Redraw();
  UpdateText();
  ourframe->UpdatePanel();
}

void AnimationCanvas::Generate() {
  StopTimer();
  ourframe->SetStatusText("Compiling...");
  if (anim) {
    delete anim;
    anim = NULL;
    Refresh();
  }
  anim = new Animation(show_descr->show, ourframe,
		       ((AnimationFrame*)ourframe)->node->GetList());
  if (anim && (anim->numsheets == 0)) {
    delete anim;
    anim = NULL;
  }
  if (anim) {
    anim->EnableCollisions(((AnimationFrame*)ourframe)->CollisionType());
    anim->GotoSheet(show_descr->curr_ss);
  }
  ourframe->UpdatePanel();
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

void AnimationCanvas::SelectCollisions() {
  unsigned i;

  if (anim) {
    for (i = 0; i < anim->numpts; i++) {
      if (anim->collisions[i]) {
	show_descr->show->Select(i);
      } else {
	show_descr->show->Select(i, FALSE);
      }
    }
    show_descr->show->winlist->UpdateSelections();
  }
}

#ifdef ANIM_OUTPUT_POVRAY
#define POV_SCALE 16
const char *AnimationCanvas::GeneratePOVFiles(const char *filebasename) {
  unsigned framenum, pt;
  wxString filename;
  FILE *fp;
  float x, y;
  Bool east = 1;
  wxPen *pen;

  if (anim) {
    GotoSheet(0);
    framenum = 1;
    do {
      filename.sprintf("%s%05d.pov", filebasename, framenum);
      fp = fopen(filename.Chars(), "w");
      if (fp == NULL) {
	return "Error opening file";
      }
      for (pt = 0; pt < anim->numpts; pt++) {
	x = COORD2FLOAT(anim->pts[pt].pos.x) * POV_SCALE;
	y = COORD2FLOAT(anim->pts[pt].pos.y) * POV_SCALE;
	/* x is already flipped because of the different axises */
	if (east) {
	  x = (-x);
	} else {
	  y = (-y);
	}
	fprintf(fp, "cylinder { <%f, 0.0, %f>, <%f, %f, %f> %f\n",
		x, y,
		x, 4.0 * POV_SCALE, y,
		0.75 * POV_SCALE);
	if (anim->curr_cmds[pt]) {
	  switch (anim->curr_cmds[pt]->Direction()) {
	  case ANIMDIR_SW:
	  case ANIMDIR_W:
	  case ANIMDIR_NW:
	    pen = CalChartPens[COLOR_POINT_ANIM_BACK];
	    break;
	  case ANIMDIR_SE:
	  case ANIMDIR_E:
	  case ANIMDIR_NE:
	    pen = CalChartPens[COLOR_POINT_ANIM_FRONT];
	    break;
	  default:
	    pen = CalChartPens[COLOR_POINT_ANIM_SIDE];
	  }
	} else {
	    pen = CalChartPens[COLOR_POINT_ANIM_FRONT];
	}
	fprintf(fp, "pigment { colour red %f green %f blue %f }\n",
		pen->GetColour().Red() / 255.0,
		pen->GetColour().Green() / 255.0,
		pen->GetColour().Blue() / 255.0);
	fprintf(fp, "}\n");
      }
      fclose(fp);
      framenum++;
    } while (NextBeat());
  }
  return NULL;
}
#endif

#ifdef ANIM_OUTPUT_RIB

static char *texturefile = "memorial.tif";
const char * AnimationCanvas::GenerateRIBFrame() {
  unsigned pt;
  float x, y;
  Bool east = 1;
  wxPen *pen;
  RtColor color;
  static RtFloat amb_intensity = 0.2, dist_intensity = 0.9;
  RtPoint field[4];

  if (!anim) return NULL;

  field[0][0] = -1408;
  field[0][1] = 400;
  field[0][2] = 0;
  field[1][0] = 1408;
  field[1][1] = 400;
  field[1][2] = 0;
  field[2][0] = 1408;
  field[2][1] = -400;
  field[2][2] = 0;
  field[3][0] = -1408;
  field[3][1] = -400;
  field[3][2] = 0;

  RiWorldBegin();
  RiAttributeBegin();
  RiLightSource(RI_AMBIENTLIGHT, RI_INTENSITY, &amb_intensity, RI_NULL);
  RiLightSource(RI_DISTANTLIGHT, RI_INTENSITY, &dist_intensity, RI_NULL);
  RiDeclare("tmap", "uniform string");
  RiSurface("fieldtexture", "tmap", &texturefile, RI_NULL);
  RiPolygon(4, RI_P, (RtPointer)field, RI_NULL);
  for (pt = 0; pt < anim->numpts; pt++) {
    if (anim->curr_cmds[pt]) {
      switch (anim->curr_cmds[pt]->Direction()) {
      case ANIMDIR_SW:
      case ANIMDIR_W:
      case ANIMDIR_NW:
	pen = CalChartPens[COLOR_POINT_ANIM_BACK];
	break;
      case ANIMDIR_SE:
      case ANIMDIR_E:
      case ANIMDIR_NE:
	pen = CalChartPens[COLOR_POINT_ANIM_FRONT];
	break;
      default:
	pen = CalChartPens[COLOR_POINT_ANIM_SIDE];
      }
    } else {
      pen = CalChartPens[COLOR_POINT_ANIM_FRONT];
    }
    color[0] = pen->GetColour().Red() / 255.0;
    color[1] = pen->GetColour().Green() / 255.0;
    color[2] = pen->GetColour().Blue() / 255.0;
    RiColor(color);
    x = COORD2FLOAT(anim->pts[pt].pos.x) * POV_SCALE;
    y = COORD2FLOAT(anim->pts[pt].pos.y) * POV_SCALE;
    /* x is already flipped because of the different axises */
    if (east) {
      x = (-x);
    } else {
      y = (-y);
    }
    RiTransformBegin();
    RiTranslate(x, y, 0.0);
    RiCylinder(0.75 * POV_SCALE, -4.0 * POV_SCALE, 0.0, 360.0, RI_NULL);
    RiDisk(-4.0 * POV_SCALE, 0.75 * POV_SCALE, 360.0, RI_NULL);
    RiTransformEnd();
  }
  RiAttributeEnd();
  RiWorldEnd();
  return NULL;
}

const char * AnimationCanvas::GenerateRIBFile(const char *filename,
					      Bool allframes) {
  unsigned framenum;
  const char *err;

  if (anim) {
    if (allframes) {
      GotoSheet(0);
    }
    framenum = 1;
    RiBegin((char *)filename);
    RiProjection(RI_PERSPECTIVE, RI_NULL);
    RiRotate(45.0, 1.0, 0.0, 0.0);
    RiTranslate(0.0, 1471.0, 1471.0);
    if (allframes) do {
      RiFrameBegin(framenum);
      err = GenerateRIBFrame();
      if (err) return err;
      RiFrameEnd();
      framenum++;
    } while (NextBeat());
    else {
      err = GenerateRIBFrame();
      if (err) return err;
    }
    RiEnd();
  }
  return NULL;
}
#endif

static char *collis_text[] = {
  "Ignore", "Show", "Beep"
};

AnimationFrame::AnimationFrame(wxFrame *frame, CC_descr *dcr,
			       CC_WinList *lst)
: wxFrameWithStuffSized(frame, "Animation") {
  // Give it an icon
  SetBandIcon(this);

  CreateStatusLine(2);

  // Make a menubar
  wxMenu *anim_menu = new wxMenu;
  anim_menu->Append(CALCHART__ANIM_REANIMATE, "&Reanimate Show");
  anim_menu->Append(CALCHART__ANIM_SELECT_COLL, "&Select Collisions");
#ifdef ANIM_OUTPUT_POVRAY
  anim_menu->Append(CALCHART__ANIM_POVRAY, "Output &POVRay Stubs");
#endif
#ifdef ANIM_OUTPUT_RIB
  anim_menu->Append(CALCHART__ANIM_RIB_FRAME, "Output RenderMan RIB Frame");
  anim_menu->Append(CALCHART__ANIM_RIB, "Output Render&Man RIB");
#endif
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
  framePanel->SetAutoLayout(TRUE);

  collis = new wxChoice(framePanel, (wxFunction)collision_callback,
			"&Collisions", -1, -1, -1, -1,
			sizeof(collis_text)/sizeof(char*), collis_text);

  AnimationSlider *sldr =
    new AnimationSlider(framePanel, slider_anim_tempo, "&Tempo",
			canvas->GetTempo(), 10, 300, 150);
  sldr->canvas = canvas;
  wxLayoutConstraints *lc = new wxLayoutConstraints;
  lc->left.AsIs();
  lc->top.AsIs();
  lc->right.SameAs(framePanel, wxRight, 5);
  lc->height.AsIs();
  sldr->SetConstraints(lc);

  framePanel->NewLine();

  // Sheet slider (will get set later with UpdatePanel())
  sldr = new AnimationSlider(framePanel, slider_gotosheet, "&Sheet",
			     1, 1, 2, 150);
  sldr->canvas = canvas;
  sheet_slider = sldr;
  lc = new wxLayoutConstraints;
  lc->left.AsIs();
  lc->top.AsIs();
  lc->right.PercentOf(framePanel, wxWidth, 50);
  lc->height.AsIs();
  sldr->SetConstraints(lc);

  // Beat slider (will get set later with UpdatePanel())
  sldr = new AnimationSlider(framePanel, slider_gotobeat, "&Beat",
			     0, 0, 1, 150);
  sldr->canvas = canvas;
  beat_slider = sldr;
  lc = new wxLayoutConstraints;
  lc->left.RightOf(sheet_slider, 5);
  lc->top.AsIs();
  lc->right.SameAs(framePanel, wxRight, 5);
  lc->height.AsIs();
  sldr->SetConstraints(lc);

  node = new CC_WinNodeAnim(lst, this);

  UpdatePanel();
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
  const char *s;
  const char *err;
  Bool allframes = TRUE;

  switch (id) {
  case CALCHART__ANIM_REANIMATE:
    canvas->Generate();
    break;
  case CALCHART__ANIM_SELECT_COLL:
    canvas->SelectCollisions();
    break;
#ifdef ANIM_OUTPUT_POVRAY
  case CALCHART__ANIM_POVRAY:
    s = wxFileSelector("Save POV Files", NULL, NULL, NULL, "*.*",
		       wxSAVE);
    if (s) {
      err = canvas->GeneratePOVFiles(s);
      if (err != NULL) {
	(void)wxMessageBox((char *)err, "Save Error"); // should be const
      }
    }
    break;
#endif
#ifdef ANIM_OUTPUT_RIB
  case CALCHART__ANIM_RIB_FRAME:
    allframes = FALSE;
  case CALCHART__ANIM_RIB:
    s = wxFileSelector("Save RIB File", NULL, NULL, NULL, "*.rib",
		       wxSAVE);
    if (s) {
      err = canvas->GenerateRIBFile(s, allframes);
      if (err != NULL) {
	(void)wxMessageBox((char *)err, "Save Error"); // should be const
      }
    }
    break;
#endif
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
  case CALCHART__ANIM_SELECT_COLL:
    msg = "Select colliding points";
    break;
#ifdef ANIM_OUTPUT_POVRAY
  case CALCHART__ANIM_POVRAY:
    msg = "Ouput files for rendering with POVRay";
    break;
#endif
#ifdef ANIM_OUTPUT_RIB
  case CALCHART__ANIM_RIB:
    msg = "Output RIB file for RenderMan rendering";
    break;
#endif
  case CALCHART__ANIM_CLOSE:
    msg = "Close window";
    break;
  }
  if (msg) SetStatusText(msg);
}

void AnimationFrame::UpdatePanel() {
  int num, curr;

  if (canvas->anim) {
    num = (int)canvas->anim->numsheets;
    curr = (int)canvas->anim->curr_sheetnum+1;
  } else {
    num = 1;
    curr = 1;
  }
  if (num > 1) {
    sheet_slider->Enable(TRUE);
    if (sheet_slider->GetMax() != num)
      sheet_slider->SetValue(1); // So Motif doesn't complain about value
      sheet_slider->SetRange(1, num);
    if (sheet_slider->GetValue() != curr)
      sheet_slider->SetValue(curr);
  } else {
    sheet_slider->Enable(FALSE);
  }

  if (canvas->anim) {
    num = ((int)canvas->anim->curr_sheet->numbeats)-1;
    curr = (int)canvas->anim->curr_beat;
  } else {
    num = 0;
    curr = 0;
  }
  if (num > 0) {
    beat_slider->Enable(TRUE);
    if (beat_slider->GetMax() != num)
      beat_slider->SetValue(0); // So Motif doesn't complain about value
      beat_slider->SetRange(0, num);
    if (beat_slider->GetValue() != curr)
      beat_slider->SetValue(curr);
  } else {
    beat_slider->Enable(FALSE);
  }
}

void AnimationCanvas::StartTimer() {
  if (!timer->Start(60000/GetTempo())) {
    ourframe->SetStatusText("Could not get timer!");
    timeron = FALSE;
  } else {
    timeron = TRUE;
  }
}

static void toolbar_anim_stop(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->StopTimer();
}

static void toolbar_anim_play(CoolToolBar *tb) {
  AnimationFrame* af = (AnimationFrame*)tb->ourframe;
  af->canvas->StartTimer();
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

static void collision_callback(wxChoice &ch, wxEvent&) {
  AnimationCanvas* ac =
    ((AnimationFrame*)(ch.GetParent()->GetParent()))->canvas;
  if (ac->anim) {
    ac->anim->EnableCollisions((CollisionWarning)ch.GetSelection());
    ac->anim->CheckCollisions();
    ac->Redraw();
  }
}

static void slider_anim_tempo(wxObject &obj, wxEvent &) {
  AnimationSlider *slider = (AnimationSlider*)&obj;
  slider->canvas->SetTempo(slider->GetValue());
}

static void slider_gotosheet(wxObject &obj, wxEvent &) {
  AnimationSlider *slider = (AnimationSlider*)&obj;
  slider->canvas->GotoSheet(slider->GetValue()-1);
}

static void slider_gotobeat(wxObject &obj, wxEvent &) {
  AnimationSlider *slider = (AnimationSlider*)&obj;
  slider->canvas->GotoBeat(slider->GetValue());
}

static void AnimErrorClose(wxButton& button, wxEvent&) {
  ((AnimErrorList*)button.GetParent()->GetParent())->Close();
}

static void AnimErrorClick(wxListBox& list, wxCommandEvent&) {
  AnimErrorList *err = (AnimErrorList*)list.GetParent()->GetParent();

  err->Update();
}

AnimErrorList::AnimErrorList(AnimateCompile *comp, CC_WinList *lst,
			     unsigned num, wxFrame *frame, char *title,
			     int x, int y, int width, int height)
: wxFrame(frame, title, x, y, width, height, CC_FRAME_OTHER), sheetnum(num) {
  unsigned i, j;

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
    if (comp->error_markers[i].pntgroup) {
      list->Append((char*)animate_err_msgs[i]);
      pointsels[j++].StealErrorMarker(&comp->error_markers[i]);
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
  if (node) {
    node->Remove();
    delete node;
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
      show->Select(j, pointsels[i].pntgroup[j]);
    }
    show->winlist->UpdateSelections(this);
  }
  show->winlist->GotoContLocation(sheetnum > show->GetNumSheets() ?
				  show->GetNumSheets()-1 : sheetnum,
				  pointsels[i].contnum,
				  pointsels[i].line, pointsels[i].col);
}
