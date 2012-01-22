/*
 * anim_ui.h
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
#include <wx/timer.h>
#define DEFAULT_ANIM_SIZE 3

class AnimationCanvas;
class AnimationFrame;

class AnimationTimer: public wxTimer
{
public:
	AnimationTimer(AnimationCanvas* c): canvas(c) {}

	void Notify();
private:
	AnimationCanvas* canvas;
};

class AnimationFrame;
// AnimationCanvas acts as both the controller and the view.
class AnimationCanvas: public wxPanel
{
public:
	AnimationCanvas(AnimationFrame *frame, CC_show *show);
	~AnimationCanvas();

	void OnPaint(wxPaintEvent& event);
	void OnLeftDownMouseEvent(wxMouseEvent& event);
	void OnLeftUpMouseEvent(wxMouseEvent& event);
	void OnRightDownMouseEvent(wxMouseEvent& event);
	void OnRightUpMouseEvent(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnChar(wxKeyEvent& event);

	inline unsigned GetTempo() { return tempo; }
	void SetTempo(unsigned t);

	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;

	inline void Redraw() { Refresh(); }
	void UpdateText();
	void RefreshCanvas();
	void Generate();
	void FreeAnim();

// Select colliding points and redraw
	void EnableCollisions(CollisionWarning col);
	void CheckCollisions();
	void SelectCollisions();

// true if changes made
	inline bool PrevBeat()
	{
		if (anim)
		{
			if (anim->PrevBeat())
			{
				RefreshCanvas(); return true;
			}
		}
		return false;
	}
	inline bool NextBeat()
	{
		if (anim)
		{
			if (anim->NextBeat())
			{
				RefreshCanvas(); return true;
			}
		}
		return false;
	}
	inline void GotoBeat(unsigned i)
	{
		if (anim) { anim->GotoBeat(i); RefreshCanvas(); }
	}
	inline bool PrevSheet()
	{
		if (anim)
		{
			if (anim->PrevSheet())
			{
				RefreshCanvas(); return true;
			}
		}
		return false;
	}
	inline bool NextSheet()
	{
		if (anim)
		{
			if (anim->NextSheet())
			{
				RefreshCanvas(); return true;
			}
		}
		return false;
	}
	inline void GotoSheet(unsigned i)
	{
		if (anim) { anim->GotoSheet(i); Refresh(); }
	}

	void StartTimer();
	inline void StopTimer()
	{
		timer->Stop(); timeron = false;
	}

private:
	void OnNotifyStatus(const wxString& status);
	bool OnNotifyErrorList(const ErrorMarker error_markers[NUM_ANIMERR], unsigned sheetnum, const wxString& message);

	Animation* anim;
	AnimationTimer* timer;
	bool timeron;

	CC_show *mShow;
	AnimationFrame *ourframe;
	unsigned tempo;
	float mUserScale;

	// for mouse and drawing
	bool mMouseDown;
	long mMouseXStart, mMouseYStart;
	long mMouseXEnd, mMouseYEnd;
	
	wxDECLARE_EVENT_TABLE();
};

class AnimationView : public wxView
{
public:
	AnimationView();
	~AnimationView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

class AnimErrorListView : public wxView
{
public:
	AnimErrorListView();
	~AnimErrorListView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

class AnimErrorList : public wxDialog
{
	wxDECLARE_DYNAMIC_CLASS( AnimErrorList );
	wxDECLARE_EVENT_TABLE();
	
public:
	AnimErrorList();
	AnimErrorList(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Animation Error"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	virtual ~AnimErrorList();

	void Init();

	bool Create(CC_show *show, const ErrorMarker error_markers[NUM_ANIMERR], unsigned num,
		wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Animation Error"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();
	bool TransferDataToWindow();

	void OnCmdUpdate(wxCommandEvent& event);

	inline bool Okay() { return ok; };

	void Unselect();
	void Update();
	void Update(int i);

private:
	CC_show *mShow;
	AnimErrorListView *mView;
	bool ok;
	unsigned sheetnum;
	ErrorMarker mErrorMarkers[NUM_ANIMERR];
	ErrorMarker pointsels[NUM_ANIMERR];
};
#endif
