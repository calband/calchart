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

auto Shape_crosshairs::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    return {
        DrawCommands::Line(GetOrigin().x + p.x - crosshairs_width, GetOrigin().y + p.y - crosshairs_width, GetOrigin().x + p.x + crosshairs_width, GetOrigin().y + p.y + crosshairs_width),
        DrawCommands::Line(GetOrigin().x + p.x + crosshairs_width, GetOrigin().y + p.y - crosshairs_width, GetOrigin().x + p.x - crosshairs_width, GetOrigin().y + p.y + crosshairs_width),
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

auto Shape_line::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    return {
        DrawCommands::Line(GetOrigin() + p, GetPoint() + p)
    };
}

auto Shape_x::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    return {
        DrawCommands::Line(GetOrigin() + p, GetPoint() + p),
        DrawCommands::Line(GetPoint().x + p.x, GetOrigin().y + p.y, GetOrigin().x + p.x, GetPoint().y + p.y),
    };
}

auto Shape_cross::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    return {
        DrawCommands::Line(GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2 + p.x, GetOrigin().y + p.y, GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2 + p.x, GetPoint().y + p.y),
        DrawCommands::Line(GetOrigin().x + p.x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2 + p.y, GetPoint().x + p.x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2 + p.y),
    };
}

auto Shape_ellipse::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    return {
        DrawCommands::Ellipse(GetOrigin() + p, GetPoint() + p)
    };
}

auto Shape_arc::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    auto boundingAngle = angle0 < 0 ? angle - angle0 : angle0 - angle;
    if (boundingAngle < 0.0 || boundingAngle > std::numbers::pi) {
        return {
            DrawCommands::Arc(
                GetOrigin() + p + CalChart::CreateCoordVectorRad(angle0 < 0 ? angle : angle0, d),
                GetOrigin() + p + CalChart::CreateCoordVectorRad(angle0 < 0 ? angle0 : angle, d),
                GetOrigin() + p)
        };
    }
    return {
        DrawCommands::Arc(
            GetOrigin() + p + CalChart::CreateCoordVectorRad(angle0 < 0 ? angle0 : angle, d),
            GetOrigin() + p + CalChart::CreateCoordVectorRad(angle0 < 0 ? angle : angle0, d),
            GetOrigin() + p)
    };
}

auto Shape_rect::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    float w, h;

    if (GetOrigin().x < GetPoint().x) {
        p.x += GetOrigin().x;
        w = GetPoint().x - GetOrigin().x + 1;
    } else {
        p.x += GetPoint().x;
        w = GetOrigin().x - GetPoint().x + 1;
    }
    if (GetOrigin().y < GetPoint().y) {
        p.y += GetOrigin().y;
        h = GetPoint().y - GetOrigin().y + 1;
    } else {
        p.y += GetPoint().y;
        h = GetOrigin().y - GetPoint().y + 1;
    }
    if ((w > 1) && (h > 1)) {
        return {
            DrawCommands::Line(p.x, p.y, p.x + w, p.y),
            DrawCommands::Line(p.x + w, p.y, p.x + w, p.y + h),
            DrawCommands::Line(p.x + w, p.y + h, p.x, p.y + h),
            DrawCommands::Line(p.x, p.y + h, p.x, p.y),
        };
    }
    return {};
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

auto Lasso::GetCC_DrawCommand(Coord p) const -> std::vector<DrawCommand>
{
    std::vector<DrawCommand> result;
    if (pntlist.size() > 1) {
        for (auto iter = 0U; iter < pntlist.size() - 1; ++iter) {
            result.emplace_back(DrawCommands::Line(p + pntlist.at(iter), p + pntlist.at(iter + 1)));
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
