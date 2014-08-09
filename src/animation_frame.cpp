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
#include "confgr.h"

#include <wx/timer.h>
#include <wx/splitter.h>
#include <wx/spinctrl.h>


BEGIN_EVENT_TABLE(AnimationFrame, AnimationFrame::super)
EVT_MENU(CALCHART__anim_reanimate, AnimationFrame::OnCmdReanimate)
EVT_MENU(CALCHART__anim_select_coll, AnimationFrame::OnCmdSelectCollisions)
EVT_MENU(wxID_CLOSE, AnimationFrame::OnCmdClose)
EVT_MENU(CALCHART__anim_stop, AnimationFrame::OnCmd_anim_stop)
EVT_MENU(CALCHART__anim_play, AnimationFrame::OnCmd_anim_play)
EVT_MENU(CALCHART__anim_prev_beat, AnimationFrame::OnCmd_anim_prev_beat)
EVT_MENU(CALCHART__anim_next_beat, AnimationFrame::OnCmd_anim_next_beat)
EVT_MENU(CALCHART__anim_prev_sheet, AnimationFrame::OnCmd_anim_prev_sheet)
EVT_MENU(CALCHART__anim_next_sheet, AnimationFrame::OnCmd_anim_next_sheet)
// omniviewer:
EVT_MENU(CALCHART__FollowMarcher, AnimationFrame::OnCmd_FollowMarcher)
EVT_MENU(CALCHART__SaveCameraAngle, AnimationFrame::OnCmd_SaveCameraAngle)
EVT_MENU(CALCHART__GoToCameraAngle, AnimationFrame::OnCmd_GoToCameraAngle)
EVT_MENU(CALCHART__ShowKeyboardControls, AnimationFrame::OnCmd_ShowKeyboardControls)
EVT_MENU(CALCHART__ToggleCrowd, AnimationFrame::OnCmd_ToggleCrowd)
EVT_MENU(CALCHART__ToggleMarching, AnimationFrame::OnCmd_ToggleMarching)
EVT_MENU(CALCHART__ToggleShowOnlySelected, AnimationFrame::OnCmd_ToggleShowOnlySelected)
// How to view show:
EVT_MENU(CALCHART__SplitViewHorizontal, AnimationFrame::OnCmd_SplitViewHorizontal)
EVT_MENU(CALCHART__SplitViewVertical, AnimationFrame::OnCmd_SplitViewVertical)
EVT_MENU(CALCHART__SplitViewUnsplit, AnimationFrame::OnCmd_SplitViewUnsplit)
EVT_MENU(CALCHART__SplitViewSwapAnimateAndOmni, AnimationFrame::OnCmd_SwapAnimateAndOmni)
EVT_CHOICE(CALCHART__anim_collisions, AnimationFrame::OnCmd_anim_collisions)
EVT_CHOICE(CALCHART__anim_errors, AnimationFrame::OnCmd_anim_errors)
EVT_SPINCTRL(CALCHART__anim_tempo, AnimationFrame::OnSlider_anim_tempo)
EVT_COMMAND_SCROLL(CALCHART__anim_gotosheet, AnimationFrame::OnSlider_anim_gotosheet)
EVT_COMMAND_SCROLL(CALCHART__anim_gotobeat, AnimationFrame::OnSlider_anim_gotobeat)
EVT_TIMER(CALCHART__anim_next_beat_timer, AnimationFrame::OnCmd_anim_next_beat_timer)
EVT_UPDATE_UI(CALCHART__SplitViewHorizontal, AnimationFrame::OnCmd_UpdateUIHorizontal)
EVT_UPDATE_UI(CALCHART__SplitViewVertical, AnimationFrame::OnCmd_UpdateUIVertical)
EVT_UPDATE_UI(CALCHART__SplitViewUnsplit, AnimationFrame::OnCmd_UpdateUIUnsplit)
EVT_CLOSE(AnimationFrame::OnCmdClose)
EVT_SIZE(AnimationFrame::OnSize)
END_EVENT_TABLE()


