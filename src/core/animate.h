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
struct CC_DrawCommand;

/**
 * The data collected when an error occurs.
 */
struct ErrorMarker
{
public:
	/**
	 * The group of points involved in the error.
	 */
	std::set<unsigned> pntgroup;
	/**
	 * The symbol of the continuity that is involved in the error.
	 */
	SYMBOL_TYPE contsymbol;
	/**
	 * The line of the continuity text that is causing the error.
	 */
	int line;
	/**
	 * The column of the continuity text where the error originates.
	 */
	int col;
	
	/**
	 * Makes an empty error.
	 */
	ErrorMarker(): contsymbol(SYMBOL_PLAIN), line(-1), col(-1) {}
};

typedef boost::function<void (const std::string& notice)> NotifyStatus;
typedef boost::function<bool (const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const std::string& message)> NotifyErrorList;

/**
 * An object which can animate the points of a show - that is, it can solve
 * for the positions of the marchers as they step through a show.
 */
class Animation
{
public:
	/**
	 * Animates the current snapshot of a show.
	 * @param show The show to animate. 
	 * @param notifyStatus An object that should recieve notifications about
	 * the current status of the animation.
	 * @param notifyErrorList An object that should recieve notifications about
	 * errors.
	 */
	Animation(const CC_show& show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
	/**
	 * Cleanup.
	 */
	~Animation();

	/**
	 * Jumps to a particular stuntsheet.
	 * @param i The index of the sheet to jump to.
	 */
	void GotoSheet(unsigned i);
	/**
	 * Jumps to the beginning of the previous stunt sheet.
	 * @return True if successful; false otherwise.
	 */
	bool PrevSheet();
	/**
	 * Jumps to the beginning of the next stunt sheet.
	 * @return True if successful; false otherwise.
	 */
	bool NextSheet();

	/**
	 * Jumps to a particular beat of the current sheet.
	 * @param i The beat to jump to.
	 */
	void GotoBeat(unsigned i);
	/**
	 * Jumps to the previous beat of the current sheet. If the beat proceeds
	 * past the beginning of the current sheet, then the animation will jump
	 * to the previous sheet.
	 * @return True if successful; false otherwise.
	 */
	bool PrevBeat();
	/**
	 * Jumps to the next beat of the current sheet. If the beat proceeds
	 * beyond the end of the current sheet, then the animation will jump to
	 * the next sheet.
	 * @return True if successful; false otherwise.
	 */
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
	animate_info_t GetAnimateInfo(unsigned which) const;

	/**
	 * Returns the number of stunt sheets in the animation.
	 * @return The number of stunt sheets in the animation.
	 */
	int GetNumberSheets() const;
	/**
	 * Returns the index of the current stunt sheet.
	 * @return The index of the current stunt sheet.
	 */
	int GetCurrentSheet() const;
	/**
	 * Returns the number of beats in the current sheet.
	 * @return The number of beats in the current sheet.
	 */
	int GetNumberBeats() const;
	/**
	 * Returns the current beat that the animation is animating for the
	 * current sheet.
	 * @return The current beat of the stuntsheet.
	 */
	int GetCurrentBeat() const;
	std::string GetCurrentSheetName() const;

	// collection of position of each point, for debugging purposes
	std::pair<std::string, std::vector<std::string> > GetCurrentInfo() const;

	std::vector<CC_DrawCommand> GenPathToDraw(unsigned point, const CC_coord& offset) const;
	AnimatePoint EndPosition(unsigned point, const CC_coord& offset) const;

private:
	/**
	 * The number of points in the animation.
	 */
	const unsigned numpts;
	/**
	 * A list of the points in the animation.
	 */
	std::vector<AnimatePoint> pts;
	/**
	 * A list of the commands that will be used to move the points to the
	 * next beat of the show.
	 */
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> >::const_iterator > curr_cmds; // pointer to the current command
	std::set<int> mCollisions;
	/**
	 * The index of the stuntsheet currently being animated.
	 */
	unsigned curr_sheetnum;
	/**
	 * The current beat being animated of the current stuntsheet.
	 */
	unsigned curr_beat;

	void BeginCmd(unsigned i);

	void EndCmd(unsigned i);

	/**
	 * Resets the animation to the beginning of the current sheet.
	 * This also makes sure that the point positions and current commands are
	 * appropriate for the beginning of this sheet, so this effectively
	 * activates the stunt sheet.
	 */
	void RefreshSheet();

	/**
	 * A list containing all animation commands for the entire show, organized
	 * by the stuntsheets that they are associated with.
	 */
	std::vector<AnimateSheet> sheets;
	
	void CheckCollisions();
	/**
	 * Records how the Animation Frame should respond to collisions that occur
	 * while the show is animating.
	 */
	CollisionAction_t mCollisionAction;

	/**
	 * Returns the list of commands that a particular point will follow as it
	 * steps through the show.
	 * @param whichPoint The index representing the point to get the
	 * commands for.
	 * @return The list of commands that the point will follow as it steps
	 * through the show.
	 */
	std::vector<boost::shared_ptr<AnimateCommand> > GetCommands(unsigned whichPoint) const;
};

#endif
