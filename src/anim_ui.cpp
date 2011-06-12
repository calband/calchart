/*
 * anim_ui.cpp
 * Animation user interface
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "anim_ui.h"
#include "modes.h"
#include "confgr.h"
#ifdef ANIM_OUTPUT_RIB
#include <ri.h>
#endif

ToolBarEntry anim_tb[] =
{
	{ wxITEM_NORMAL, NULL, wxT("Stop (space toggle)"), CALCHART__anim_stop },
	{ wxITEM_NORMAL, NULL, wxT("Play (space toggle)"), CALCHART__anim_play },
	{ wxITEM_NORMAL, NULL, wxT("Previous beat (left arrow)"), CALCHART__anim_prev_beat },
	{ wxITEM_NORMAL, NULL, wxT("Next beat (right arrow)"), CALCHART__anim_next_beat },
	{ wxITEM_NORMAL, NULL, wxT("Previous stuntsheet"), CALCHART__anim_prev_sheet },
	{ wxITEM_NORMAL, NULL, wxT("Next stuntsheet"), CALCHART__anim_next_sheet }
};

BEGIN_EVENT_TABLE(AnimationCanvas, wxPanel)
EVT_CHAR(AnimationCanvas::OnChar)
EVT_LEFT_DOWN(AnimationCanvas::OnLeftMouseEvent)
EVT_RIGHT_DOWN(AnimationCanvas::OnRightMouseEvent)
EVT_ERASE_BACKGROUND(AnimationCanvas::OnEraseBackground)
EVT_PAINT(AnimationCanvas::OnPaint)
EVT_SIZE(AnimationCanvas::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(AnimationFrame, wxFrame)
EVT_CHAR(AnimationFrame::OnChar)
EVT_MENU(CALCHART__ANIM_REANIMATE, AnimationFrame::OnCmdReanimate)
EVT_MENU(CALCHART__ANIM_SELECT_COLL, AnimationFrame::OnCmdSelectCollisions)
#ifdef ANIM_OUTPUT_POVRAY
EVT_MENU(CALCHART__ANIM_POVRAY, AnimationFrame::OnCmdPOV)
#endif
#ifdef ANIM_OUTPUT_RIB
EVT_MENU(CALCHART__ANIM_RIB_FRAME, AnimationFrame::OnCmdRIBFrame)
EVT_MENU(CALCHART__ANIM_RIB, AnimationFrame::OnCmdRIBAll)
#endif
EVT_MENU(wxID_CLOSE, AnimationFrame::OnCmdClose)
EVT_MENU(CALCHART__anim_stop, AnimationFrame::OnCmd_anim_stop)
EVT_MENU(CALCHART__anim_play, AnimationFrame::OnCmd_anim_play)
EVT_MENU(CALCHART__anim_prev_beat, AnimationFrame::OnCmd_anim_prev_beat)
EVT_MENU(CALCHART__anim_next_beat, AnimationFrame::OnCmd_anim_next_beat)
EVT_MENU(CALCHART__anim_prev_sheet, AnimationFrame::OnCmd_anim_prev_sheet)
EVT_MENU(CALCHART__anim_next_sheet, AnimationFrame::OnCmd_anim_next_sheet)
EVT_CHOICE(CALCHART__anim_collisions, AnimationFrame::OnCmd_anim_collisions)
EVT_COMMAND_SCROLL(CALCHART__anim_tempo, AnimationFrame::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotosheet, AnimationFrame::OnSlider_anim_gotosheet)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationFrame::OnSlider_anim_gotobeat)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(AnimErrorList, wxDialog)

BEGIN_EVENT_TABLE(AnimErrorList, wxDialog)
EVT_LISTBOX(CALCHART__anim_update, AnimErrorList::OnCmdUpdate)
END_EVENT_TABLE()

void AnimationTimer::Notify()
{
	if (!canvas->NextBeat()) Stop();
}


AnimationCanvas::AnimationCanvas(AnimationFrame *frame, CC_show *show)
: wxPanel(frame, wxID_ANY, wxDefaultPosition,
wxSize(COORD2INT(show->GetMode().Size().x)*DEFAULT_ANIM_SIZE,
COORD2INT(show->GetMode().Size().y)*DEFAULT_ANIM_SIZE)),
anim(NULL), mShow(show), ourframe(frame), tempo(120),
mUserScale(DEFAULT_ANIM_SIZE * (COORD2INT(1 << 16)/65536.0))
{
	timer = new AnimationTimer(this);
}


AnimationCanvas::~AnimationCanvas()
{
	if (timer) delete timer;
	if (anim) delete anim;
}


void AnimationCanvas::OnEraseBackground(wxEraseEvent& event)
{
}


void AnimationCanvas::OnPaint(wxPaintEvent& event)
{
	unsigned long x, y;
	unsigned i;
	wxPaintDC rdc(this);
	wxDC *dc = &rdc;

	dc->SetUserScale(mUserScale, mUserScale);
	dc->SetBackground(*CalChartBrushes[COLOR_FIELD]);
	dc->Clear();
	dc->SetPen(*CalChartPens[COLOR_FIELD_DETAIL]);
	mShow->GetMode().DrawAnim(dc);
	if (anim)
		for (i = 0; i < anim->numpts; i++)
	{
		if (anim->collisions[i])
		{
			dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_COLLISION]);
			dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_COLLISION]);
		}
		else if (mShow->IsSelected(i))
		{
			if (anim->curr_cmds[i])
			{
				switch (anim->curr_cmds[i]->Direction())
				{
					case ANIMDIR_SW:
					case ANIMDIR_W:
					case ANIMDIR_NW:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_BACK]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_BACK]);
						break;
					case ANIMDIR_SE:
					case ANIMDIR_E:
					case ANIMDIR_NE:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_FRONT]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_FRONT]);
						break;
					default:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_SIDE]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_SIDE]);
				}
			}
			else
			{
				dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_HILIT_FRONT]);
				dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_HILIT_FRONT]);
			}
		}
		else
		{
			if (anim->curr_cmds[i])
			{
				switch (anim->curr_cmds[i]->Direction())
				{
					case ANIMDIR_SW:
					case ANIMDIR_W:
					case ANIMDIR_NW:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_BACK]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_BACK]);
						break;
					case ANIMDIR_SE:
					case ANIMDIR_E:
					case ANIMDIR_NE:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_FRONT]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_FRONT]);
						break;
					default:
						dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_SIDE]);
						dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_SIDE]);
				}
			}
			else
			{
				dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_FRONT]);
				dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_FRONT]);
			}
		}
		x = anim->pts[i].pos.x+mShow->GetMode().Offset().x;
		y = anim->pts[i].pos.y+mShow->GetMode().Offset().y;
		dc->DrawRectangle(x - INT2COORD(1)/2, y - INT2COORD(1)/2,
			INT2COORD(1), INT2COORD(1));
	}
}


void AnimationCanvas::OnSize(wxSizeEvent& event)
{
	float x = mShow->GetMode().Size().x;
	float y = mShow->GetMode().Size().y;
	float newX = event.GetSize().x;
	float newY = event.GetSize().y;
	
	float showAspectRatio = x/y;
	float newSizeRatio = newX/newY;
	float newvalue = 1.0;
	// always choose x when the new aspect ratio is smaller than the show.
	// This will keep the whole field on in the canvas
	if (newSizeRatio < showAspectRatio)
	{
		newvalue = newX / (float)COORD2INT(x);
	}
	else
	{
		newvalue = newY / (float)COORD2INT(y);
	}
	mUserScale = (newvalue * (COORD2INT(1 << 16)/65536.0));
	Refresh();
	return wxPanel::OnSize(event);
}

void AnimationCanvas::OnLeftMouseEvent(wxMouseEvent& event)
{
	PrevBeat();
}


void AnimationCanvas::OnRightMouseEvent(wxMouseEvent& event)
{
	NextBeat();
}


void AnimationCanvas::SetTempo(unsigned t)
{
	tempo = t;
	if (timeron)
	{
		StartTimer();
	}
}


void AnimationCanvas::UpdateText()
{
	wxString tempbuf;

	if (anim)
	{
		tempbuf.Printf(wxT("Beat %u of %u  Sheet %d of %d \"%.32s\""),
			anim->curr_beat, anim->curr_sheet->numbeats,
			anim->curr_sheetnum+1, anim->numsheets,
			anim->curr_sheet->name.c_str());
		ourframe->SetStatusText(tempbuf, 1);
	}
	else
	{
		ourframe->SetStatusText(wxT("No animation available"), 1);
	}
}


void AnimationCanvas::RefreshCanvas()
{
	Redraw();
	UpdateText();
	ourframe->UpdatePanel();
}


void AnimationCanvas::Generate()
{
	StopTimer();
	ourframe->SetStatusText(wxT("Compiling..."));
	if (anim)
	{
		delete anim;
		anim = NULL;
		RefreshCanvas();
	}
	anim = new Animation(mShow, ourframe);
	if (anim && (anim->numsheets == 0))
	{
		delete anim;
		anim = NULL;
	}
	if (anim)
	{
		anim->EnableCollisions(((AnimationFrame*)ourframe)->CollisionType());
		anim->GotoSheet(mShow->GetCurrentSheetNum());
	}
	ourframe->UpdatePanel();
	RefreshCanvas();
	ourframe->SetStatusText(wxT("Ready"));
}


void AnimationCanvas::FreeAnim()
{
	if (anim)
	{
		delete anim;
		anim = NULL;
		RefreshCanvas();
	}
}


void AnimationCanvas::SelectCollisions()
{
	unsigned i;

	if (anim)
	{
		CC_show::SelectionList select;
		for (i = 0; i < anim->numpts; i++)
		{
			if (anim->collisions[i])
			{
				select.insert(i);
			}
		}
		mShow->SetSelection(select);
	}
}


#ifdef ANIM_OUTPUT_POVRAY
#define POV_SCALE 16
wxString AnimationCanvas::GeneratePOVFiles(const wxString& filebasename)
{
	unsigned framenum, pt;
	wxString filename;
	FILE *fp;
	float x, y;
	bool east = 1;
	wxPen *pen;

	if (anim)
	{
		GotoSheet(0);
		framenum = 1;
		do
		{
			filename.Printf(wxT("%s%05d.pov"), filebasename.c_str(), framenum);
			fp = CC_fopen(filename.fn_str(), "w");
			if (fp == NULL)
			{
				return wxT("Error opening file");
			}
			for (pt = 0; pt < anim->numpts; pt++)
			{
				x = COORD2FLOAT(anim->pts[pt].pos.x) * POV_SCALE;
				y = COORD2FLOAT(anim->pts[pt].pos.y) * POV_SCALE;
/* x is already flipped because of the different axises */
				if (east)
				{
					x = (-x);
				}
				else
				{
					y = (-y);
				}
				fprintf(fp, "cylinder { <%f, 0.0, %f>, <%f, %f, %f> %f\n",
					x, y,
					x, 4.0 * POV_SCALE, y,
					0.75 * POV_SCALE);
				if (anim->curr_cmds[pt])
				{
					switch (anim->curr_cmds[pt]->Direction())
					{
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
				}
				else
				{
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
	return wxT("");
}
#endif

#ifdef ANIM_OUTPUT_RIB

static const char *texturefile = "memorial.tif";
wxString AnimationCanvas::GenerateRIBFrame()
{
	unsigned pt;
	float x, y;
	bool east = 1;
	wxPen *pen;
	RtColor color;
	static RtFloat amb_intensity = 0.2, dist_intensity = 0.9;
	RtPoint field[4];

	if (!anim) return wxT("");

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
	for (pt = 0; pt < anim->numpts; pt++)
	{
		if (anim->curr_cmds[pt])
		{
			switch (anim->curr_cmds[pt]->Direction())
			{
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
		}
		else
		{
			pen = CalChartPens[COLOR_POINT_ANIM_FRONT];
		}
		color[0] = pen->GetColour().Red() / 255.0;
		color[1] = pen->GetColour().Green() / 255.0;
		color[2] = pen->GetColour().Blue() / 255.0;
		RiColor(color);
		x = COORD2FLOAT(anim->pts[pt].pos.x) * POV_SCALE;
		y = COORD2FLOAT(anim->pts[pt].pos.y) * POV_SCALE;
/* x is already flipped because of the different axises */
		if (east)
		{
			x = (-x);
		}
		else
		{
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
	return wxT("");
}


wxString AnimationCanvas::GenerateRIBFile(const wxString& filename,
bool allframes)
{
	unsigned framenum;
	wxString err;

	if (anim)
	{
		if (allframes)
		{
			GotoSheet(0);
		}
		framenum = 1;
		RiBegin((char *)filename);
		RiProjection(RI_PERSPECTIVE, RI_NULL);
		RiRotate(45.0, 1.0, 0.0, 0.0);
		RiTranslate(0.0, 1471.0, 1471.0);
		if (allframes) do
		{
			RiFrameBegin(framenum);
			err = GenerateRIBFrame();
			if (err) return err;
			RiFrameEnd();
			framenum++;
		} while (NextBeat());
		else
		{
			err = GenerateRIBFrame();
			if (err) return err;
		}
		RiEnd();
	}
	return wxT("");
}
#endif

static const wxString collis_text[] =
{
	wxT("Ignore"), wxT("Show"), wxT("Beep")
};

AnimationView::AnimationView() {}
AnimationView::~AnimationView() {}

void AnimationView::OnDraw(wxDC *dc) {}
void AnimationView::OnUpdate(wxView *sender, wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_modified)))
	{
		static_cast<AnimationFrame*>(GetFrame())->canvas->FreeAnim();
	}
}

AnimationFrame::AnimationFrame(wxFrame *frame, CC_show *show)
: wxFrame(frame, wxID_ANY, wxT("Animation"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, wxT("anim"))
{
	mView = new AnimationView;
	mView->SetDocument(show);
	mView->SetFrame(this);
// Give it an icon
	SetBandIcon(this);

	CreateStatusBar(2);

// Make a menubar
	wxMenu *anim_menu = new wxMenu;
	anim_menu->Append(CALCHART__ANIM_REANIMATE, wxT("&Reanimate Show"), wxT("Regenerate animation"));
	anim_menu->Append(CALCHART__ANIM_SELECT_COLL, wxT("&Select Collisions"), wxT("Select colliding points"));
#ifdef ANIM_OUTPUT_POVRAY
	anim_menu->Append(CALCHART__ANIM_POVRAY, wxT("Output &POVRay Stubs"), wxT("Output files for rendering with POVRay"));
#endif
#ifdef ANIM_OUTPUT_RIB
	anim_menu->Append(CALCHART__ANIM_RIB_FRAME, wxT("Output RenderMan RIB Frame"), wxT("Output RIB frame for RenderMan rendering"));
	anim_menu->Append(CALCHART__ANIM_RIB, wxT("Output Render&Man RIB"), wxT("Output RIB file for RenderMan rendering"));
#endif
	anim_menu->Append(wxID_CLOSE, wxT("&Close Window\tCTRL-W"), wxT("Close window"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(anim_menu, wxT("&Animate"));
	SetMenuBar(menu_bar);

// Add a toolbar
	CreateCoolToolBar(anim_tb, sizeof(anim_tb)/sizeof(ToolBarEntry), this);

// Add the field canvas
	canvas = new AnimationCanvas(this, show);
	canvas->UpdateText();

// Add the controls
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(canvas, 1, wxEXPAND | wxBORDER, 5);

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Collisions")),
		wxSizerFlags());
	collis = new wxChoice(this, CALCHART__anim_collisions,
		wxDefaultPosition, wxDefaultSize,
		sizeof(collis_text)/sizeof(const wxString), collis_text);
	collis->SetSelection(1);
	sizer1->Add(collis, wxSizerFlags().Expand().Border(5));

	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Tempo")),
		wxSizerFlags());
	wxSlider *sldr =
		new wxSlider(this, CALCHART__anim_tempo,
		canvas->GetTempo(), 10, 300, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer1->Add(sldr, wxSizerFlags().Expand().Border(5));

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
// Sheet slider (will get set later with UpdatePanel())
	sizer2->Add(new wxStaticText(this, wxID_ANY, wxT("&Sheet")),
		wxSizerFlags());
	sheet_slider = new wxSlider(this, CALCHART__anim_gotosheet,
		1, 1, 2, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer2->Add(sheet_slider, wxSizerFlags().Expand().Border(5));

// Beat slider (will get set later with UpdatePanel())
	sizer2->Add(new wxStaticText(this, wxID_ANY, wxT("&Beat")),
		wxSizerFlags());
	beat_slider = new wxSlider(this, CALCHART__anim_gotobeat,
		0, 0, 1, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer2->Add(beat_slider, wxSizerFlags().Expand().Border(5));

//create a sizer with no border and centered horizontally
	topsizer->Add(sizer1, wxSizerFlags(0).Left());
	topsizer->Add(sizer2, wxSizerFlags(0).Left());

	SetSizer(topsizer);							  // use the sizer for layout

	topsizer->SetSizeHints(this);				  // set size hints to honour minimum size

	UpdatePanel();
	Fit();
	Show(true);
}


AnimationFrame::~AnimationFrame()
{
	delete mView;
}


void AnimationFrame::OnCmdReanimate(wxCommandEvent& event)
{
	canvas->Generate();
}


void AnimationFrame::OnCmdSelectCollisions(wxCommandEvent& event)
{
	canvas->SelectCollisions();
}


#ifdef ANIM_OUTPUT_POVRAY
void AnimationFrame::OnCmdPOV(wxCommandEvent& event)
{
	wxString s;
	wxString err;
	s = wxFileSelector(wxT("Save POV Files"), NULL, NULL, NULL, wxT("*.*"),
		wxSAVE);
	if (!s.empty())
	{
		err = canvas->GeneratePOVFiles(s);
		if (!err.empty())
		{
			(void)wxMessageBox(err, wxT("Save Error"));
		}
	}
}
#endif

#ifdef ANIM_OUTPUT_RIB
void AnimationFrame::OnCmdRIBFrame(wxCommandEvent& event)
{
	OnCmdRIB(event, false);
}


void AnimationFrame::OnCmdRIBAll(wxCommandEvent& event)
{
	OnCmdRIB(event, true);
}


void AnimationFrame::OnCmdRIB(wxCommandEvent& event, bool allframes)
{
	wxString s;
	wxString err;
	s = wxFileSelector(wxT("Save RIB File"), NULL, NULL, NULL, wxT("*.rib"),
		wxSAVE);
	if (!s.empty())
	{
		err = canvas->GenerateRIBFile(s, allframes);
		if (!err.empty())
		{
			(void)wxMessageBox(err, wxT("Save Error"));
		}
	}
}
#endif

void AnimationFrame::OnCmdClose(wxCommandEvent& event)
{
	Close();
}


void AnimationFrame::OnCmd_anim_stop(wxCommandEvent& event)
{
	canvas->StopTimer();
}


void AnimationFrame::OnCmd_anim_play(wxCommandEvent& event)
{
	canvas->StartTimer();
}


void AnimationFrame::OnCmd_anim_prev_beat(wxCommandEvent& event)
{
	canvas->PrevBeat();
}


void AnimationFrame::OnCmd_anim_next_beat(wxCommandEvent& event)
{
	canvas->NextBeat();
}


void AnimationFrame::OnCmd_anim_prev_sheet(wxCommandEvent& event)
{
	canvas->PrevSheet();
}


void AnimationFrame::OnCmd_anim_next_sheet(wxCommandEvent& event)
{
	canvas->NextSheet();
}


void AnimationFrame::OnCmd_anim_collisions(wxCommandEvent& event)
{
	if (canvas->anim)
	{
		canvas->anim->EnableCollisions((CollisionWarning)event.GetSelection());
		canvas->anim->CheckCollisions();
		canvas->Redraw();
	}
}


void AnimationFrame::OnSlider_anim_tempo(wxScrollEvent& event)
{
	canvas->SetTempo(event.GetPosition());
}


void AnimationFrame::OnSlider_anim_gotosheet(wxScrollEvent& event)
{
	canvas->GotoSheet(event.GetPosition()-1);
}


void AnimationFrame::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
	canvas->GotoBeat(event.GetPosition());
}


void AnimationFrame::UpdatePanel()
{
	int num, curr;

	if (canvas->anim)
	{
		num = (int)canvas->anim->numsheets;
		curr = (int)canvas->anim->curr_sheetnum+1;
	}
	else
	{
		num = 1;
		curr = 1;
	}
	if (num > 1)
	{
		sheet_slider->Enable(true);
		if (sheet_slider->GetMax() != num)
			sheet_slider->SetValue(1);			  // So Motif doesn't complain about value
		sheet_slider->SetRange(1, num);
		if (sheet_slider->GetValue() != curr)
			sheet_slider->SetValue(curr);
	}
	else
	{
		sheet_slider->Enable(false);
	}

	if (canvas->anim)
	{
		num = ((int)canvas->anim->curr_sheet->numbeats)-1;
		curr = (int)canvas->anim->curr_beat;
	}
	else
	{
		num = 0;
		curr = 0;
	}
	if (num > 0)
	{
		beat_slider->Enable(true);
		if (beat_slider->GetMax() != num)
			beat_slider->SetValue(0);			  // So Motif doesn't complain about value
		beat_slider->SetRange(0, num);
		if (beat_slider->GetValue() != curr)
			beat_slider->SetValue(curr);
	}
	else
	{
		beat_slider->Enable(false);
	}
}


void AnimationFrame::OnChar(wxKeyEvent& event)
{
	canvas->OnChar(event);
}


void AnimationCanvas::StartTimer()
{
	if (!timer->Start(60000/GetTempo()))
	{
		ourframe->SetStatusText(wxT("Could not get timer!"));
		timeron = false;
	}
	else
	{
		timeron = true;
	}
}


void AnimationCanvas::OnChar(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_LEFT)
		PrevBeat();
	else if (event.GetKeyCode() == WXK_RIGHT)
		NextBeat();
	else if (event.GetKeyCode() == WXK_SPACE)
	{
		if (timeron)
			StopTimer();
		else
			StartTimer();
	}
	else
		event.Skip();
}


AnimErrorListView::AnimErrorListView() {}
AnimErrorListView::~AnimErrorListView() {}

void AnimErrorListView::OnDraw(wxDC *dc) {}
void AnimErrorListView::OnUpdate(wxView *sender, wxObject *hint)
{
	if (hint && hint->IsKindOf(CLASSINFO(CC_show_modified)))
	{
		static_cast<AnimErrorList*>(GetFrame())->Close();
	}
}

AnimErrorList::AnimErrorList()
{
	Init();
}


AnimErrorList::AnimErrorList(AnimateCompile *comp, unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	Init();
	
	Create(comp, num, parent, id, caption, pos, size, style);
}


bool AnimErrorList::Create(AnimateCompile *comp, unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;

	show = comp->show;
	for (size_t i = 0; i < NUM_ANIMERR; ++i)
		mErrorMarkers[i] = comp->error_markers[i];

	// give this a view so it can pick up document changes
	mView = new AnimErrorListView;
	mView->SetDocument(show);
	mView->SetFrame(this);

	CreateControls();

// This fits the dalog to the minimum size dictated by the sizers
	GetSizer()->Fit(this);
// This ensures that the dialog cannot be smaller than the minimum size
	GetSizer()->SetSizeHints(this);

	Center();

	return true;
}


void AnimErrorList::Init()
{
}


void AnimErrorList::CreateControls()
{
// create a sizer for laying things out top down:
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(topsizer);

	wxButton *closeBut = new wxButton(this, wxID_OK, wxT("Close"));
	topsizer->Add(closeBut);

	wxListBox* lst = new wxListBox(this, CALCHART__anim_update, wxDefaultPosition, wxSize(200,200), 0, NULL, wxLB_SINGLE);

	topsizer->Add(lst, wxSizerFlags().Expand().Border(5) );
}

bool AnimErrorList::TransferDataToWindow()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);

	for (unsigned i = 0, j = 0; i < NUM_ANIMERR; i++)
	{
		if (!mErrorMarkers[i].pntgroup.empty())
		{
			lst->Append(animate_err_msgs[i]);
			pointsels[j++] = mErrorMarkers[i];
		}
	}
	return true;
}

AnimErrorList::~AnimErrorList()
{
	delete mView;
}


void AnimErrorList::OnCmdUpdate(wxCommandEvent& event)
{
	Update(event.IsSelection() ? event.GetSelection() : -1);
}


void AnimErrorList::Unselect()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);
	int i = lst->GetSelection();

	if (i >= 0)
	{
		lst->Deselect(i);
	}
}


void AnimErrorList::Update()
{
	wxListBox* lst = (wxListBox*)FindWindow(CALCHART__anim_update);
	Update(lst->GetSelection());
}


void AnimErrorList::Update(int i)
{
	if (i >= 0)
	{
		CC_show::SelectionList select;
		for (unsigned j = 0; j < show->GetNumPoints(); j++)
		{
			if (pointsels[i].pntgroup.count(j))
			{
				select.insert(j);
			}
		}
		show->SetSelection(select);
	}
	show->SetCurrentSheet(sheetnum > show->GetNumSheets() ? show->GetNumSheets()-1 : sheetnum);
	show->AllViewGoToCont(pointsels[i].contnum, pointsels[i].line, pointsels[i].col);
}