AnimationFrame::AnimationFrame(std::function<void ()> onClose, wxDocument *doc, wxView *view, wxFrame *parent, const wxSize& size) :
#if defined(BUILD_FOR_VIEWER) && (BUILD_FOR_VIEWER != 0)
wxDocChildFrame(doc, view, parent, wxID_ANY, wxT("CalChart Viewer"), wxDefaultPosition, size),
#else
wxFrame(parent, wxID_ANY, wxT("CalChart Viewer"), wxDefaultPosition, size),
#endif
mCanvas(NULL),
mOmniViewCanvas(NULL),
mTimer(new wxTimer(this, CALCHART__anim_next_beat_timer)),
mTempo(120),
mTimerOn(false),
mWhenClosed(onClose)
{
// Give it an icon
	// give this a view so it can pick up document changes
	mAnimationView = new AnimationView();
	mAnimationView->SetDocument(doc);
	mAnimationView->SetFrame(this);
	SetBandIcon(this);

	// this frame has 2 status bars at the bottom
	CreateStatusBar(2);

// Make a menubar
	wxMenu *anim_menu = new wxMenu;
	anim_menu->Append(CALCHART__anim_reanimate, wxT("&Reanimate Show...\tCTRL-RETURN"), wxT("Regenerate animation"));
	anim_menu->Append(CALCHART__anim_select_coll, wxT("&Select Collisions"), wxT("Select colliding points"));
	anim_menu->Append(wxID_CLOSE, wxT("&Close Window\tCTRL-W"), wxT("Close window"));

	
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(anim_menu, wxT("&Animate"));
	
	wxMenu *omni_menu = new wxMenu;
	omni_menu->Append(CALCHART__FollowMarcher, wxT("&Follow Marcher"), wxT("Follow Marcher"));
	omni_menu->Append(CALCHART__ToggleCrowd, wxT("&Toggle Crowd"), wxT("Toggle Crowd"));
	omni_menu->Append(CALCHART__ToggleMarching, wxT("&Toggle Marching"), wxT("Toggle Marching"));
	omni_menu->Append(CALCHART__ToggleShowOnlySelected, wxT("&Toggle Show Selected"), wxT("Toggle Show Selected"));
	omni_menu->Append(CALCHART__ShowKeyboardControls, wxT("&Show Controls"), wxT("Show keyboard controls"));
	menu_bar->Append(omni_menu, wxT("&OmniView"));

	wxMenu *split_menu = new wxMenu;
	split_menu->Append(CALCHART__SplitViewHorizontal, wxT("Split &Horizontally\tCtrl-H"), wxT("Split Horizontally"));
	split_menu->Append(CALCHART__SplitViewVertical, wxT("Split &Vertically\tCtrl-V"), wxT("Split Vertically"));
	split_menu->Append(CALCHART__SplitViewUnsplit, wxT("&Unsplit\tCtrl-U"), wxT("Unsplit"));
	split_menu->Append(CALCHART__SplitViewSwapAnimateAndOmni, wxT("&Swap Views\tCtrl-S"), wxT("Swap Views"));
	menu_bar->Append(split_menu, wxT("&Split"));
	
	SetMenuBar(menu_bar);

// Add a toolbar
	// Add the field canvas here so that it gets the focus when we switch to frame.
	mSplitter = new wxSplitterWindow(this, wxID_ANY);
	mSplitter->SetSize(GetClientSize());
	mSplitter->SetSashGravity(0.5);
	mSplitter->SetMinimumPaneSize(20);
	mSplitter->SetWindowStyleFlag(mSplitter->GetWindowStyleFlag() | wxSP_LIVE_UPDATE);
	mSplitter->SetMinSize(wxSize(300, 400));
	
	mOmniViewCanvas = new CCOmniView_Canvas(mAnimationView, mSplitter);
	mCanvas = new AnimationCanvas(mAnimationView, mSplitter);
	
	mSplitA = mOmniViewCanvas;
	mSplitB = mCanvas;
	if (!GetConfiguration_AnimationFrameOmniAnimation())
	{
		std::swap(mSplitA, mSplitB);
	}
	mSplitter->Initialize(mSplitA);
	if (GetConfiguration_AnimationFrameSplitScreen())
	{
		if (GetConfiguration_AnimationFrameSplitVertical())
		{
			mSplitter->SplitVertically(mSplitA, mSplitB);
		}
		else
		{
			mSplitter->SplitHorizontally(mSplitA, mSplitB);
		}
	}
	mSplitter->SetSashPosition(GetConfiguration_AnimationFrameSashPosition());
		
	
	AddCoolToolBar(GetAnimationToolBar(), *this);

	// set up the frame
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *toprow = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags topRowSizerFlags = wxSizerFlags(0).Expand().Border(0, 5);
	wxSizerFlags centerText = wxSizerFlags(0).Border(wxALL, 5).Align(wxALIGN_CENTER_HORIZONTAL);
	wxSizerFlags centerWidget = wxSizerFlags(0).Expand().Border(wxALL, 5).Align(wxALIGN_CENTER_HORIZONTAL);
	
	// Add the controls
	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Collisions")), centerText);
	static const wxString collis_text[] =
	{
		wxT("Ignore"), wxT("Show"), wxT("Beep")
	};
	wxChoice *collis = new wxChoice(this, CALCHART__anim_collisions, wxDefaultPosition, wxDefaultSize, sizeof(collis_text)/sizeof(const wxString), collis_text);
	collis->SetSelection(mAnimationView->GetCollisionType());
	sizer1->Add(collis, centerWidget);
	toprow->Add(sizer1, topRowSizerFlags);

	sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Tempo")), centerText);
	// defect 3538572: Callings set may update tempo, cache value before call.
	unsigned tmp = GetTempo();
	wxSpinCtrl *tempo = new wxSpinCtrl(this, CALCHART__anim_tempo);
	tempo->SetRange(10, 300);
	tempo->SetValue(tmp);
	SetTempo(tmp);
	sizer1->Add(tempo, centerWidget);
	toprow->Add(sizer1, topRowSizerFlags);

// Sheet slider (will get set later with UpdatePanel())
	sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Sheet")), centerText);
	mSheetSlider = new wxSlider(this, CALCHART__anim_gotosheet, 1, 1, 2, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
	sizer1->Add(mSheetSlider, centerWidget);
	toprow->Add(sizer1, topRowSizerFlags);

// Beat slider (will get set later with UpdatePanel())
	sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Beat")), centerText);
	mBeatSlider = new wxSlider(this, CALCHART__anim_gotobeat, 0, 0, 1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
	sizer1->Add(mBeatSlider, centerWidget);
	toprow->Add(sizer1, topRowSizerFlags);

	sizer1 = new wxBoxSizer(wxVERTICAL);
	mErrorList = new wxChoice(this, CALCHART__anim_errors, wxDefaultPosition, wxDefaultSize, 0, NULL);
	mErrorList->Append(wxT("Errors..."));
	mErrorList->SetSelection(0);
	sizer1->Add(mErrorList, centerWidget);
	mErrorText = new FancyTextWin(this, wxID_ANY);
	sizer1->Add(mErrorText, wxSizerFlags(1).Expand().Border(wxALL, 5).Align(wxALIGN_CENTER_HORIZONTAL));
	toprow->Add(sizer1, topRowSizerFlags);
	
	topsizer->Add(toprow, wxSizerFlags(0).Border(0, 5));
	
	// Add the field canvas
	topsizer->Add(mSplitter, wxSizerFlags(1).Expand());

	SetSizer( topsizer );
	topsizer->SetSizeHints(this);				  // set size hints to honour minimum size
	this->Layout();

	mAnimationView->Generate();

	UpdatePanel();

	Fit();
	SetSize(size);
	Show(true);

	// make animation screen large by default.
//	Maximize(true);
}


AnimationFrame::~AnimationFrame()
{
	if (mCanvas)
		mCanvas->SetView(NULL);
	if (mOmniViewCanvas)
		mOmniViewCanvas->SetView(NULL);
	mTimer->Stop();
	delete mAnimationView;
}


void
AnimationFrame::OnSize(wxSizeEvent& event)
{
	// HACK: Prevent width and height from growing out of control
	int w = event.GetSize().GetWidth();
	int h = event.GetSize().GetHeight();
	SetConfiguration_AnimationFrameWidth((w > 1200) ? 1200 : w);
	SetConfiguration_AnimationFrameHeight((h > 700) ? 700 : h);
	super::OnSize(event);
}


void
AnimationFrame::OnCmdReanimate(wxCommandEvent& event)
{
	StopTimer();
	mErrorMarkers.clear();
	if (mAnimationView)
	{
		mAnimationView->Generate();
	}
}


void
AnimationFrame::OnCmdSelectCollisions(wxCommandEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->SelectCollisions();
	}
}


void
AnimationFrame::OnCmdClose(wxCloseEvent& event)
{
	if (mSplitter)
	{
		SetConfiguration_AnimationFrameSashPosition(mSplitter->GetSashPosition());
	}
	// we should inform the parent frame we are closing
	if (mWhenClosed)
	{
		mWhenClosed();
	}
	Destroy();
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
	if (mAnimationView)
	{
		mAnimationView->PrevBeat();
	}
}


void
AnimationFrame::OnCmd_anim_next_beat(wxCommandEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->NextBeat();
	}
}


