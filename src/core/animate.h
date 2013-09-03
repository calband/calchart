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
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <map>
#include <set>
#include <vector>

AnimateDir AnimGetDirFromAngle(float ang);

class AnimateCommand;
class AnimateSheet;
struct AnimateDraw;

struct ErrorMarker
{
public:
	std::set<unsigned> pntgroup;			  // which points have this error
	unsigned contnum;						  // which continuity
	int line, col;							  // where
	ErrorMarker(): contnum(0), line(-1), col(-1) {}
};

typedef boost::function<void (const std::string& notice)> NotifyStatus;
typedef boost::function<bool (const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const std::string& message)> NotifyErrorList;

class Animation
{
public:
	Animation(const CC_show& show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
	~Animation();

// Returns true if changes made
	void GotoSheet(unsigned i);
	bool PrevSheet();
	bool NextSheet();

	void GotoBeat(unsigned i);
	bool PrevBeat();
	bool NextBeat();

	typedef void (*CollisionAction_t)();
	// set collision action returns the previous collision action
	CollisionAction_t SetCollisionAction(CollisionAction_t col) { CollisionAction_t oldaction = mCollisionAction; mCollisionAction = col; return oldaction; }

	// For drawing:
	struct animate_info_t
	{
		bool mCollision;
		AnimateDir mDirection;
		float mRealDirection;
		CC_coord mPosition;
		animate_info_t(bool col, AnimateDir dir, float rdir, CC_coord pos) : mCollision(col), mDirection(dir), mRealDirection(rdir), mPosition(pos) {}
	};
	const animate_info_t GetAnimateInfo(unsigned which) const;

	int GetNumberSheets() const;
	int GetCurrentSheet() const;
	int GetNumberBeats() const;
	int GetCurrentBeat() const;
	std::string GetCurrentSheetName() const;

	// collection of position of each point, for debugging purposes
	std::string GetCurrentInfo() const;

	std::vector<AnimateDraw> GenPathToDraw(unsigned point, const CC_coord& offset) const;
	AnimatePoint EndPosition(unsigned point, const CC_coord& offset) const;

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

	std::vector<boost::shared_ptr<AnimateCommand> > GetCommands(unsigned whichPoint) const;
};

#endif
