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
#include "animation_view.h"
#include "animation_canvas.h"
#include "basic_ui.h"
#include "cc_omniview_canvas.h"
#include "toolbar.h"
#include "ui_enums.h"
#include "basic_ui.h"

#include <wx/timer.h>


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
EVT_MENU(CALCHART__FollowMarcher, AnimationFrame::OnCmd_FollowMarcher)
EVT_MENU(CALCHART__SaveCameraAngle, AnimationFrame::OnCmd_SaveCameraAngle)
EVT_MENU(CALCHART__GoToCameraAngle, AnimationFrame::OnCmd_GoToCameraAngle)
EVT_MENU(CALCHART__ShowKeyboardControls, AnimationFrame::OnCmd_ShowKeyboardControls)
EVT_MENU(CALCHART__ToggleCrowd, AnimationFrame::OnCmd_ToggleCrowd)
EVT_MENU(CALCHART__ToggleMarching, AnimationFrame::OnCmd_ToggleMarching)
EVT_MENU(CALCHART__ToggleShowOnlySelected, AnimationFrame::OnCmd_ToggleShowOnlySelected)
EVT_CHOICE(CALCHART__anim_collisions, AnimationFrame::OnCmd_anim_collisions)
EVT_CHOICE(CALCHART__anim_errors, AnimationFrame::OnCmd_anim_errors)
EVT_COMMAND_SCROLL(CALCHART__anim_tempo, AnimationFrame::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotosheet, AnimationFrame::OnSlider_anim_gotosheet)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationFrame::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationFrame::OnCmd_anim_next_beat_timer)
END_EVENT_TABLE()


static const wxString collis_text[] =
{
	wxT("Ignore"), wxT("Show"), wxT("Beep")
};

AnimationFrame::AnimationFrame(wxWindow *parent, wxDocument* doc, bool OmniViewer, const wxPoint& pos, const wxSize& size) :
wxFrame(parent, wxID_ANY, wxT("Animation"), pos, size),
mCanvas(NULL),
mOmniViewCanvas(NULL),
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
	
	if (OmniViewer)
	{
		wxMenu *omni_menu = new wxMenu;
		omni_menu->Append(CALCHART__FollowMarcher, wxT("&Follow Marcher"), wxT("Follow Marcher"));
		omni_menu->Append(CALCHART__ToggleCrowd, wxT("&Toggle Crowd"), wxT("Toggle Crowd"));
		omni_menu->Append(CALCHART__ToggleMarching, wxT("&Toggle Marching"), wxT("Toggle Marching"));
		omni_menu->Append(CALCHART__ToggleShowOnlySelected, wxT("&Toggle Show Selected"), wxT("Toggle Show Selected"));
		omni_menu->Append(CALCHART__ShowKeyboardControls, wxT("&Show Controls"), wxT("Show keyboard controls"));
		menu_bar->Append(omni_menu, wxT("&OmniView"));
	}

	SetMenuBar(menu_bar);

// Add a toolbar
	AddCoolToolBar(GetAnimationToolBar(), this);

	// Add the field canvas
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	if (OmniViewer)
	{
		mOmniViewCanvas = new CCOmniView_Canvas(this, mView);
		topsizer->Add(mOmniViewCanvas, wxSizerFlags(1).Expand().Border(5));
	}
	else
	{
		mCanvas = new AnimationCanvas(this, mView);
		topsizer->Add(mCanvas, wxSizerFlags(1).Expand().Border(5));
	}

	// Add the controls
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Collisions")), wxSizerFlags());
	wxChoice *collis = new wxChoice(this, CALCHART__anim_collisions, wxDefaultPosition, wxDefaultSize, sizeof(collis_text)/sizeof(const wxString), collis_text);
	collis->SetSelection(1);
	sizer1->Add(collis, wxSizerFlags().Expand().Border(5));

	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("&Tempo")), wxSizerFlags());
	wxSlider *sldr = new wxSlider(this, CALCHART__anim_tempo, GetTempo(), 10, 300, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	sizer1->Add(sldr, wxSizerFlags().Expand().Border(5));

	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Errors")), wxSizerFlags());
	mErrorList = new wxChoice(this, CALCHART__anim_errors, wxDefaultPosition, wxDefaultSize, 0, NULL);
	mErrorList->SetSelection(1);
	mErrorList->Append(wxT("----------"));
	sizer1->Add(mErrorList, wxSizerFlags().Expand().Border(5));

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
	wxBoxSizer *Sub2 = new wxBoxSizer(wxVERTICAL);
	Sub2->Add(sizer1, wxSizerFlags(0).Left());
	Sub2->Add(sizer2, wxSizerFlags(0).Left());
	wxBoxSizer *Sub1 = new wxBoxSizer(wxHORIZONTAL);
	Sub1->Add(Sub2, wxSizerFlags(0).Left());
	mErrorText = new FancyTextWin(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, 100));
	Sub1->Add(mErrorText, wxSizerFlags().Expand().Border(5));
	
	topsizer->Add(Sub1, wxSizerFlags(0).Left());

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
	if (mCanvas)
		mCanvas->SetView(NULL);
	if (mOmniViewCanvas)
		mOmniViewCanvas->SetView(NULL);
	mTimer->Stop();
	delete mView;
}