void
AnimationFrame::OnCmd_anim_next_beat_timer(wxTimerEvent& event)
{
	// next_beat could come from the timer.  If so, stop the timer.
	if (mAnimationView && !mAnimationView->NextBeat())
	{
		StopTimer();
	}
}


void
AnimationFrame::OnCmd_anim_prev_sheet(wxCommandEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->PrevSheet();
	}
}


void
AnimationFrame::OnCmd_anim_next_sheet(wxCommandEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->NextSheet();
	}
}


void
AnimationFrame::OnCmd_anim_collisions(wxCommandEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->SetCollisionType(static_cast<CollisionWarning>(event.GetSelection()));
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
		mAnimationView->SetSelection(mErrorMarkers.at(which).first.pntgroup);
		mAnimationView->GotoSheet(mErrorMarkers.at(which).second);

		mErrorText->Clear();
		CC_continuity c = mAnimationView->GetContinuityOnSheet(mErrorMarkers.at(which).second, mErrorMarkers.at(which).first.contsymbol);
		if (!c.GetText().empty())
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
AnimationFrame::OnSlider_anim_tempo(wxSpinEvent& event)
{
	SetTempo(event.GetPosition());
	StartTimer();
}


void
AnimationFrame::OnSlider_anim_gotosheet(wxScrollEvent& event)
{
	if (mAnimationView)
	{
		mAnimationView->GotoAnimationSheet(event.GetPosition()-1);
	}
}


void
AnimationFrame::OnSlider_anim_gotobeat(wxScrollEvent& event)
{
	if (mAnimationView)
	{
		//False if the change in the slider should cause the animation to transition to the next/previous stunt sheet
		//true otherwise
		bool beatChangeIsInternal = true;
		if (OnSlider_shouldTransitionToNextSheet(event)) {
			beatChangeIsInternal = false;
			TransitionToNextSheet();
		}
		else if (OnSlider_shouldTransitionToPreviousSheet(event)) {
			beatChangeIsInternal = false;
			TransitionToPreviousSheet();
		}
		if (beatChangeIsInternal) {
			mAnimationView->GotoBeat(event.GetPosition());
		}
	}
}

bool
AnimationFrame::OnSlider_shouldTransitionToNextSheet(wxScrollEvent& event) {
	if (OnSlider_isNextBeatEvent(event)) {
		return (mAnimationView->GetCurrentBeat() == mAnimationView->GetNumberBeats() - 1);
	}
	return false;
}

bool
AnimationFrame::OnSlider_shouldTransitionToPreviousSheet(wxScrollEvent& event) {
	if (OnSlider_isPreviousBeatEvent(event)) {
		return (mAnimationView->GetCurrentBeat() == 0);
	}
	return false;
}

bool
AnimationFrame::OnSlider_isPreviousBeatEvent(wxScrollEvent& event) {
	wxEventType pEventType = event.GetEventType();
	return (pEventType == wxEVT_SCROLL_LINEUP || pEventType == wxEVT_SCROLL_PAGEUP);
}

bool
AnimationFrame::OnSlider_isNextBeatEvent(wxScrollEvent& event) {
	wxEventType pEventType = event.GetEventType();
	return (pEventType == wxEVT_SCROLL_LINEDOWN || pEventType == wxEVT_SCROLL_PAGEDOWN);
}

void
AnimationFrame::TransitionToNextSheet() {
	mAnimationView->GotoBeat(mAnimationView->GetNumberBeats() - 1);
	mAnimationView->NextBeat();
}

void
AnimationFrame::TransitionToPreviousSheet() {
	mAnimationView->GotoBeat(0);
	mAnimationView->PrevBeat();
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
		auto& labels = mAnimationView->GetShow()->GetPointLabels();
		auto which = std::find(labels.begin(), labels.end(), value);
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
		SelectionList sl;
		sl.insert(std::distance(labels.begin(), which));
		mAnimationView->SetSelection(sl);
	}
	Refresh();
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
						   wxT("1, 2, 3 : Select different camera angles (student, field, upper corner)\n")
						   wxT("shift-1, shift-2, shift-3 : Select ALUMNI different camera angles (Alumni, field viewing east, upper corner)\n")
						   wxT("4, 5, 6 : Select custom camera angles (set to student, field, upper corner by default)\n")
						   wxT("shift-4, shift-5, shift-6 : Set custom camera angle\n")
						   wxT("+ : Move camera up\n")
						   wxT("- : Move camera down\n")
						   wxT("w : Move camera forward\n")
						   wxT("s : Move camera backward\n")
						   wxT("a : Move camera left\n")
						   wxT("d : Move camera right\n")
						   wxT("q : Pan camera left\n")
						   wxT("e : Pan camera right\n")
						   wxT("r : Pan camera up\n")
						   wxT("f : Pan camera down\n")
						   wxT("< : Decrease Field Of View\n")
						   wxT("> : Increase Field Of View\n")
						   wxT("o : Toggle Crowd\n")
						   wxT("t : Toggle show selected\n")
						   wxT("left arrow : Back 1 beat\n")
						   wxT("right arrow : Forward 1 beat\n")
						   wxT("space : Toggle Marching\n"),
						   wxT("Keyboard Commands"),
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
	int num = (mAnimationView) ? mAnimationView->GetNumberSheets() : 1;
	int curr = (mAnimationView) ? mAnimationView->GetCurrentSheet()+1 : 1;

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

	num = (mAnimationView) ? mAnimationView->GetNumberBeats()-1 : 1;
	curr = (mAnimationView) ? mAnimationView->GetCurrentBeat() : 1;

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
	if (mAnimationView)
	{
		SetStatusText(mAnimationView->GetStatusText(), 1);
	}
}


