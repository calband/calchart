/*
 * homeview_view.h
 * view for viewer only version
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#ifndef __HOMEVIEW_VIEW_H__
#define __HOMEVIEW_VIEW_H__

#include "CC_show.h"
#include "CC_coord.h"

#include <wx/docview.h>

#include <boost/shared_ptr.hpp>

class FieldFrame;
class Animation;

// Field:
// Field is the editable overhead view of the marchers on the field.
// This is where in the app you edit a marcher's location and continuity
/**
 * Unused, as far as I can tell. Related to HomeView.
 */
class HomeViewView : public wxView
{
public:
    FieldFrame *mFrame;
  
    HomeViewView();
    ~HomeViewView();

    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = true);

	void OnWizardSetup(CC_show& show);

	int FindPoint(CC_coord pos) const;
	CC_coord PointPosition(int which) const;

	///// Change show attributes /////
	void GoToSheet(size_t which);
	void GoToNextSheet();
	void GoToPrevSheet();

	///// Select /////
	void SelectWithLasso(const CC_lasso *lasso, bool toggleSelected);
	void SelectPointsInRect(const CC_coord& c1, const CC_coord& c2, bool toggleSelected);

private:
	CC_show* mShow;

    DECLARE_DYNAMIC_CLASS(HomeView)
};

#endif
