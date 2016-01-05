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

#define _USE_MATH_DEFINES
#include <cmath>

#include "cc_shapes.h"
#include "cc_drawcommand.h"

void CC_shape_1point::OnMove(const CC_coord& p, const CC_coord&)
{
    MoveOrigin(p);
}

void CC_shape_cross::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    MoveOrigin(snapped_p);
}

std::vector<CC_DrawCommand> CC_shape_cross::GetCC_DrawCommand(float x,
    float y) const
{
    std::vector<CC_DrawCommand> result;
    result.emplace_back(origin.x + x - cross_width, origin.y + y - cross_width,
        origin.x + x + cross_width, origin.y + y + cross_width);
    result.emplace_back(origin.x + x + cross_width, origin.y + y - cross_width,
        origin.x + x - cross_width, origin.y + y + cross_width);
    return result;
}

void CC_shape_2point::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    MovePoint(p);
}

void CC_shape_line::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    MovePoint(snapped_p);
}

std::vector<CC_DrawCommand> CC_shape_line::GetCC_DrawCommand(float x,
    float y) const
{
    std::vector<CC_DrawCommand> result;
    result.emplace_back(origin.x + x, origin.y + y, point.x + x, point.y + y);
    return result;
}

void CC_shape_angline::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    auto o = GetOrigin();
    auto p1 = snapped_p - o;
    auto d = (Coord2Float(p1.x) * Coord2Float(vect.x) + Coord2Float(p1.y) * Coord2Float(vect.y)) / (mag * mag);
    p1.x = (Coord)(o.x + vect.x * d);
    p1.y = (Coord)(o.y + vect.y * d);
    MovePoint(p1);
}

CC_shape_arc::CC_shape_arc(const CC_coord& c, const CC_coord& p)
    : CC_shape_2point(c, p)
{
    auto p1 = p - c;

    r = r0 = p1.Direction() * M_PI / 180.0;
    d = p1.Magnitude() * COORD_DECIMAL;
}

CC_shape_arc::CC_shape_arc(const CC_coord& c, const CC_coord& p1,
    const CC_coord& p2)
    : CC_shape_2point(c, p2)
{
    auto p = p1 - c;

    r0 = p.Direction();
    d = p.Magnitude() * COORD_DECIMAL;
    r = (p2 - c).Direction();
}

void CC_shape_arc::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    auto p1 = snapped_p;

    r = GetOrigin().Direction(p1) * M_PI / 180.0;
    p1.x = Coord(origin.x + d * cos(r));
    p1.y = Coord(origin.y + -d * sin(r));
    MovePoint(p1);
}

std::vector<CC_DrawCommand> CC_shape_arc::GetCC_DrawCommand(float x,
    float y) const
{
    std::vector<CC_DrawCommand> result;
    if (GetAngle() < 0.0 || GetAngle() > 180.0) {
        result.emplace_back(origin.x + x + d * cos(r), origin.y + y + -d * sin(r),
            origin.x + x + d * cos(r0), origin.y + y + -d * sin(r0),
            origin.x + x, origin.y + y);
    }
    else {
        result.emplace_back(origin.x + x + d * cos(r0), origin.y + y + -d * sin(r0),
            origin.x + x + d * cos(r), origin.y + y + -d * sin(r),
            origin.x + x, origin.y + y);
    }
    return result;
}

std::vector<CC_DrawCommand> CC_shape_rect::GetCC_DrawCommand(float x,
    float y) const
{
    float w, h;

    if (origin.x < point.x) {
        x += origin.x;
        w = point.x - origin.x + 1;
    }
    else {
        x += point.x;
        w = origin.x - point.x + 1;
    }
    if (origin.y < point.y) {
        y += origin.y;
        h = point.y - origin.y + 1;
    }
    else {
        y += point.y;
        h = origin.y - point.y + 1;
    }
    std::vector<CC_DrawCommand> result;
    if ((w > 1) && (h > 1)) {
        result.emplace_back(x, y, x + w, y);
        result.emplace_back(x + w, y, x + w, y + h);
        result.emplace_back(x + w, y + h, x, y + h);
        result.emplace_back(x, y + h, x, y);
    }
    return result;
}

CC_lasso::CC_lasso(const CC_coord& p) { Append(p); }

void CC_lasso::OnMove(const CC_coord& p, const CC_coord&) { Append(p); }

void CC_lasso::Clear() { pntlist.clear(); }

void CC_lasso::Start(const CC_coord& p)
{
    Clear();
    Append(p);
}

// Closes polygon
void CC_lasso::End()
{
    if (!pntlist.empty()) {
        pntlist.push_back(pntlist[0]);
    }
}

void CC_lasso::Append(const CC_coord& p) { pntlist.push_back(p); }

// Test if inside polygon using odd-even rule
bool CC_lasso::Inside(const CC_coord& p) const
{
    bool parity = false;
    if (pntlist.size() < 2) {
        return parity;
    }
    for (auto prev = pntlist.begin(), next = prev + 1; next != pntlist.end();
         ++prev, ++next) {
        if (CrossesLine(*prev, *next, p)) {
            parity = !parity;
        }
    }
    // don't forget the first one:
    if (CrossesLine(pntlist.back(), pntlist.front(), p)) {
        parity = !parity;
    }

    return parity;
}

std::vector<CC_DrawCommand> CC_lasso::GetCC_DrawCommand(float x,
    float y) const
{
    std::vector<CC_DrawCommand> result;
    if (pntlist.size() > 1) {
        for (auto iter = 0u; iter < pntlist.size() - 1; ++iter) {
            result.push_back(CC_DrawCommand(
                x + pntlist.at(iter).x, y + pntlist.at(iter).y,
                x + pntlist.at(iter + 1).x, y + pntlist.at(iter + 1).y));
        }
    }
    return result;
}

void CC_lasso::Drag(const CC_coord& p)
{
    if (!pntlist.empty()) {
        pntlist.back() = p;
    }
}

bool CC_lasso::CrossesLine(const CC_coord& start, const CC_coord& end,
    const CC_coord& p)
{
    if (start.y > end.y) {
        if (!((p.y <= start.y) && (p.y > end.y))) {
            return false;
        }
    }
    else {
        if (!((p.y <= end.y) && (p.y > start.y))) {
            return false;
        }
    }
    if (p.x >= ((end.x - start.x) * (p.y - start.y) / (end.y - start.y) + start.x)) {
        return true;
    }
    return false;
}

CC_poly::CC_poly(const CC_coord& p)
    : CC_lasso(p)
{
    // add end point
    Append(p);
}

void CC_poly::OnMove(const CC_coord& p, const CC_coord&) { Drag(p); }
