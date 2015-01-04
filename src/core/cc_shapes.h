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

#pragma once

#include "cc_coord.h"
#include <vector>

struct CC_DrawCommand;

class CC_shape
{
public:
	virtual ~CC_shape() = default;

	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const = 0;
	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) = 0;
};

class CC_shape_1point: public CC_shape
{
public:
	CC_shape_1point(const CC_coord& p) : origin(p) {}

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	void MoveOrigin(const CC_coord& p) { origin = p; }
	CC_coord GetOrigin() const { return origin; }

protected:
	CC_coord origin;
};

class CC_shape_cross: public CC_shape_1point
{
public:
	CC_shape_cross(const CC_coord& p, Coord width) : CC_shape_1point(p), cross_width(width) {}

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const override;

protected:
	Coord cross_width;
};

class CC_shape_2point: public CC_shape_1point
{
public:
	CC_shape_2point(const CC_coord& p) : CC_shape_1point(p), point(p) {}
	CC_shape_2point(const CC_coord& p1, const CC_coord& p2) : CC_shape_1point(p1), point(p2) {}

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	void MovePoint(const CC_coord& p) { point = p; }
	CC_coord GetPoint() const { return point; }

protected:
	CC_coord point;
};

class CC_shape_line: public CC_shape_2point
{
public:
	CC_shape_line(const CC_coord& p) : CC_shape_2point(p) {}
	CC_shape_line(const CC_coord& p1, const CC_coord& p2) : CC_shape_2point(p1, p2) {}

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class CC_shape_angline: public CC_shape_line
{
public:
	CC_shape_angline(const CC_coord& p, const CC_coord& refvect) : CC_shape_line(p), vect(refvect), mag(refvect.Magnitude()) {}

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
private:
	CC_coord vect;
	float mag;
};

class CC_shape_arc: public CC_shape_2point
{
public:
	CC_shape_arc(const CC_coord& c, const CC_coord& p);
	CC_shape_arc(const CC_coord& c, const CC_coord& p1, const CC_coord& p2);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const override;

	inline float GetAngle() const { return r-r0; }
private:
	float r,r0,d;
};

class CC_shape_rect: public CC_shape_2point
{
public:
	CC_shape_rect(const CC_coord& p) : CC_shape_2point(p) {}
	CC_shape_rect(const CC_coord& p1, const CC_coord& p2) : CC_shape_2point(p1, p2) {}

	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class CC_lasso: public CC_shape
{
public:
	CC_lasso(const CC_coord& p);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
	void Clear();
	void Start(const CC_coord& p);
	void End();
	void Append(const CC_coord& p);
	bool Inside(const CC_coord& p) const;
	virtual std::vector<CC_DrawCommand> GetCC_DrawCommand(float x, float y) const override;
	void Drag(const CC_coord& p);
	inline const CC_coord *FirstPoint() const
	{
		return pntlist.empty() ? nullptr : &pntlist.front();
	}
private:
	static bool CrossesLine(const CC_coord& start, const CC_coord& end, const CC_coord& p);
	typedef std::vector<CC_coord> PointList;
	PointList pntlist;
};

class CC_poly: public CC_lasso
{
public:
	CC_poly(const CC_coord& p);

	virtual void OnMove(const CC_coord& p, const CC_coord& snapped_p) override;
};
