/*
 * AnimationView.h
 * Header for animation user interface
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

#ifndef _ANIM_UI_H_
#define _ANIM_UI_H_

#include "animate.h"

class AnimationFrame;

class AnimationView : public wxView
{
public:
	AnimationView();
	~AnimationView();

//	virtual bool OnCreate(wxDocument *doc, long flags);
//	virtual bool OnClose(bool deleteWindow = true);
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	void RefreshFrame();

	void EnableCollisions(CollisionWarning col);
	void CheckCollisions();
	void SelectCollisions();
	
	void Generate();

	// true if changes made
	bool PrevBeat();
	bool NextBeat();
	void GotoBeat(unsigned i);
	bool PrevSheet();
	bool NextSheet();
	void GotoSheet(unsigned i);

	// info
	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;
	
	wxString GetStatusText() const;

	const CC_coord& GetShowSize() const;

	void SelectMarchersInBox(long mouseXStart, long mouseYStart, long mouseXEnd, long mouseYEnd);
	
	void ToggleTimer();
	bool OnBeat() const;

private:
	void OnNotifyStatus(const wxString& status);
	bool OnNotifyErrorList(const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message);
	
	const AnimationFrame *GetAnimationFrame() const;
	AnimationFrame *GetAnimationFrame();
	const CC_show *GetShow() const;
	CC_show *GetShow();

	Animation *mAnimation;
	// Don't like this, need to avoid friends
	friend class CCOmniView_Canvas;
	friend class CCOmniView_Context;
	friend class AnimationFrame;
};

#endif
