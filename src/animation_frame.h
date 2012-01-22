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

class AnimationCanvas;
class AnimationView;
class CC_show;

enum
{
	CALCHART__ANIM_REANIMATE = 1,
	CALCHART__ANIM_SELECT_COLL,
	
	CALCHART__anim_stop,
	CALCHART__anim_play,
	CALCHART__anim_prev_beat,
	CALCHART__anim_next_beat,
	CALCHART__anim_prev_sheet,
	CALCHART__anim_next_sheet,
	CALCHART__anim_collisions,
	CALCHART__anim_tempo,
	CALCHART__anim_gotosheet,
	CALCHART__anim_gotobeat,
	
	CALCHART__anim_update
};

class AnimationFrame: public wxFrame
{
public:
	AnimationFrame(wxFrame *frame, CC_show *show);
	~AnimationFrame();

	void OnCmdReanimate(wxCommandEvent& event);
	void OnCmdSelectCollisions(wxCommandEvent& event);
	void OnCmdClose(wxCommandEvent& event);

	void OnCmd_anim_stop(wxCommandEvent& event);
	void OnCmd_anim_play(wxCommandEvent& event);
	void OnCmd_anim_prev_beat(wxCommandEvent& event);
	void OnCmd_anim_next_beat(wxCommandEvent& event);
	void OnCmd_anim_prev_sheet(wxCommandEvent& event);
	void OnCmd_anim_next_sheet(wxCommandEvent& event);
	void OnCmd_anim_collisions(wxCommandEvent& event);
	void OnSlider_anim_tempo(wxScrollEvent& event);
	void OnSlider_anim_gotosheet(wxScrollEvent& event);
	void OnSlider_anim_gotobeat(wxScrollEvent& event);
	void OnChar(wxKeyEvent& event);

	void UpdatePanel();

	inline CollisionWarning CollisionType()
	{
		return (CollisionWarning)collis->GetSelection();
	}

	AnimationCanvas *canvas;
private:
	AnimationView *mView;
	wxChoice *collis;
	wxSlider *sheet_slider;
	wxSlider *beat_slider;

	friend class AnimationCanvas;

	wxDECLARE_EVENT_TABLE();
};

#endif // _ANIMATION_FRAME_H_

