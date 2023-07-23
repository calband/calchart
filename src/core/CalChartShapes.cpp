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

#include "CalChartShapes.h"
#include "CalChartDrawCommand.h"
#include <cmath>
#include <numeric>

namespace CalChart {

auto Shape_crosshairs::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Line(GetOrigin().x - crosshairs_width, GetOrigin().y - crosshairs_width, GetOrigin().x + crosshairs_width, GetOrigin().y + crosshairs_width),
        Draw::Line(GetOrigin().x + crosshairs_width, GetOrigin().y - crosshairs_width, GetOrigin().x - crosshairs_width, GetOrigin().y + crosshairs_width),
    };
}

auto Shape_2point::GetPolygon() const -> RawPolygon_t
{
    return {
        Coord(GetOrigin().x, GetOrigin().y),
        Coord(GetPoint().x, GetOrigin().y),
        Coord(GetPoint().x, GetPoint().y),
        Coord(GetOrigin().x, GetPoint().y),
    };
}

auto Shape_line::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Line(GetOrigin(), GetPoint())
    };
}

auto Shape_x::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Line(GetOrigin(), GetPoint()),
        Draw::Line(GetPoint().x, GetOrigin().y, GetOrigin().x, GetPoint().y),
    };
}

auto Shape_cross::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Line(GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2, GetOrigin().y, GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2, GetPoint().y),
        Draw::Line(GetOrigin().x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2, GetPoint().x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2),
    };
}

auto Shape_ellipse::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Ellipse(GetOrigin(), GetPoint())
    };
}

auto Shape_arc::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    auto boundingAngle = angle0 < CalChart::Radian{} ? angle - angle0 : angle0 - angle;
    if (boundingAngle < CalChart::Radian{} || boundingAngle > CalChart::pi) {
        return {
            Draw::Arc(
                GetOrigin() + CalChart::CreateCoordVector(angle0 < CalChart::Radian{} ? angle : angle0, d),
                GetOrigin() + CalChart::CreateCoordVector(angle0 < CalChart::Radian{} ? angle0 : angle, d),
                GetOrigin())
        };
    }
    return {
        Draw::Arc(
            GetOrigin() + CalChart::CreateCoordVector(angle0 < CalChart::Radian{} ? angle0 : angle, d),
            GetOrigin() + CalChart::CreateCoordVector(angle0 < CalChart::Radian{} ? angle : angle0, d),
            GetOrigin())
    };
}

auto Shape_rect::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    return {
        Draw::Line(GetOrigin().x, GetOrigin().y, GetPoint().x, GetOrigin().y),
        Draw::Line(GetPoint().x, GetOrigin().y, GetPoint().x, GetPoint().y),
        Draw::Line(GetPoint().x, GetPoint().y, GetOrigin().x, GetPoint().y),
        Draw::Line(GetOrigin().x, GetPoint().y, GetOrigin().x, GetOrigin().y),
    };
}

auto Lasso::GetPointsOnLine(int numpnts) const -> std::vector<Coord>
{
    if (numpnts < 1 || pntlist.empty()) {
        return {};
    }
    std::vector<Coord> results;
    results.reserve(numpnts);
    results.push_back(pntlist.front());
    if (numpnts < 2) {
        return results;
    }

    const auto each_segment = GetDistance(GetPolygon()) / (numpnts - 1);
    auto iter = pntlist.begin();
    auto curr_pnt = *iter++;
    auto curr_dist = iter->Distance(curr_pnt);
    auto running_dist = each_segment;
    while (iter != pntlist.end()) {
        // emergency rip chord on rounding cases
        if (static_cast<int>(results.size()) == numpnts) {
            break;
        }
        // if the distance to the next point is more than we need, use it up.
        if (running_dist > curr_dist) {
            running_dist -= curr_dist;
            curr_pnt = *iter++;
            curr_dist = iter->Distance(curr_pnt);
        } else {
            // the new point will be along the vector by
            auto dist_vector = (*iter - curr_pnt);
            auto factor = (running_dist / curr_dist);
            dist_vector *= factor;
            auto new_pnt = curr_pnt + dist_vector;

            // we found a place for this point.
            results.push_back(new_pnt);

            curr_pnt = new_pnt;
            curr_dist = iter->Distance(curr_pnt);
            running_dist = each_segment;
        }
    }
    // emergency rip chord on rounding cases
    while (static_cast<int>(results.size()) < numpnts) {
        results.push_back(pntlist.back());
    }
    return results;
}

auto Lasso::GetCC_DrawCommand() const -> std::vector<DrawCommand>
{
    std::vector<DrawCommand> result;
    if (pntlist.size() > 1) {
        for (auto iter = 0U; iter < pntlist.size() - 1; ++iter) {
            result.emplace_back(Draw::Line(pntlist.at(iter), pntlist.at(iter + 1)));
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

}
