/*
 * cc_shapes.h
 * Header for calchart selector shapes
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

#ifndef _CC_SHAPES_H_
#define _CC_SHAPES_H_

#include "cc_coord.h"
#include <vector>

struct CC_DrawCommand;

/**
 * A shape that can be drawn via CC_DrawCommand objects.
 */
class CC_shape
{
public:
	/**
	 * Makes the shape.
	 */
	CC_shape();
	/**
	 * Cleanup.
	 */
	virtual ~CC_shape();

	/**
	 * Returns a list of draw commands that, when executed, will draw the shape.
	 * @param x The x position to treat as the origin when drawing the shape.
	 * @param y The y position to treat as the origin when drawing the shape.
	 * @return A list of draw commands that, when executed, will draw the shape.
	 */
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const = 0;
	/**
	 * Called when the mouse is "dragged" to a new position. Whether or not
	 * the mouse was dragged depends on the current drag mode of the field
	 * frame.
	 * @param p The position to which the mouse was dragged.
	 * @param snapped_p The position to which the mouse was dragged, snapped
	 * to the grid that is currently active for the Field Frame.
	 */
	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) = 0;
};

/**
 * A shape that uses at one point to define its figure.
 */
class CC_shape_1point: public CC_shape
{
public:
	/**
	 * Makes the shape.
	 * @param p The point that defines the shape.
	 */
	CC_shape_1point(const CC_coord& p);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	/**
	 * Moves the point that defines the shape.
	 * @param p The position to which the origin point of this shape should
	 * be moved.
	 */
	void MoveOrigin(const CC_coord& p);
	/**
	 * Returns the position of the point that defines the shape (its origin).
	 * @return The origin of the shape.
	 */
	CC_coord GetOrigin() const;

protected:
	/**
	 * The origin of the shape - that is, the point that defines its figure.
	 */
	CC_coord origin;
};

/**
 * A cross shape, where the center of the cross is at an origin point,
 * and the hairs branch off in the positive/negative x and y directions
 * for a certain distance.
 */
class CC_shape_cross: public CC_shape_1point
{
public:
	/**
	 * Makes the shape.
	 * @param p The origin of the cross.
	 * @param width The length of the hairs of the cross.
	 */
	CC_shape_cross(const CC_coord& p, Coord width);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const;

protected:
	/**
	 * The length of the hairs of the cross.
	 */
	Coord cross_width;
};

/**
 * A shape whose figure is defined by two points.
 */
class CC_shape_2point: public CC_shape_1point
{
public:
	/**
	 * Makes the shape.
	 * @param p The position to use for both points that define the shape.
	 */
	CC_shape_2point(const CC_coord& p);
	/**
	 * Makes the shape.
	 * @param p1 The first point of the shape.
	 * @param p2 The second point of the shape.
	 */
	CC_shape_2point(const CC_coord& p1, const CC_coord& p2);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	/**
	 * Moves the second point (the non-origin point) of the shape to a
	 * particular location.
	 * @param p The position to which to move the non-origin point.
	 */
	void MovePoint(const CC_coord& p);
	/**
	 * Returns the second point (the non-origin point) of the shape.
	 * @return The non-origin point of the shape.
	 */
	CC_coord GetPoint() const;

protected:
	/**
	 * The second point that defines the shape.
	 */
	CC_coord point;
};

/**
 * A line shape.
 */
class CC_shape_line: public CC_shape_2point
{
public:
	/**
	 * Makes a degenerate line (a point).
	 * @param p The position of the point.
	 */
	CC_shape_line(const CC_coord& p);
	/**
	 * Makes a line between two points.
	 * @param p1 The first point.
	 * @param p2 The second point.
	 */
	CC_shape_line(const CC_coord& p1, const CC_coord& p2);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const;
};

//TODO One unit up? Is that accurate?
/**
 * Makes a line where the second point is repositioned depending on how far the
 * mouse is dragged from the origin point.
 */
class CC_shape_angline: public CC_shape_line
{
public:
	/**
	 * Makes the line.
	 * @param p The starting point for both points on the line.
	 * @param refvect The direction in which the second point of
	 * the line should be moved when the mouse is dragged one unit
	 * to the right and one unit up.
	 */
	CC_shape_angline(const CC_coord& p, const CC_coord& refvect);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
private:
	/**
	 * The direction in which to move the second point of the line
	 * when the mouse is dragged one unit to the right and one unit up.
	 */
	CC_coord vect;
	/**
	 * The magnitude of vect, used to normalize the direction in which
	 * the second point should be moved when the mouse is dragged.
	 */
	float mag;
};

