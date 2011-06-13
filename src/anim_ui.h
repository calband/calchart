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

#include "basic_ui.h"
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
class AnimationCanvas: public wxPanel
{
public:
	AnimationCanvas(AnimationFrame *frame, CC_show *show);
	~AnimationCanvas();

	void OnEraseBackground(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnLeftMouseEvent(wxMouseEvent& event);
	void OnRightMouseEvent(wxMouseEvent& event);
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
#ifdef ANIM_OUTPUT_POVRAY
	wxString GeneratePOVFiles(const wxString& filebasename);
#endif
#ifdef ANIM_OUTPUT_RIB
	wxString GenerateRIBFrame();
	wxString GenerateRIBFile(const wxString& filename, bool all = true);
#endif

private:
	Animation* anim;
	AnimationTimer* timer;
	bool timeron;

	CC_show *mShow;
	AnimationFrame *ourframe;
	unsigned tempo;
	float mUserScale;

	DECLARE_EVENT_TABLE()
};

class AnimationView : public wxView
{
public:
	AnimationView();
	~AnimationView();
    virtual void OnDraw(wxDC *dc);
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
};

class AnimationFrame: public wxFrame
{
public:
	AnimationFrame(wxFrame *frame, CC_show *show);
	~AnimationFrame();

	void OnCmdReanimate(wxCommandEvent& event);
	void OnCmdSelectCollisions(wxCommandEvent& event);
#ifdef ANIM_OUTPUT_POVRAY
	void OnCmdPOV(wxCommandEvent& event);
#endif
#ifdef ANIM_OUTPUT_RIB
	void OnCmdRIBFrame(wxCommandEvent& event);
	void OnCmdRIBAll(wxCommandEvent& event);
	void OnCmdRIB(wxCommandEvent& event, bool allframes);
#endif
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

	DECLARE_EVENT_TABLE()
};

enum
{
	CALCHART__ANIM_REANIMATE = 1,
	CALCHART__ANIM_SELECT_COLL,
#ifdef ANIM_OUTPUT_POVRAY
	CALCHART__ANIM_POVRAY,
#endif
#ifdef ANIM_OUTPUT_RIB
	CALCHART__ANIM_RIB_FRAME,
	CALCHART__ANIM_RIB,
#endif

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
	DECLARE_CLASS( AnimErrorList )
	DECLARE_EVENT_TABLE()
	
public:
	AnimErrorList();
	AnimErrorList(AnimateCompile *comp, unsigned num,
		wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Animation Error"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	virtual ~AnimErrorList();

	void Init();

	bool Create(AnimateCompile *comp, unsigned num,
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

	CC_show *show;
private:
	AnimErrorListView *mView;
	bool ok;
	unsigned sheetnum;
	ErrorMarker mErrorMarkers[NUM_ANIMERR];
	ErrorMarker pointsels[NUM_ANIMERR];
};
#endif
