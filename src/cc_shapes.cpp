/*
 * cc_shapes.cpp
 * Implementation for calchart selector shapes
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

#include "cc_shapes.h"
#include "main_ui.h"
#include "platconf.h"

CC_shape::CC_shape() {}
CC_shape::~CC_shape() {}

CC_shape_1point::CC_shape_1point(const CC_coord& p)
: origin(p.x, p.y) {}

void CC_shape_1point::OnMove(const CC_coord& p, MainFrame *)
{
	MoveOrigin(p);
}


void CC_shape_1point::MoveOrigin(const CC_coord& p)
{
	origin.x = p.x;
	origin.y = p.y;
}


CC_coord CC_shape_1point::GetOrigin() const
{
	return CC_coord(Coord(origin.x), Coord(origin.y));
}


CC_shape_cross::CC_shape_cross(const CC_coord& p, Coord width)
: CC_shape_1point(p), cross_width(width) {}

void CC_shape_cross::OnMove(const CC_coord& p, MainFrame *frame)
{
	CC_coord p1 = p;

	frame->SnapToGrid(p1);
	MoveOrigin(p1);
}


void CC_shape_cross::Draw(wxDC *dc, float x, float y) const
{
	dc->DrawLine(origin.x + x - cross_width, origin.y + y - cross_width,
		origin.x + x + cross_width, origin.y + y + cross_width);
	dc->DrawLine(origin.x + x + cross_width, origin.y + y - cross_width,
		origin.x + x - cross_width, origin.y + y + cross_width);
}


CC_shape_2point::CC_shape_2point(const CC_coord& p)
: CC_shape_1point(p), point(p.x, p.y) {}

CC_shape_2point::CC_shape_2point(const CC_coord& p1, const CC_coord& p2)
: CC_shape_1point(p1), point(p2.x, p2.y) {}

void CC_shape_2point::OnMove(const CC_coord& p, MainFrame *)
{
	MovePoint(p);
}


void CC_shape_2point::MovePoint(const CC_coord& p)
{
	point.x = p.x;
	point.y = p.y;
}


CC_coord CC_shape_2point::GetPoint() const
{
	return CC_coord(Coord(point.x), Coord(point.y));
}


CC_shape_line::CC_shape_line(const CC_coord& p)
: CC_shape_2point(p) {}

CC_shape_line::CC_shape_line(const CC_coord& p1, const CC_coord& p2)
: CC_shape_2point(p1, p2) {}

void CC_shape_line::OnMove(const CC_coord& p, MainFrame *frame)
{
	CC_coord p1 = p;

	frame->SnapToGrid(p1);
	MovePoint(p1);
}


void CC_shape_line::Draw(wxDC *dc, float x, float y) const
{
	dc->DrawLine(origin.x + x, origin.y + y,
		point.x + x, point.y + y);
}


CC_shape_angline::CC_shape_angline(const CC_coord& p, const CC_coord& refvect)
: CC_shape_line(p), vect(refvect), mag(refvect.Magnitude())
{
}


void CC_shape_angline::OnMove(const CC_coord& p, MainFrame *frame)
{
	CC_coord o = GetOrigin();
	CC_coord p1 = p;
	float d;

	frame->SnapToGrid(p1);
	p1 -= o;
	d = (COORD2FLOAT(p1.x) * COORD2FLOAT(vect.x) +
		COORD2FLOAT(p1.y) * COORD2FLOAT(vect.y)) / (mag*mag);
	p1.x = (Coord)(o.x + vect.x * d);
	p1.y = (Coord)(o.y + vect.y * d);
	MovePoint(p1);
}


CC_shape_arc::CC_shape_arc(const CC_coord& c, const CC_coord& p)
: CC_shape_2point(c, p)
{
	CC_coord p1 = p-c;

	r = r0 = p1.Direction()*PI/180.0;
	d = p1.Magnitude()*COORD_DECIMAL;
}


CC_shape_arc::CC_shape_arc(const CC_coord& c,
const CC_coord& p1, const CC_coord& p2)
: CC_shape_2point(c, p2)
{
	CC_coord p = p1-c;

	r0 = p.Direction();
	d = p.Magnitude()*COORD_DECIMAL;
	r = (p2-c).Direction();
}


void CC_shape_arc::OnMove(const CC_coord& p, MainFrame *frame)
{
	CC_coord p1 = p;

	frame->SnapToGrid(p1);
	r = GetOrigin().Direction(p1)*PI/180.0;
	p1.x = Coord(origin.x + d*cos(r));
	p1.y = Coord(origin.y + -d*sin(r));
	MovePoint(p1);
}


void CC_shape_arc::Draw(wxDC *dc, float x, float y) const
{
// DrawArc always goes counterclockwise
	if (GetAngle() < 0.0 || GetAngle() > 180.0)
	{
		dc->DrawArc(origin.x + x + d*cos(r), origin.y + y + -d*sin(r),
			origin.x + x + d*cos(r0), origin.y + y + -d*sin(r0),
			origin.x + x, origin.y + y);
	}
	else
	{
		dc->DrawArc(origin.x + x + d*cos(r0), origin.y + y + -d*sin(r0),
			origin.x + x + d*cos(r), origin.y + y + -d*sin(r),
			origin.x + x, origin.y + y);
	}
}


CC_shape_rect::CC_shape_rect(const CC_coord& p)
: CC_shape_2point(p) {}

CC_shape_rect::CC_shape_rect(const CC_coord& p1, const CC_coord& p2)
: CC_shape_2point(p1, p2) {}

void CC_shape_rect::Draw(wxDC *dc, float x, float y) const
{
	float w, h;

	if (origin.x < point.x)
	{
		x += origin.x;
		w = point.x - origin.x + 1;
	}
	else
	{
		x += point.x;
		w = origin.x - point.x + 1;
	}
	if (origin.y < point.y)
	{
		y += origin.y;
		h = point.y - origin.y + 1;
	}
	else
	{
		y += point.y;
		h = origin.y - point.y + 1;
	}
	if ((w > 1) && (h > 1))
	{
		dc->DrawRectangle(x, y, w, h);
	}
}


CC_lasso::CC_lasso(const CC_coord &p)
{
	Append(p);
}


CC_lasso::~CC_lasso()
{
	Clear();
}


void CC_lasso::OnMove(const CC_coord& p, MainFrame *)
{
	Append(p);
}


void CC_lasso::Clear()
{
	pntlist.clear();
}


void CC_lasso::Start(const CC_coord& p)
{
	Clear();
	Append(p);
}


// Closes polygon
void CC_lasso::End()
{
	if (!pntlist.empty())
	{
		pntlist.push_back(pntlist[0]);
	}
}


void CC_lasso::Append(const CC_coord& p)
{
	pntlist.push_back(wxPoint(p.x, p.y));
}


// Test if inside polygon using odd-even rule
bool CC_lasso::Inside(const CC_coord& p) const
{
	bool parity = false;
	if (pntlist.size() < 2)
	{
		return parity;
	}
	for (PointList::const_iterator prev=pntlist.begin(), next=prev+1;
		next != pntlist.end();
		++prev, ++next)
	{
		if (CrossesLine(*prev, *next, p))
		{
			parity = !parity;
		}
	}
	// don't forget the first one:
	if (CrossesLine(pntlist.back(), pntlist.front(), p))
	{
		parity = !parity;
	}
	
	return parity;
}


void CC_lasso::Draw(wxDC *dc, float x, float y) const
{
	if (pntlist.size() > 1)
	{
		dc->DrawLines(pntlist.size(), const_cast<wxPoint*>(&pntlist[0]), x, y);
	}
}


void CC_lasso::Drag(const CC_coord& p)
{
	if (!pntlist.empty())
	{
		pntlist.back() = wxPoint(p.x, p.y);
	}
}


bool CC_lasso::CrossesLine(const wxPoint& start, const wxPoint& end,
const CC_coord& p) const
{
	if (start.y > end.y)
	{
		if (!((p.y <= start.y) && (p.y > end.y)))
		{
			return false;
		}
	}
	else
	{
		if (!((p.y <= end.y) && (p.y > start.y)))
		{
			return false;
		}
	}
	if (p.x >=
		((end.x-start.x) * (p.y-start.y) / (end.y-start.y) + start.x))
	{
		return true;
	}
	return false;
}


CC_poly::CC_poly(const CC_coord &p)
: CC_lasso(p)
{
// add end point
	Append(p);
}


void CC_poly::OnMove(const CC_coord& p, MainFrame *)
{
	Drag(p);
}

