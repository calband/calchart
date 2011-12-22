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

#include "basic_ui.h"
#include "anim_ui.h"
#include "modes.h"
#include "confgr.h"
#include "cc_shapes.h"
#ifdef ANIM_OUTPUT_RIB
#include <ri.h>
#endif
#include <boost/bind.hpp>

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
EVT_LEFT_DOWN(AnimationCanvas::OnLeftDownMouseEvent)
EVT_LEFT_UP(AnimationCanvas::OnLeftUpMouseEvent)
EVT_RIGHT_DOWN(AnimationCanvas::OnRightDownMouseEvent)
EVT_RIGHT_UP(AnimationCanvas::OnRightUpMouseEvent)
EVT_MOTION(AnimationCanvas::OnMouseMove)
EVT_PAINT(AnimationCanvas::OnPaint)
EVT_SIZE(AnimationCanvas::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(AnimationFrame, wxFrame)
EVT_CHAR(AnimationFrame::OnChar)
EVT_MENU(CALCHART__ANIM_REANIMATE, AnimationFrame::OnCmdReanimate)
EVT_MENU(CALCHART__ANIM_SELECT_COLL, AnimationFrame::OnCmdSelectCollisions)
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

wxIMPLEMENT_DYNAMIC_CLASS(AnimErrorList, wxDialog)

BEGIN_EVENT_TABLE(AnimErrorList, wxDialog)
EVT_LISTBOX(CALCHART__anim_update, AnimErrorList::OnCmdUpdate)
END_EVENT_TABLE()

void AnimationTimer::Notify()
{
	if (!canvas->NextBeat()) Stop();
}


AnimationCanvas::AnimationCanvas(AnimationFrame *frame, CC_show *show)
: wxPanel(frame, wxID_ANY, wxDefaultPosition,
wxSize(Coord2Int(show->GetMode().Size().x)*DEFAULT_ANIM_SIZE,
Coord2Int(show->GetMode().Size().y)*DEFAULT_ANIM_SIZE)),
anim(NULL), mShow(show), ourframe(frame), tempo(120),
mUserScale(DEFAULT_ANIM_SIZE * (Coord2Int(1 << 16)/65536.0))
{
	timer = new AnimationTimer(this);
}


AnimationCanvas::~AnimationCanvas()
{
	if (timer) delete timer;
	if (anim) delete anim;
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
	if (mMouseDown)
	{
		dc->SetBrush(*wxTRANSPARENT_BRUSH);
		dc->SetPen(*CalChartPens[COLOR_SHAPES]);
		dc->DrawRectangle(mMouseXStart, mMouseYStart,
						  mMouseXEnd - mMouseXStart, mMouseYEnd - mMouseYStart);
	}
	dc->SetPen(*CalChartPens[COLOR_FIELD_DETAIL]);
	mShow->GetMode().DrawAnim(*dc);
	if (anim)
		for (i = 0; i < mShow->GetNumPoints(); i++)
	{
		if (anim->IsCollision(i))
		{
			dc->SetPen(*CalChartPens[COLOR_POINT_ANIM_COLLISION]);
			dc->SetBrush(*CalChartBrushes[COLOR_POINT_ANIM_COLLISION]);
		}
		else if (mShow->IsSelected(i))
		{
			switch (anim->Direction(i))
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
			switch (anim->Direction(i))
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
		CC_coord position = anim->Position(i);
		x = position.x+mShow->GetMode().Offset().x;
		y = position.y+mShow->GetMode().Offset().y;
		dc->DrawRectangle(x - Int2Coord(1)/2, y - Int2Coord(1)/2,
			Int2Coord(1), Int2Coord(1));
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
		newvalue = newX / (float)Coord2Int(x);
	}
	else
	{
		newvalue = newY / (float)Coord2Int(y);
	}
	mUserScale = (newvalue * (Coord2Int(1 << 16)/65536.0));
	Refresh();
}

void AnimationCanvas::OnLeftDownMouseEvent(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );

	mMouseXEnd = mMouseXStart = x;
	mMouseYEnd = mMouseYStart = y;
	mMouseDown = true;
}


void AnimationCanvas::OnLeftUpMouseEvent(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	mMouseXEnd = x;
	mMouseYEnd = y;
	mMouseDown = false;

	// if mouse lifted very close to where clicked, then it is a previous beat move
	if ((abs(mMouseXEnd - mMouseXStart) < Int2Coord(1)/2) && (abs(mMouseYEnd - mMouseYStart) < Int2Coord(1)/2))
	{
		PrevBeat();
	}
	else
	{
		// otherwise, Select points within rectangle
		Coord x_off = mShow->GetMode().Offset().x;
		Coord y_off = mShow->GetMode().Offset().y;
		CC_lasso lasso(CC_coord(mMouseXStart-x_off, mMouseYStart-y_off));
		lasso.Append(CC_coord(mMouseXStart-x_off, mMouseYEnd-y_off));
		lasso.Append(CC_coord(mMouseXEnd-x_off, mMouseYEnd-y_off));
		lasso.Append(CC_coord(mMouseXEnd-x_off, mMouseYStart-y_off));
		lasso.End();
		CC_show::SelectionList pointlist;
		for (unsigned i = 0; i < mShow->GetNumPoints(); ++i)
		{
			CC_coord position = anim->Position(i);
			x = position.x+x_off;
			y = position.y+y_off;
			if (lasso.Inside(position))
			{
				pointlist.insert(i);
			}
		}
		mShow->SetSelection(pointlist);
	}
	Refresh();
}


void
AnimationCanvas::OnMouseMove(wxMouseEvent& event)
{
	wxClientDC dc(this);
	dc.SetUserScale(mUserScale, mUserScale);
	long x,y;
	event.GetPosition(&x, &y);
	x = dc.DeviceToLogicalX( x );
	y = dc.DeviceToLogicalY( y );
	mMouseXEnd = x;
	mMouseYEnd = y;
	if (event.Dragging())
	{
		Refresh();
	}
}


void AnimationCanvas::OnRightDownMouseEvent(wxMouseEvent& event)
{
}


void AnimationCanvas::OnRightUpMouseEvent(wxMouseEvent& event)
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


int AnimationCanvas::GetNumberSheets() const
{
	return (anim) ? anim->GetNumberSheets() : 0;
}

int AnimationCanvas::GetCurrentSheet() const
{
	return (anim) ? anim->GetCurrentSheet() : 0;
}

int AnimationCanvas::GetNumberBeats() const
{
	return (anim) ? anim->GetNumberBeats() : 0;
}

int AnimationCanvas::GetCurrentBeat() const
{
	return (anim) ? anim->GetCurrentBeat() : 0;
}


void AnimationCanvas::UpdateText()
{
	wxString tempbuf;

	if (anim)
	{
		tempbuf.Printf(wxT("Beat %u of %u  Sheet %d of %d \"%.32s\""),
			anim->GetCurrentBeat(), anim->GetNumberBeats(),
			anim->GetCurrentSheet()+1, anim->GetNumberSheets(),
			anim->GetCurrentSheetName().c_str());
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


void AnimationCanvas::OnNotifyStatus(const wxString& status)
{
	ourframe->SetStatusText(status);
}

bool AnimationCanvas::OnNotifyErrorList(const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message)
{
	AnimErrorList* errorList = new AnimErrorList(mShow, error_markers, sheetnum,
				ourframe, wxID_ANY, message);
	errorList->Show();
	return (wxMessageBox(wxT("Ignore errors?"), wxT("Animate"), wxYES_NO) != wxYES);
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

	// flush out the show
	mShow->FlushAllTextWindows();		  // get all changes in text windows

	NotifyStatus notifyStatus = boost::bind(&AnimationCanvas::OnNotifyStatus, this, _1);
	NotifyErrorList notifyErrorList = boost::bind(&AnimationCanvas::OnNotifyErrorList, this, _1, _2, _3);
	anim = new Animation(mShow, notifyStatus, notifyErrorList);
	if (anim && (anim->GetNumberSheets() == 0))
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


void AnimationCanvas::EnableCollisions(CollisionWarning col)
{
	if (anim)
	{
		anim->EnableCollisions(col);
	}
}

void AnimationCanvas::CheckCollisions()
{
	if (anim)
	{
		anim->CheckCollisions();
	}
}

void AnimationCanvas::SelectCollisions()
{
	unsigned i;

	if (anim)
	{
		CC_show::SelectionList select;
		for (i = 0; i < mShow->GetNumPoints(); i++)
		{
			if (anim->IsCollision(i))
			{
				select.insert(i);
			}
		}
		mShow->SetSelection(select);
	}
}


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
	topsizer->Add(canvas, wxSizerFlags(1).Expand().Border(5));

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

	// make animation screen large by default.
	Maximize(true);
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
	canvas->EnableCollisions((CollisionWarning)event.GetSelection());
	canvas->CheckCollisions();
	canvas->Redraw();
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

	num = canvas->GetNumberSheets();
	curr = canvas->GetCurrentSheet()+1;

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

	num = canvas->GetNumberBeats()-1;
	curr = canvas->GetCurrentBeat();

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


AnimErrorList::AnimErrorList(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	Init();
	
	Create(show, error_markers, num, parent, id, caption, pos, size, style);
}


bool AnimErrorList::Create(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size,
		long style)
{
	if (!wxDialog::Create(parent, id, caption, pos, size, style))
		return false;

	mShow = show;
	for (size_t i = 0; i < NUM_ANIMERR; ++i)
		mErrorMarkers[i] = error_markers[i];

	// give this a view so it can pick up document changes
	mView = new AnimErrorListView;
	mView->SetDocument(mShow);
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
		for (unsigned j = 0; j < mShow->GetNumPoints(); j++)
		{
			if (pointsels[i].pntgroup.count(j))
			{
				select.insert(j);
			}
		}
		mShow->SetSelection(select);
	}
	mShow->SetCurrentSheet(sheetnum > mShow->GetNumSheets() ? mShow->GetNumSheets()-1 : sheetnum);
	mShow->AllViewGoToCont(pointsels[i].contnum, pointsels[i].line, pointsels[i].col);
}