bool
AnimationFrame::OnBeat() const
{
	return mBeatSlider->GetValue() & 1;
}


// controlling how the screen splits between views
void
AnimationFrame::OnCmd_SplitViewHorizontal(wxCommandEvent& event)
{
	if (mSplitter->IsSplit())
	{
		mSplitter->Unsplit();
	}
	mSplitA->Show(true);
	mSplitB->Show(true);
	mSplitter->SplitHorizontally(mSplitA, mSplitB);
	SetConfiguration_AnimationFrameSplitScreen(true);
	SetConfiguration_AnimationFrameSplitVertical(false);
}

void
AnimationFrame::OnCmd_SplitViewVertical(wxCommandEvent& event)
{
	if (mSplitter->IsSplit())
	{
		mSplitter->Unsplit();
	}
	mSplitA->Show(true);
	mSplitB->Show(true);
	mSplitter->SplitVertically(mSplitA, mSplitB);
	SetConfiguration_AnimationFrameSplitScreen(true);
	SetConfiguration_AnimationFrameSplitVertical(true);
}

void
AnimationFrame::OnCmd_SplitViewUnsplit(wxCommandEvent& event)
{
	if (mSplitter->IsSplit())
	{
		mSplitter->Unsplit();
	}
	SetConfiguration_AnimationFrameSplitScreen(false);
}

