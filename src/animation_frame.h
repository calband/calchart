/*
 * animation_frame.h
 * Header for AnimationFrame
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

#ifndef _ANIMATION_FRAME_H_
#define _ANIMATION_FRAME_H_

#include "animate.h"

#include <wx/wx.h>
#include <wx/docmdi.h>

class AnimationView;
class AnimationCanvas;
class CCOmniView_Canvas;
class FancyTextWin;
class wxSplitterWindow;

class AnimationFrame: public wxFrame
{
public:
	AnimationFrame(wxWindow *parent, wxDocument* doc);
	~AnimationFrame();
	
	virtual void SetView(wxView *view);

	void OnCmdReanimate(wxCommandEvent& event);
	void OnCmdSelectCollisions(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);

	void OnCmd_anim_stop(wxCommandEvent& event);
	void OnCmd_anim_play(wxCommandEvent& event);
	void OnCmd_anim_prev_beat(wxCommandEvent& event);
	void OnCmd_anim_next_beat(wxCommandEvent& event);
	void OnCmd_anim_next_beat_timer(wxTimerEvent& event);
	void OnCmd_anim_prev_sheet(wxCommandEvent& event);
	void OnCmd_anim_next_sheet(wxCommandEvent& event);
	void OnCmd_anim_collisions(wxCommandEvent& event);
	void OnCmd_anim_errors(wxCommandEvent& event);
	void OnSlider_anim_tempo(wxScrollEvent& event);
	void OnSlider_anim_gotosheet(wxScrollEvent& event);
	void OnSlider_anim_gotobeat(wxScrollEvent& event);

	void OnCmd_FollowMarcher(wxCommandEvent& event);
	void OnCmd_SaveCameraAngle(wxCommandEvent& event);
	void OnCmd_GoToCameraAngle(wxCommandEvent& event);
	void OnCmd_ShowKeyboardControls(wxCommandEvent& event);
	void OnCmd_ToggleCrowd(wxCommandEvent& event);
	void OnCmd_ToggleMarching(wxCommandEvent& event);
	void OnCmd_ToggleShowOnlySelected(wxCommandEvent& event);

	// Called by the view
	void ToggleTimer();
	void UpdatePanel();
	CollisionWarning CollisionType();
	
	bool OnBeat() const;

	void OnNotifyErrorList(const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message);

	// controlling how the screen splits between views
	void OnCmd_SplitViewHorizontal(wxCommandEvent& event);
	void OnCmd_SplitViewVertical(wxCommandEvent& event);
	void OnCmd_SplitViewUnsplit(wxCommandEvent& event);
	void OnCmd_SwapAnimateAndOmni(wxCommandEvent& event);

	// update UI when these happen
	void OnCmd_UpdateUIHorizontal(wxUpdateUIEvent& event);
	void OnCmd_UpdateUIVertical(wxUpdateUIEvent& event);
	void OnCmd_UpdateUIUnsplit(wxUpdateUIEvent& event);
	
private:
	AnimationView *mView;
	// we really do need one of each.  We can't do inheritance because they have different base classes 
	AnimationCanvas *mCanvas;
	CCOmniView_Canvas *mOmniViewCanvas;
	wxSlider *mSheetSlider;
	wxSlider *mBeatSlider;

	wxSplitterWindow *mSplitter;
	// these are just the observers we use to manipulate the split
	wxWindow *mSplitA;
	wxWindow *mSplitB;
	
	// continuity errors:
	wxChoice *mErrorList;
	std::vector<std::pair<ErrorMarker, unsigned> > mErrorMarkers;
	FancyTextWin *mErrorText;
	
// timer stuff:
	void StartTimer();
	void StopTimer();
	unsigned GetTempo() const;
	void SetTempo(unsigned tempo);
	wxTimer *mTimer;
	unsigned mTempo;
	bool mTimerOn;

	wxDECLARE_EVENT_TABLE();
};

#endif // _ANIMATION_FRAME_H_