/**
 * An arc shape.
 */
class CC_shape_arc: public CC_shape_2point
{
public:
	/**
	 * Makes the arc, with no angular span.
	 * @param c The center of the arc.
	 * @param p A point on the arc, at a radius away from the center point.
	 */
	CC_shape_arc(const CC_coord& c, const CC_coord& p);
	/**
	 * Makes an arc.
	 * @param c The center of the arc.
	 * @param p A point on the arc, at a radius away from the center point.
	 * The arc will begin at this point.
	 * @param p2 A point in the direction, relative to the center point, where
	 * the arc will end.
	 */
	CC_shape_arc(const CC_coord& c, const CC_coord& p1, const CC_coord& p2);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const;

	/**
	 * Returns the angle that the arc spans. The angle will be negative if
	 * the arc spins counter-clockwise. The angle is measured in radians.
	 * @return The angle that the arc spans.
	 */
	inline float GetAngle() const { return r-r0; }
private:
	/* 
	 * The angle, in radians, of the start of the arc.
	 * The angle is measured counter-clockwise from the positive x-axis.
	 */
	float r;
	/*
	 * The angle, in radians, of the end of the arc.
	 * The angle is measured counter-clockwise from the positive x-axis.
	 */
	float r0;
	/**
	 * The radius of the arc.
	 */
	float d;
};

/**
 * A rectangle.
 */
class CC_shape_rect: public CC_shape_2point
{
public:
	/**
	 * Makes a degenerate rectangle (a point).
	 * @param p The point.
	 */
	CC_shape_rect(const CC_coord& p);
	/**
	 * Makes a rectangle.
	 * @param p1 One corner of the rectangle.
	 * @param p2 The opposite corner of the rectangle.
	 */
	CC_shape_rect(const CC_coord& p1, const CC_coord& p2);

	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const;
};

/**
 * A lasso shape. The lasso has a starting point, but accumulates more points
 * along the path that the mouse is dragged.
 */
class CC_lasso: public CC_shape
{
public:
	/**
	 * Makes an incomplete lasso shape, to be completed by dragging the mouse.
	 * @param p The first point in the lasso.
	 */
	CC_lasso(const CC_coord& p);
	/**
	 * Cleanup.
	 */
	virtual ~CC_lasso();

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
	/**
	 * Clears all points that make up the lasso.
	 */
	void Clear();
	/**
	 * Restarts the lasso, giving it an initial point.
	 * @param The new first point for the lasso.
	 */
	void Start(const CC_coord& p);
	/**
	 * Completes the lasso, making it come around full-circle.
	 */
	void End();
	/**
	 * Adds a point to the lasso.
	 * @param p The point to add.
	 */
	void Append(const CC_coord& p);
	/**
	 * Checks whether or not a point is located within the loop of the lasso.
	 * @param p The point to check.
	 * @return True if the point is in the lasso; false otherwise.
	 */
	bool Inside(const CC_coord& p) const;
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const;
	/**
	 * Responds to a dragged mouse by adding points to the lasso.
	 * @param p The position of the mouse, after being dragged.
	 */
	void Drag(const CC_coord& p);
	/**
	 * Returns the first point of the lasso, or NULL, if no such point exists.
	 * @return The first point of the lasso, or NULL if none exists.
	 */
	inline const CC_coord *FirstPoint() const
	{
		return pntlist.empty() ? NULL : &pntlist.front();
	}
private:
	/**
	 * TODO kinda weird
	 */
	bool CrossesLine(const CC_coord& start, const CC_coord& end,
		const CC_coord& p) const;
	
	typedef std::vector<CC_coord> PointList;
	/**
	* The list of points that make up the lasso.
	*/
	PointList pntlist;
};

/**
 * A polygon shape. This is a lasso that only accumulates more points in its
 * figure only when the user clicks.
 */
class CC_poly: public CC_lasso
{
public:
	/**
	 * Makes a polygon.
	 * @param p The first point in the polygon.
	 */
	CC_poly(const CC_coord& p);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p);
};

#endif
