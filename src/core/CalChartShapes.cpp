/*
 * CalChartShapes.cpp
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
#include <numeric>

#include "CalChartDrawCommand.h"
#include "CalChartShapes.h"

namespace CalChart {

static bool CrossesLine(Coord start, Coord end, Coord p)
{
    if (start.y > end.y) {
        if (!((p.y <= start.y) && (p.y > end.y))) {
            return false;
        }
    } else {
        if (!((p.y <= end.y) && (p.y > start.y))) {
            return false;
        }
    }
    if (p.x >= ((end.x - start.x) * (p.y - start.y) / (end.y - start.y) + start.x)) {
        return true;
    }
    return false;
}

// Test if inside polygon using odd-even rule
bool Inside(Coord p, RawPolygon_t const& polygon)
{
    auto parity = false;
    if (polygon.size() < 2) {
        return parity;
    }
    for (auto prev = polygon.begin(), next = prev + 1; next != polygon.end();
         ++prev, ++next) {
        if (CrossesLine(*prev, *next, p)) {
            parity = !parity;
        }
    }
    // don't forget the first one:
    if (CrossesLine(polygon.back(), polygon.front(), p)) {
        parity = !parity;
    }

    return parity;
}

void Shape_1point::OnMove(Coord p, Coord)
{
    MoveOrigin(p);
}

void Shape_crosshairs::OnMove(Coord, Coord snapped_p)
{
    MoveOrigin(snapped_p);
}

std::vector<DrawCommand> Shape_crosshairs::GetCC_DrawCommand(float x, float y) const
{
    return {
        DrawCommands::Line(origin.x + x - crosshairs_width, origin.y + y - crosshairs_width, origin.x + x + crosshairs_width, origin.y + y + crosshairs_width),
        DrawCommands::Line(origin.x + x + crosshairs_width, origin.y + y - crosshairs_width, origin.x + x - crosshairs_width, origin.y + y + crosshairs_width),
    };
}

void Shape_2point::OnMove(Coord p, Coord)
{
    MovePoint(p);
}

std::optional<RawPolygon_t> Shape_2point::GetPolygon() const
{
    return { {
        Coord(origin.x, origin.y),
        Coord(point.x, origin.y),
        Coord(point.x, point.y),
        Coord(origin.x, point.y),
    } };
}

void Shape_line::OnMove(Coord, Coord snapped_p)
{
    MovePoint(snapped_p);
}

std::vector<DrawCommand> Shape_line::GetCC_DrawCommand(float x, float y) const
{
    return {
        DrawCommands::Line(origin.x + x, origin.y + y, point.x + x, point.y + y)
    };
}

void Shape_x::OnMove(Coord, Coord snapped_p)
{
    MovePoint(snapped_p);
}

std::vector<DrawCommand> Shape_x::GetCC_DrawCommand(float x, float y) const
{
    return {
        DrawCommands::Line(origin.x + x, origin.y + y, point.x + x, point.y + y),
        DrawCommands::Line(point.x + x, origin.y + y, origin.x + x, point.y + y),
    };
}

void Shape_cross::OnMove(Coord, Coord snapped_p)
{
    MovePoint(snapped_p);
}

std::vector<DrawCommand> Shape_cross::GetCC_DrawCommand(float x, float y) const
{
    return {
        DrawCommands::Line(origin.x + (point.x - origin.x) / 2 + x, origin.y + y, origin.x + (point.x - origin.x) / 2 + x, point.y + y),
        DrawCommands::Line(origin.x + x, origin.y + (point.y - origin.y) / 2 + y, point.x + x, origin.y + (point.y - origin.y) / 2 + y),
    };
}

void Shape_ellipse::OnMove(Coord, Coord snapped_p)
{
    MovePoint(snapped_p);
}

std::vector<DrawCommand> Shape_ellipse::GetCC_DrawCommand(float x, float y) const
{
    return {
        DrawCommands::Ellipse(origin.x + x, origin.y + y, point.x + x, point.y + y)
    };
}

void Shape_angline::OnMove(Coord, Coord snapped_p)
{
    auto o = GetOrigin();
    auto p1 = snapped_p - o;
    auto d = (CoordUnits2Float(p1.x) * CoordUnits2Float(vect.x) + CoordUnits2Float(p1.y) * CoordUnits2Float(vect.y)) / (mag * mag);
    p1.x = (Coord::units)(o.x + vect.x * d);
    p1.y = (Coord::units)(o.y + vect.y * d);
    MovePoint(p1);
}

Shape_arc::Shape_arc(Coord c, Coord p)
    : Shape_2point(c, p)
{
    auto p1 = p - c;

    r = r0 = p1.Direction() * M_PI / 180.0;
    d = p1.Magnitude() * kCoordDecimal;
}

Shape_arc::Shape_arc(Coord c, Coord p1, Coord p2)
    : Shape_2point(c, p2)
{
    auto p = p1 - c;

    r0 = p.Direction();
    d = p.Magnitude() * kCoordDecimal;
    r = (p2 - c).Direction();
}

void Shape_arc::OnMove(Coord, Coord snapped_p)
{
    auto p1 = snapped_p;

    r = GetOrigin().Direction(p1) * M_PI / 180.0;
    p1.x = Coord::units(origin.x + d * cos(r));
    p1.y = Coord::units(origin.y + -d * sin(r));
    MovePoint(p1);
}

std::vector<DrawCommand> Shape_arc::GetCC_DrawCommand(float x, float y) const
{
    if (GetAngle() < 0.0 || GetAngle() > 180.0) {
        return {
            DrawCommands::Arc(
                origin.x + x + d * cos(r),
                origin.y + y + -d * sin(r),
                origin.x + x + d * cos(r0),
                origin.y + y + -d * sin(r0),
                origin.x + x,
                origin.y + y)
        };
    }
    return {
        DrawCommands::Arc(
            origin.x + x + d * cos(r0),
            origin.y + y + -d * sin(r0),
            origin.x + x + d * cos(r),
            origin.y + y + -d * sin(r),
            origin.x + x,
            origin.y + y)
    };
}

std::vector<DrawCommand> Shape_rect::GetCC_DrawCommand(float x, float y) const
{
    float w, h;

    if (origin.x < point.x) {
        x += origin.x;
        w = point.x - origin.x + 1;
    } else {
        x += point.x;
        w = origin.x - point.x + 1;
    }
    if (origin.y < point.y) {
        y += origin.y;
        h = point.y - origin.y + 1;
    } else {
        y += point.y;
        h = origin.y - point.y + 1;
    }
    if ((w > 1) && (h > 1)) {
        return {
            DrawCommands::Line(x, y, x + w, y),
            DrawCommands::Line(x + w, y, x + w, y + h),
            DrawCommands::Line(x + w, y + h, x, y + h),
            DrawCommands::Line(x, y + h, x, y),
        };
    }
    return {};
}

Lasso::Lasso(Coord p) { Append(p); }

void Lasso::OnMove(Coord p, Coord) { Append(p); }

void Lasso::Clear() { pntlist.clear(); }

void Lasso::Start(Coord p)
{
    Clear();
    Append(p);
}

// Closes polygon
void Lasso::End()
{
    if (!pntlist.empty()) {
        pntlist.push_back(pntlist[0]);
    }
}

void Lasso::Append(Coord p) { pntlist.push_back(p); }

float Lasso::GetDistance() const
{
    if (pntlist.empty()) {
        return {};
    }
    auto iter = pntlist.begin();
    auto curr_pnt = *iter++;
    auto total = 0.0f;
    while (iter != pntlist.end()) {
        total += std::hypot(iter->x - curr_pnt.x, iter->y - curr_pnt.y);
        curr_pnt = *iter++;
    }
    return total;
}

std::vector<Coord> Lasso::GetPointsOnLine(int numpnts) const
{
    if (numpnts < 1 || pntlist.size() < 1) {
        return {};
    }
    std::vector<Coord> results;
    results.push_back(pntlist.front());
    if (numpnts < 2) {
        return results;
    }

    if (pntlist.size() < 2) {
        return std::vector<Coord>(numpnts, *FirstPoint());
    }

    const auto each_segment = GetDistance() / (numpnts - 1);
    auto iter = pntlist.begin();
    auto curr_pnt = *iter++;
    auto curr_dist = std::hypot(iter->x - curr_pnt.x, iter->y - curr_pnt.y);
    auto running_dist = each_segment;
    while (iter != pntlist.end()) {
        if (running_dist > curr_dist) {
            running_dist -= curr_dist;
            curr_pnt = *iter++;
            curr_dist = std::hypot(iter->x - curr_pnt.x, iter->y - curr_pnt.y);
        } else {
            auto dist_vector = (*iter - curr_pnt);
            auto factor = (running_dist / curr_dist);
            dist_vector *= factor;
            auto new_pnt = curr_pnt + dist_vector;

            results.push_back(new_pnt);
            // emergency rip chord on rounding cases
            if (static_cast<int>(results.size()) == numpnts) {
                return results;
            }
            curr_pnt = new_pnt;
            curr_dist = std::hypot(iter->x - curr_pnt.x, iter->y - curr_pnt.y);
            running_dist = each_segment;
        }
    }
    // emergency rip chord on rounding cases
    while (static_cast<int>(results.size()) < numpnts) {
        results.push_back(pntlist.back());
    }
    return results;
}

std::vector<DrawCommand> Lasso::GetCC_DrawCommand(float x, float y) const
{
    std::vector<DrawCommand> result;
    if (pntlist.size() > 1) {
        for (auto iter = 0u; iter < pntlist.size() - 1; ++iter) {
            result.emplace_back(
                DrawCommands::Line(
                    x + pntlist.at(iter).x, y + pntlist.at(iter).y,
                    x + pntlist.at(iter + 1).x, y + pntlist.at(iter + 1).y));
        }
    }
    return result;
}

void Lasso::Drag(Coord p)
{
    if (!pntlist.empty()) {
        pntlist.back() = p;
    }
}

Poly::Poly(Coord p)
    : Lasso(p)
{
    // add end point
    Append(p);
}

void Poly::OnMove(Coord p, Coord) { Drag(p); }
}
