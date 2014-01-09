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

#ifndef _ANIMATION_VIEW_H_
#define _ANIMATION_VIEW_H_

#include "animate.h"
#include "calchartdoc.h"
#include "cc_continuity.h"

#include <boost/shared_ptr.hpp>

class AnimationFrame;
class FieldView;

/**
 * A view of an Animation object that is connected to and recieves updates
 * from the CalChartDoc.
 */
class AnimationView : public wxView
{
public:
	/**
	 * Makes the view.
	 */
	AnimationView();
	/**
	 * Cleanup.
	 */
	~AnimationView();

//	virtual bool OnCreate(wxDocument *doc, long flags);
//	virtual bool OnClose(bool deleteWindow = true);
	/**
	 * Called when the view is drawn to the AnimationCanvas.
	 * @param dc The device context to draw with.
	 */
    virtual void OnDraw(wxDC *dc);
	/**
	 * Called when the CalChartDoc updates its views.
	 * @param sender The view that is sending the update.
	 * @param hint A message containing the reason for the update.
	 */
    virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

	void RefreshFrame();

	void SetCollisionType(CollisionWarning col);
	CollisionWarning GetCollisionType() const { return mCollisionWarningType; }
	void SelectCollisions();
	
	void Generate();

	/**
	 * Jumps to the previous beat of the animation.
	 * @return True if successful; false otherwise.
	 */
	bool PrevBeat();
	/**
	 * Jumps to the next beat of the animation.
	 * @return True if successful; false otherwise.
	 */
	bool NextBeat();
	/**
	 * Jumps to a specified beat of the current stuntsheet in the animation.
	 * @param i The beat of the current stunt sheet to jump to.
	 */
	void GotoBeat(unsigned i);
	/**
	 * Jumps to the beginning of the previous stuntsheet in the animation.
	 * @return True if successful; false otherwise.
	 */
	bool PrevSheet();
	/**
	 * Jumps to the beginning of the next stuntsheet in the animation.
	 * @return True if successful; false otherwise.
	 */
	bool NextSheet();
	/**
	 * Jumps to a particular stuntsheet of the animation.
	 * @param i The stuntsheet to jump to.
	 */
	void GotoSheet(unsigned i);
	/**
	 * Sets the active point selection.
	 * @param sl The list of points that should be selected.
	 */
	void SetSelection(const SelectionList& sl);

	// info
	/**
	 * Returns the number of sheets in the animation.
	 * @return The number of sheets in the animation.
	 */
	int GetNumberSheets() const;
	/**
	 * Returns the stuntsheet that is currently being
	 * animated.
	 */
	int GetCurrentSheet() const;
	/**
	 * Returns the number of beats in this stuntsheet
	 * of the animation.
	 */
	int GetNumberBeats() const;
	/**
	 * Returns the current beat being animated.
	 */
	int GetCurrentBeat() const;
	
	/**
	 * Returns the human-readable text describing the status of the
	 * animation.
	 */
	wxString GetStatusText() const;

	/**
	 * Returns the size of the region where the show is being animated.
	 * @return The size of the show.
	 */
	const CC_coord& GetShowSize() const;

	/**
	 * Deselects all marchers.
	 */
	void UnselectMarchers();
	/**
	 * Selects all marchers within a box.
	 * @param mouseXStart The x position of the first corner of the box.
	 * @param mouseYStart The y position of the first corner of the box.
	 * @param mouseXEnd The x position of the opposite corner of the box.
	 * @param mouseYEnd The y position of the opposite corner of the box.
	 * @param altDown True if the "alt" key is being pressed; false otherwise.
	 */
	void SelectMarchersInBox(long mouseXStart, long mouseYStart, long mouseXEnd, long mouseYEnd, bool altDown);
	
	void ToggleTimer();
	bool OnBeat() const;

	CC_continuity GetContinuityOnSheet(unsigned whichSheet, SYMBOL_TYPE whichSymbol) const;

	const ShowMode& GetShowMode() const;

	const CalChartDoc *GetShow() const;
	CalChartDoc *GetShow();

	boost::shared_ptr<Animation> GetAnimation();

private:
	void OnNotifyStatus(const wxString& status);
	bool OnNotifyErrorList(const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const wxString& message);
	
	bool mErrorOccurred;
	
	const AnimationFrame *GetAnimationFrame() const;
	AnimationFrame *GetAnimationFrame();

	/**
	 * The animation of the show that is being displayed through this view.
	 */
	boost::shared_ptr<Animation> mAnimation;
	CollisionWarning mCollisionWarningType;
};

#endif // _ANIMATION_VIEW_H_