void
AnimationFrame::SetView(wxView *view)
{
//	wxDocMDIChildFrame::SetView(view);
	if (mCanvas)
		mCanvas->SetView(static_cast<AnimationView*>(view));
	if (mOmniViewCanvas)
		mOmniViewCanvas->SetView(static_cast<AnimationView*>(view));
	mView = static_cast<AnimationView*>(view);
}


void
AnimationFrame::OnCmdReanimate(wxCommandEvent& event)
{
	StopTimer();
	mErrorMarkers.clear();
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
AnimationFrame::OnCmd_anim_errors(wxCommandEvent& event)
{
	mErrorText->Clear();
	size_t which = mErrorList->GetSelection() - 1;
	if (which < mErrorMarkers.size())
	{
		mView->SetSelection(mErrorMarkers.at(which).first.pntgroup);
		mView->GotoSheet(mErrorMarkers.at(which).second);

		mErrorText->Clear();
		CC_show::const_CC_sheet_iterator_t current_sheet = mView->GetShow()->GetNthSheet(mErrorMarkers.at(which).second);
		const CC_continuity& c = current_sheet->GetNthContinuity(mErrorMarkers.at(which).first.contnum);
		if (!c.GetText().IsEmpty())
		{
			mErrorText->WriteText(c.GetText());
			mErrorText->SetEditable(false);
		}
		if (mErrorMarkers.at(which).first.line > 0 && mErrorMarkers.at(which).first.col > 0)
		{
			mErrorText->SetInsertionPoint(mErrorText->XYToPosition((long)mErrorMarkers.at(which).first.line-1,(long)mErrorMarkers.at(which).first.col-1));
//			mErrorText->SetFocus();
		}
	}
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
AnimationFrame::OnCmd_FollowMarcher(wxCommandEvent& event)
{
	wxTextEntryDialog dialog(this,
							 wxT("Please enter a marcher to follow (or -1 to stop following)\n"),
							 wxT("Follow Marcher"),
							 wxT(""),
							 wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString value = dialog.GetValue();
		const std::vector<wxString>& labels = mView->GetShow()->GetPointLabels();
		std::vector<wxString>::const_iterator which = std::find(labels.begin(), labels.end(), value);
		if (which == labels.end())
		{
			wxString upper_value = value.Upper();
			which = std::find(labels.begin(), labels.end(), upper_value);
		}
		if (which == labels.end())
		{
			wxMessageDialog dialog(this, wxT("Could not find marcher"), wxT("Could not find marcher ") + value, wxOK);
			dialog.ShowModal();
			return;
		}
		if (mOmniViewCanvas)
		{
			mOmniViewCanvas->OnCmd_FollowMarcher(std::distance(labels.begin(), which));
		}
	}
}

void
AnimationFrame::OnCmd_SaveCameraAngle(wxCommandEvent& event)
{
}

void
AnimationFrame::OnCmd_GoToCameraAngle(wxCommandEvent& event)
{
}

void
AnimationFrame::OnCmd_ShowKeyboardControls(wxCommandEvent& event)
{
	wxMessageDialog dialog(this,
						   wxT("Keyboard Commands"),
						   wxT("1, 2, 3 : Select different camera angles (field, student, upper corner)\n")
						   wxT("+ : Move camera up\n")
						   wxT("- : Move camera down\n")
						   wxT("up arrow : Move camera forward\n")
						   wxT("down arrow : Move camera backward\n")
						   wxT("left arrow : Move camera left\n")
						   wxT("right arrow : Move camera rigth\n")
						   wxT("q : Pan camera left\n")
						   wxT("w : Pan camera right\n")
						   wxT("a : Pan camera up\n")
						   wxT("z : Pan camera down\n")
						   wxT("< : Decrease Field Of View\n")
						   wxT("> : Increase Field Of View\n")
						   wxT("o : Toggle Crowd\n")
						   wxT("t : Toggle show selected\n")
						   wxT("space : Toggle Marching\n"),
						   wxOK);
	dialog.ShowModal();
	return;
}

void
AnimationFrame::OnCmd_ToggleCrowd(wxCommandEvent& event)
{
	if (mOmniViewCanvas)
	{
		mOmniViewCanvas->OnCmd_ToggleCrowd();
	}
}

void
AnimationFrame::OnCmd_ToggleMarching(wxCommandEvent& event)
{
	if (mOmniViewCanvas)
	{
		mOmniViewCanvas->OnCmd_ToggleMarching();
	}
}

void
AnimationFrame::OnCmd_ToggleShowOnlySelected(wxCommandEvent& event)
{
	if (mOmniViewCanvas)
	{
		mOmniViewCanvas->OnCmd_ToggleShowOnlySelected();
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


bool
AnimationFrame::OnBeat() const
{
	return mBeatSlider->GetValue() & 1;
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

void
AnimationFrame::OnNotifyErrorList(const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message)
{
	for (unsigned i = 0; i < NUM_ANIMERR; i++)
	{
		if (!error_markers[i].pntgroup.empty())
		{
			wxString error_string;
			error_string.Printf(wxT("Sheet %d: \"%.32s\": %.32s"), sheetnum, message, animate_err_msgs[i]);
			mErrorList->Append(error_string);
			mErrorMarkers.push_back(std::pair<ErrorMarker, unsigned>(error_markers[i], sheetnum));
		}
	}
}
