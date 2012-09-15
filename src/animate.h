/*
 * animate.h
 * Classes for animating shows
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

#ifndef _ANIMATE_H_
#define _ANIMATE_H_

#include "animate_types.h"

#include "cc_coord.h"
#include "cc_show.h"
#include <wx/string.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <map>
#include <set>
#include <vector>

AnimateDir AnimGetDirFromAngle(float ang);

class AnimateCommand;
class AnimateSheet;

class ErrorMarker
{
public:
	std::set<unsigned> pntgroup;			  // which points have this error
	unsigned contnum;						  // which continuity
	int line, col;							  // where
	ErrorMarker(): contnum(0), line(-1), col(-1) {}
	~ErrorMarker() {}
	void Reset()
	{
		pntgroup.clear();
		contnum = 0;
		line = col = -1;
	}
};

typedef boost::function<void (const wxString& notice)> NotifyStatus;
typedef boost::function<bool (const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const wxString& message)> NotifyErrorList;

class Animation
{
public:
	Animation(CC_show *show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
	~Animation();

// Returns true if changes made
	bool PrevSheet();
	bool NextSheet();

	void GotoBeat(unsigned i);

	bool PrevBeat();
	bool NextBeat();

	void GotoSheet(unsigned i);

	typedef void (*CollisionAction_t)();
	// set collision action returns the previous collision action
	CollisionAction_t SetCollisionAction(CollisionAction_t col) { CollisionAction_t oldaction = mCollisionAction; mCollisionAction = col; return oldaction; }

	// For drawing:
	bool IsCollision(unsigned which) const;
	AnimateDir Direction(unsigned which) const;
	float RealDirection(unsigned which) const;
	CC_coord Position(unsigned which) const;

	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;
	wxString GetCurrentSheetName() const;

	// collection of position of each point, for debugging purposes
	std::string GetCurrentInfo() const;

	void DrawPath(wxDC& dc, int whichPoint, const CC_coord& offset) const;

private:
	const unsigned numpts;
	std::vector<AnimatePoint> pts;
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> >::iterator > curr_cmds; // pointer to the current command
	std::set<int> mCollisions;
	unsigned curr_sheetnum;
	unsigned curr_beat;

	void BeginCmd(unsigned i);
	void EndCmd(unsigned i);

	void RefreshSheet();

	std::vector<AnimateSheet> sheets;
	
	void CheckCollisions();
	CollisionAction_t mCollisionAction;
};

#endif