void
AnimationFrame::OnCmd_SwapAnimateAndOmni(wxCommandEvent& event)
{
	bool isSplit = mSplitter->IsSplit();
	wxSplitMode mode = mSplitter->GetSplitMode();
	if (isSplit)
	{
		mSplitter->Unsplit();
	}
	std::swap(mSplitA, mSplitB);
	SetConfiguration_AnimationFrameOmniAnimation(!GetConfiguration_AnimationFrameOmniAnimation());
	if (mode == wxSPLIT_HORIZONTAL)
	{
		mSplitter->SplitHorizontally(mSplitA, mSplitB);
	}
	else if (mode == wxSPLIT_VERTICAL)
	{
		mSplitter->SplitVertically(mSplitA, mSplitB);
	}
	if (!isSplit)
	{
		mSplitter->Unsplit();
	}
}

void
AnimationFrame::OnCmd_UpdateUIHorizontal(wxUpdateUIEvent& event)
{
	wxSplitMode mode = mSplitter->GetSplitMode();
	event.Enable( (!mSplitter->IsSplit()) || (mode != wxSPLIT_HORIZONTAL) );
}

void
AnimationFrame::OnCmd_UpdateUIVertical(wxUpdateUIEvent& event)
{
	wxSplitMode mode = mSplitter->GetSplitMode();
	event.Enable( (!mSplitter->IsSplit()) || (mode != wxSPLIT_VERTICAL) );
}

void
AnimationFrame::OnCmd_UpdateUIUnsplit(wxUpdateUIEvent& event)
{
	event.Enable( mSplitter->IsSplit() );
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
AnimationFrame::OnNotifyErrorList(const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const wxString& message)
{
	for (unsigned i = 0; i < error_markers.size(); i++)
	{
		if (!error_markers[i].pntgroup.empty())
		{
			wxString error_string;
			error_string.Printf(wxT("Sheet %d: \"%.32s\": %.32s"), sheetnum, message, animate_err_msgs(i));
			mErrorList->Append(error_string);
			mErrorMarkers.push_back(std::pair<ErrorMarker, unsigned>(error_markers[i], sheetnum));
		}
	}
}
