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

class AnimationFrame: public wxFrame
{
public:
	AnimationFrame(wxWindow *parent, wxDocument* doc, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
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
	void OnSlider_anim_tempo(wxScrollEvent& event);
	void OnSlider_anim_gotosheet(wxScrollEvent& event);
	void OnSlider_anim_gotobeat(wxScrollEvent& event);

	// Called by the view
	void ToggleTimer();
	void UpdatePanel();
	CollisionWarning CollisionType();

private:
	AnimationView *mView;
	AnimationCanvas *mCanvas;
	wxSlider *mSheetSlider;
	wxSlider *mBeatSlider;

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

