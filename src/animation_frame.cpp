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
#include "basic_ui.h"
#include "anim_ui.h"
//#include "modes.h"
//#include "confgr.h"
//#include "cc_shapes.h"
//#include <boost/bind.hpp>

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

static const wxString collis_text[] =
{
	wxT("Ignore"), wxT("Show"), wxT("Beep")
};

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

