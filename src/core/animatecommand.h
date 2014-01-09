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

#ifndef _ANIMATECOMMAND_H_
#define _ANIMATECOMMAND_H_

#include "animate.h"
#include "cc_drawcommand.h"

/**
 * A command that modifies a point, pushing it beat by beat through an
 * animation.
 */
class AnimateCommand
{
public:
	/**
	 * Makes the command.
	 * @param beats The duration of the command, in beats.
	 */
	AnimateCommand(unsigned beats);
	/**
	 * Cleanup.
	 */
	virtual ~AnimateCommand();


	/**
	 * Resets the command to its beginning.
	 * @param pt The point to apply the command to.
	 * @return False if the command is finished, true otherwise.
	 */
	virtual bool Begin(AnimatePoint& pt);
	/**
	 * Rests the command to its end.
	 * @param pt The point to apply the command to.
	 * @return False if the command is finished, true otherwise.
	 */
	virtual bool End(AnimatePoint& pt);
	/**
	 * Pushes the command to its next beat.
	 * @param pt The point to apply the command to.
	 * @return False if the command is finished, true otherwise.
	 */
	virtual bool NextBeat(AnimatePoint& pt);
	/**
	 * Pushes the command back to its previous beat.
	 * @param pt The point to apply the command to.
	 * @return False if the command is finished, true otherwise.
	 */
	virtual bool PrevBeat(AnimatePoint& pt);

	/**
	 * Applies the command forward, in its entirety.
	 * @param pt The point to apply the command to.
	 */
	virtual void ApplyForward(AnimatePoint& pt);
	/**
	* Applies the command backward, in its entirety.
	* @param pt The point to apply the command to.
	*/
	virtual void ApplyBackward(AnimatePoint& pt);

	/**
	 * Returns the direction that a point would face during the
	 * current beat, if executing this command.
	 * @return The direction that a point executing this command
	 * should be facing.
	 */
	virtual AnimateDir Direction() const = 0;
	/**
	 * Returns the angle which a point would face on the current
	 * beat, if executing this command. The angle is measured
	 * in degrees, counter-clockwise from the positive x-axis.
	 * @return The angle at which a point executing this
	 * command should be facing.
	 */
	virtual float RealDirection() const = 0;
	/**
	* Returns the angle at which a point would be travelling on the
	* current beat, if executing this command. The angle is measured
	* in degrees, counter-clockwise from the positive x-axis.
	* @return The angle in which a point executing this
	* command should be traveling.
	*/
	virtual float MotionDirection() const;
	/**
	 * Sets the duration of the command, in beats.
	 * @param beats The new duration for the command.
	 */
	virtual void ClipBeats(unsigned beats);

	/**
	 * Returns the duration of the command, in beats.
	 */
	virtual unsigned NumBeats() const { return mNumBeats; }

	/**
	 * Returns the marching style that points executing this command should
	 * exhibit.
	 * @return The marching style of points executing this command.
	 */
	virtual MarchingStyle StepStyle() { return STYLE_HighStep; }

	/**
	 * Returns a command that can be used to draw a visual representation
	 * of this command.
	 * @param pt The point that is executing the command.
	 * @param offset The offset of the drawing.
	 */
	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const { return CC_DrawCommand(); }

protected:
	/**
	 * The duration of the command, in beats.
	 */
	unsigned mNumBeats;
	/**
	 * The number of beats elapsed since the beginning of the command.
	 */
	unsigned mBeat;
};

/**
 * A command that causes animated points to mark time.
 */
class AnimateCommandMT : public AnimateCommand
{
public:
	/**
	 * Makes the command.
	 * @param beats The duration of the mark time, in beats.
	 * @param direction The angle at which points should
	 * face while marking time. The angle is measured in
	 * degrees, and counter-clockwise with respect to the
	 * positive x-axis.
	 */
	AnimateCommandMT(unsigned beats, float direction);
	/**
	 * Cleanup.
	 */
	virtual ~AnimateCommandMT() {}

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
protected:
	/**
	 * The direction in which the points should face
	 * while marking time.
	 */
	AnimateDir dir;
	/**
	 * The angle in which the points should face while
	 * marking time.
	 */
	float realdir;
};

/**
 * A command used to make animated points move.
 */
class AnimateCommandMove : public AnimateCommandMT
{
public:
	/**
	 * Makes the command.
	 * @param beats The duration of the command, in beats.
	 * @param movement A vector representing the movement that
	 * a point should undergo (direction and distance) while
	 * progressing though this command.
	 */
	AnimateCommandMove(unsigned beats, CC_coord movement);
	/**
	 * Makes the command.
	 * @param beats The duration of the command, in beats.
	 * @param movement A vector representing the movement that
	 * a point should undergo (direction and distance) while
	 * progressing though this command.
	 * @param The angle at which points should face while moving.
	 */
	AnimateCommandMove(unsigned beats, CC_coord movement, float direction);
	/**
	 * Cleanup.
	 */
	virtual ~AnimateCommandMove() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual float MotionDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const;

private:
	/**
	 * The vector representing the total change in position that will be
	 * applied to a point as it progresses through the command.
	 */
	CC_coord mVector;
};

/**
 * A command that causes animated points to move in an arc at a radius
 * from some central point.
 */
class AnimateCommandRotate : public AnimateCommand
{
public:
	//TODO counter clockwise - is that accurate?
	/**
	 * Makes the command.
	 * @param beats The duration of the movement, in beats.
	 * @param cntr The center of the arc.
	 * @param rad The radius at which the arc should be traversed.
	 * @param ang1 The angle at which the arc starts, measured
	 * in degrees counter-clockwise from the positive x-axis.
	 * @param ang2 The angle at which the arc ends, measured
	 * in degrees counter-clockwise from the positive x-axis.
	 * @param backwards TODO used to define whether points look
	 * in toward center of arc or out away toward center
	 */
	AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		float ang1, float ang2, bool backwards = false);
	/**
	 * Cleanup.
	 */
	virtual ~AnimateCommandRotate() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const;

private:
	/**
	 * The center of the arc path.
	 */
	CC_coord mOrigin;
	/**
	 * The radius at which the arc path is drawn.
	 */
	float mR;
	/**
	 * The angle at which the arc starts.
	 */
	float mAngStart;
	/**
	 * The angle at which the arc ends.
	 */
	float mAngEnd;
	/**
	 * The angle at which the dots traveling along the
	 * arc should face, relative to the direction of travel.
	 */
	float mFace;
};

#endif //_ANIMATECOMMAND_H_
