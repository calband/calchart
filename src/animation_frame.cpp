/*
 * animation_frame.cpp
 * Implimentation for AnimationFrame
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

#include "animation_frame.h"
#include "anim_ui.h"
#include "animation_canvas.h"
#include "basic_ui.h"

#include <wx/timer.h>


enum
{
	CALCHART__anim_reanimate = 1000,
	CALCHART__anim_select_coll,
	CALCHART__anim_stop,
	CALCHART__anim_play,
	CALCHART__anim_prev_beat,
	CALCHART__anim_next_beat,
	CALCHART__anim_next_beat_timer,
	CALCHART__anim_prev_sheet,
	CALCHART__anim_next_sheet,
	CALCHART__anim_collisions,
	CALCHART__anim_tempo,
	CALCHART__anim_gotosheet,
	CALCHART__anim_gotobeat,
};


ToolBarEntry anim_tb[] =
{
	{ wxITEM_NORMAL, NULL, wxT("Stop (space toggle)"), CALCHART__anim_stop },
	{ wxITEM_NORMAL, NULL, wxT("Play (space toggle)"), CALCHART__anim_play },
	{ wxITEM_NORMAL, NULL, wxT("Previous beat (left arrow)"), CALCHART__anim_prev_beat },
	{ wxITEM_NORMAL, NULL, wxT("Next beat (right arrow)"), CALCHART__anim_next_beat },
	{ wxITEM_NORMAL, NULL, wxT("Previous stuntsheet"), CALCHART__anim_prev_sheet },
	{ wxITEM_NORMAL, NULL, wxT("Next stuntsheet"), CALCHART__anim_next_sheet }
};


BEGIN_EVENT_TABLE(AnimationFrame, wxFrame)
EVT_MENU(CALCHART__anim_reanimate, AnimationFrame::OnCmdReanimate)
EVT_MENU(CALCHART__anim_select_coll, AnimationFrame::OnCmdSelectCollisions)
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
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationFrame::OnCmd_anim_next_beat_timer)
END_EVENT_TABLE()


static const wxString collis_text[] =
{
	wxT("Ignore"), wxT("Show"), wxT("Beep")
};


AnimationFrame::AnimationFrame(/*wxView* view, */wxWindow *parent, wxDocument* doc, const wxPoint& pos, const wxSize& size) :
wxFrame(/*doc, view, */parent, wxID_ANY, wxT("Animation"), pos, size),
//mView(static_cast<AnimationView*>(view)),
mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer)),
mTempo(120),
mTimerOn(false)
{
// Give it an icon
	// give this a view so it can pick up document changes
	mView = new AnimationView();
	mView->SetDocument(doc);
	mView->SetFrame(this);
	SetBandIcon(this);

	// this frame has 2 status bars at the bottom
	CreateStatusBar(2);

// Make a menubar
	wxMenu *anim_menu = new wxMenu;
	anim_menu->Append(CALCHART__anim_reanimate, wxT("&Reanimate Show"), wxT("Regenerate animation"));
	anim_menu->Append(CALCHART__anim_select_coll, wxT("&Select Collisions"), wxT("Select colliding points"));
	anim_menu->Append(wxID_CLOSE, wxT("&Close Window\tCTRL-W"), wxT("Close window"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(anim_menu, wxT("&Animate"));
	SetMenuBar(menu_bar);

// Add a toolbar
	CreateCoolToolBar(anim_tb, sizeof(anim_tb)/sizeof(ToolBarEntry), this);

// Add the field canvas
	mCanvas = new AnimationCanvas(this, mView);

// Add the controls
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(mCanvas, wxSizerFlags(1).Expand().Border(5));

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Collisions")), wxSizerFlags());
	wxChoice *collis = new wxChoice(this, CALCHART__anim_collisions, wxDefaultPosition, wxDefaultSize, sizeof(collis_text)/sizeof(const wxString), collis_text);
	collis->SetSelection(1);
	sizer1->Add(collis, wxSizerFlags().Expand().Border(5));

	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Tempo")), wxSizerFlags());
	wxSlider *sldr = new wxSlider(this, CALCHART__anim_tempo, GetTempo(), 10, 300, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer1->Add(sldr, wxSizerFlags().Expand().Border(5));

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
// Sheet slider (will get set later with UpdatePanel())
	sizer2->Add(new wxStaticText(this, wxID_ANY, wxT("&Sheet")), wxSizerFlags());
	mSheetSlider = new wxSlider(this, CALCHART__anim_gotosheet, 1, 1, 2, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer2->Add(mSheetSlider, wxSizerFlags().Expand().Border(5));

// Beat slider (will get set later with UpdatePanel())
	sizer2->Add(new wxStaticText(this, wxID_ANY, wxT("&Beat")), wxSizerFlags());
	mBeatSlider = new wxSlider(this, CALCHART__anim_gotobeat, 0, 0, 1, wxDefaultPosition,
                    wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer2->Add(mBeatSlider, wxSizerFlags().Expand().Border(5));

//create a sizer with no border and centered horizontally
	topsizer->Add(sizer1, wxSizerFlags(0).Left());
	topsizer->Add(sizer2, wxSizerFlags(0).Left());

	SetSizer(topsizer);							  // use the sizer for layout

	topsizer->SetSizeHints(this);				  // set size hints to honour minimum size

	mView->Generate();

	UpdatePanel();
	Fit();
	Show(true);

	// make animation screen large by default.
	Maximize(true);
}


AnimationFrame::~AnimationFrame()
{
	mCanvas->SetView(NULL);
	mTimer->Stop();
	delete mView;
}


void
AnimationFrame::SetView(wxView *view)
{
//	wxDocMDIChildFrame::SetView(view);
	mCanvas->SetView(static_cast<AnimationView*>(view));
	mView = static_cast<AnimationView*>(view);
}


void
AnimationFrame::OnCmdReanimate(wxCommandEvent& event)
{
	StopTimer();
	if (mView)
	{
		mView->Generate();
	}
}


void
AnimationFrame::OnCmdSelectCollisions(wxCommandEvent& event)
{
	if (mView)
	{
		mView->SelectCollisions();
	}
}


void
AnimationFrame::OnCmdClose(wxCommandEvent& event)
{
	Close();
}


void
AnimationFrame::OnCmd_anim_stop(wxCommandEvent& event)
{
	StopTimer();
}


void
AnimationFrame::OnCmd_anim_play(wxCommandEvent& event)
{
	StartTimer();
}


void
AnimationFrame::OnCmd_anim_prev_beat(wxCommandEvent& event)
{
	if (mView)
	{
		mView->PrevBeat();
	}
}


void
AnimationFrame::OnCmd_anim_next_beat(wxCommandEvent& event)
{
	if (mView)
	{
		mView->NextBeat();
	}
}


void
AnimationFrame::OnCmd_anim_next_beat_timer(wxTimerEvent& event)
{
	// next_beat could come from the timer.  If so, stop the timer.
	if (mView && !mView->NextBeat())
	{
		StopTimer();
	}
}


void
AnimationFrame::OnCmd_anim_prev_sheet(wxCommandEvent& event)
{
	if (mView)
	{
		mView->PrevSheet();
	}
}


void
AnimationFrame::OnCmd_anim_next_sheet(wxCommandEvent& event)
{
	if (mView)
	{
		mView->NextSheet();
	}
}


void
AnimationFrame::OnCmd_anim_collisions(wxCommandEvent& event)
{
	if (mView)
	{
		mView->EnableCollisions(static_cast<CollisionWarning>(event.GetSelection()));
		mView->CheckCollisions();
	}
	Refresh();
}


void
AnimationFrame::OnSlider_anim_tempo(wxScrollEvent& event)
{
	SetTempo(event.GetPosition());
	StartTimer();
}


void
AnimationFrame::OnSlider_anim_gotosheet(wxScrollEvent& event)
{
	if (mView)
	{
		mView->GotoSheet(event.GetPosition()-1);
	}
}


void
AnimationFrame::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
	if (mView)
	{
		mView->GotoBeat(event.GetPosition());
	}
}


void
AnimationFrame::ToggleTimer()
{
	if (mTimerOn)
		StopTimer();
	else
		StartTimer();
}


void
AnimationFrame::UpdatePanel()
{
	int num = (mView) ? mView->GetNumberSheets() : 1;
	int curr = (mView) ? mView->GetCurrentSheet()+1 : 1;

	if (num > 1)
	{
		mSheetSlider->Enable(true);
		if (mSheetSlider->GetMax() != num)
			mSheetSlider->SetValue(1);			  // So Motif doesn't complain about value
		mSheetSlider->SetRange(1, num);
		if (mSheetSlider->GetValue() != curr)
			mSheetSlider->SetValue(curr);
	}
	else
	{
		mSheetSlider->Enable(false);
	}

	num = (mView) ? mView->GetNumberBeats()-1 : 1;
	curr = (mView) ? mView->GetCurrentBeat() : 1;

	if (num > 0)
	{
		mBeatSlider->Enable(true);
		if (mBeatSlider->GetMax() != num)
			mBeatSlider->SetValue(0);			  // So Motif doesn't complain about value
		mBeatSlider->SetRange(0, num);
		if (mBeatSlider->GetValue() != curr)
			mBeatSlider->SetValue(curr);
	}
	else
	{
		mBeatSlider->Enable(false);
	}
	if (mView)
	{
		SetStatusText(mView->GetStatusText(), 1);
	}
}


CollisionWarning
AnimationFrame::CollisionType()
{
	wxChoice* choiceCtrl = static_cast<wxChoice*>(FindWindow(CALCHART__anim_collisions));

	return static_cast<CollisionWarning>(choiceCtrl->GetSelection());
}


void
AnimationFrame::StartTimer()
{
	if (!mTimer->Start(60000/GetTempo()))
	{
		SetStatusText(wxT("Could not set tempo!"));
		mTimerOn = false;
	}
	else
	{
		mTimerOn = true;
	}
}


void
AnimationFrame::StopTimer()
{
	mTimer->Stop();
	mTimerOn = false;
}


unsigned
AnimationFrame::GetTempo() const
{
	return mTempo;
}


void
AnimationFrame::SetTempo(unsigned tempo)
{
	mTempo = tempo;
}

