/*
 * animation_error.h
 * Header for animation error dialog
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

#ifndef _ANIMATION_ERROR_H_
#define _ANIMATION_ERROR_H_

#include "animate.h"

class CC_show;

// This could possibly be replaced by a simple dialog, except we would like to interact with errors
// so we require having a slightly more complication window
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

#endif // _ANIMATION_ERROR_H_
