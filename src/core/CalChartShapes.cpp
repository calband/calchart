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
#include "CalChartConfiguration.h"
#include "CalChartDrawCommand.h"
#include "CalChartFileFormat.h"
#include "CalChartRanges.h"
#include <cmath>
#include <format>
#include <numeric>

namespace CalChart {

auto Shape_crosshairs::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
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

auto Shape_line::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return {
        Draw::Line(GetOrigin(), GetPoint())
    };
}

auto Shape_x::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return {
        Draw::Line(GetOrigin(), GetPoint()),
        Draw::Line(GetPoint().x, GetOrigin().y, GetOrigin().x, GetPoint().y),
    };
}

auto Shape_cross::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return {
        Draw::Line(GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2, GetOrigin().y, GetOrigin().x + (GetPoint().x - GetOrigin().x) / 2, GetPoint().y),
        Draw::Line(GetOrigin().x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2, GetPoint().x, GetOrigin().y + (GetPoint().y - GetOrigin().y) / 2),
    };
}

auto Shape_ellipse::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return {
        Draw::Ellipse(GetOrigin(), GetPoint())
    };
}

auto Shape_arc::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
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

auto Shape_rect::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return {
        Draw::Line(GetOrigin().x, GetOrigin().y, GetPoint().x, GetOrigin().y),
        Draw::Line(GetPoint().x, GetOrigin().y, GetPoint().x, GetPoint().y),
        Draw::Line(GetPoint().x, GetPoint().y, GetOrigin().x, GetPoint().y),
        Draw::Line(GetOrigin().x, GetPoint().y, GetOrigin().x, GetOrigin().y),
    };
}

namespace {
    auto PointsOnLine(std::vector<Coord> const& pntlist, int numpnts) -> std::vector<Coord>
    {
        if (numpnts < 1 || pntlist.empty()) {
            return {};
        }
        // if we are in the degenerative case where first and last point are equal,
        // then we just return the first point repeated.
        if (pntlist.size() == 2 && pntlist.front() == pntlist.back()) {
            return std::vector<Coord>(numpnts, pntlist.front());
        }
        std::vector<Coord> results;
        results.reserve(numpnts);
        results.push_back(pntlist.front());
        if (numpnts < 2) {
            return results;
        }

        const auto each_segment = GetDistance(pntlist) / (numpnts - 1);
        auto iter = pntlist.begin();
        auto curr_pnt = *iter++;
        auto running_dist = each_segment;
        while (iter != pntlist.end()) {
            // emergency rip chord on rounding cases
            if (static_cast<int>(results.size()) == numpnts) {
                break;
            }
            auto curr_dist = iter->Distance(curr_pnt);
            // if the distance to the next point is more than we need, use it up.
            if (running_dist > curr_dist) {
                running_dist -= curr_dist;
                curr_pnt = *iter++;
                continue;
            }
            // the new point will be along the vector by
            auto dist_vector = (*iter - curr_pnt);
            auto factor = (running_dist / curr_dist);
            dist_vector *= factor;
            auto new_pnt = curr_pnt + dist_vector;

            // we found a place for this point.
            results.push_back(new_pnt);

            curr_pnt = new_pnt;
            running_dist = each_segment;
        }
        // emergency rip chord on rounding cases
        while (static_cast<int>(results.size()) < numpnts) {
            results.push_back(pntlist.back());
        }
        return results;
    }
}

auto Lasso::GetPointsOnLine(int numpnts) const -> std::vector<Coord>
{
    return PointsOnLine(pntlist, numpnts);
}

auto Lasso::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    std::vector<Draw::DrawCommand> result;
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

namespace {
    // Compute a point on a Catmull-Rom spline segment
    auto catmullRom(CalChart::Coord p0, CalChart::Coord p1, CalChart::Coord p2, CalChart::Coord p3, double t) -> CalChart::Coord
    {
        // Catmull-Rom basis functions
        auto t2 = t * t;
        auto t3 = t2 * t;

        auto b0 = -0.5 * t3 + t2 - 0.5 * t;
        auto b1 = 1.5 * t3 - 2.5 * t2 + 1.0;
        auto b2 = -1.5 * t3 + 2.0 * t2 + 0.5 * t;
        auto b3 = 0.5 * t3 - 0.5 * t2;

        return (p0 * b0) + (p1 * b1) + (p2 * b2) + (p3 * b3);
    }

    // Generate the linear points of a Catmull-Rom spline
    auto generateCatmullRomPoints(std::vector<CalChart::Coord> const& controlPoints, int segments) -> std::vector<CalChart::Coord>
    {
        auto points = std::vector<CalChart::Coord>{};

        if (controlPoints.size() < 4) {
            return {};
        }

        points.emplace_back(controlPoints[1]);
        // Loop over segments defined by consecutive points
        for (size_t i = 1; i < controlPoints.size() - 2; ++i) {
            auto p0 = controlPoints[i - 1];
            auto p1 = controlPoints[i];
            auto p2 = controlPoints[i + 1];
            auto p3 = controlPoints[i + 2];

            // Generate points along the segment
            CalChart::Coord prevPoint = p1; // Start at the current point
            auto step = 1.0 / segments;

            for (int j = 1; j <= segments; ++j) {
                auto t = j * step;
                auto currentPoint = catmullRom(p0, p1, p2, p3, t);
                prevPoint = currentPoint;
                points.emplace_back(prevPoint);
            }
        }

        return points;
    }

    // Generate line segments from a Catmull-Rom spline
    auto generateDrawSegments(std::vector<CalChart::Coord> const& segmentPoints) -> std::vector<CalChart::Draw::DrawCommand>
    {
        auto pairs = CalChart::Ranges::adjacent_view<2>(segmentPoints);
        return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
            pairs | std::views::transform([](auto points) {
                return CalChart::Draw::Line{ std::get<0>(points), std::get<1>(points) };
            }));
    }

    auto GenerateCurve(std::vector<CalChart::Coord> const& segmentPoints) -> std::vector<CalChart::Draw::DrawCommand>
    {
        return generateDrawSegments(segmentPoints);
    }

    // Helper function to compute distance from a point to a line segment
    auto pointToSegmentDistance(CalChart::Coord point,
        CalChart::Coord segStart,
        CalChart::Coord segEnd) -> double
    {
        auto segment = segEnd - segStart;
        auto pointVec = point - segStart;

        auto segmentLengthSq = static_cast<double>(segment.Dot(segment));
        if (segmentLengthSq == 0) {
            // Segment is a single point
            return pointVec.Length();
        }

        // Project point onto segment, clamped to [0, 1]
        auto t = std::clamp(pointVec.Dot(segment) / segmentLengthSq, 0.0, 1.0);

        // Find the closest point on the segment
        auto projection = segStart + segment * t;

        // Return distance to the closest point
        return (point - projection).Length();
    }

    // Function to check if a point is close to any of these segments, and returns which and how much distance was traveled
    auto isPointOnCurve(
        std::vector<CalChart::Coord> const& segmentPoints,
        CalChart::Coord point,
        double tolerance) -> std::optional<std::tuple<size_t, double>>
    {
        auto distance = 0.0;
        auto lineSegments = CalChart::Ranges::adjacent_view<2>(segmentPoints);
        for (auto [i, segment] : CalChart::Ranges::enumerate_view(lineSegments)) {
            const auto& [segStart, segEnd] = segment;

            // Check if the point is within tolerance of the segment
            if (pointToSegmentDistance(point, segStart, segEnd) <= tolerance) {
                // Determine which control points the segment is between
                return std::tuple<size_t, double>{ i, distance };
            }
            distance += segStart.Distance(segEnd);
        }

        return std::nullopt; // No hit
    }
}

void Curve::Append(Coord p)
{
    mControlPoints.push_back(p);
    mMovingPoint = std::nullopt;
    Regenerate();
}

auto Curve::GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand>
{
    return GenerateCurve(mSegmentPoints);
}

auto Curve::GetPointsOnLine(int numpnts) const -> std::vector<Coord>
{
    return PointsOnLine(mSegmentPoints, numpnts);
}

auto Curve::LowerControlPointOnLine(Coord point, Coord::units searchBound) const -> std::optional<std::tuple<size_t, double>>
{
    auto totalDistance = GetDistance(mSegmentPoints);

    if (auto found = isPointOnCurve(mSegmentPoints, point, searchBound);
        found.has_value()) {
        auto [whichSegment, distance] = *found;
        return std::tuple<size_t, double>{ whichSegment / kNumberSegments, distance / totalDistance };
    }
    return std::nullopt;
}

auto Curve::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<int32_t>(mControlPoints.size()));
    for (auto&& point : mControlPoints) {
        Parser::Append(result, static_cast<int32_t>(point.x));
        Parser::Append(result, static_cast<int32_t>(point.y));
    }
    return result;
}

void Curve::Regenerate()
{
    auto drawPoints = std::vector<CalChart::Coord>{};
    drawPoints.reserve(mControlPoints.size() + 3);
    drawPoints.push_back(mControlPoints.front());
    std::copy(mControlPoints.begin(), mControlPoints.end(), std::back_inserter(drawPoints));
    if (mMovingPoint) {
        drawPoints.push_back(*mMovingPoint);
        drawPoints.push_back(*mMovingPoint);
    } else {
        drawPoints.push_back(mControlPoints.back());
    }
    mSegmentPoints = generateCatmullRomPoints(drawPoints, kNumberSegments);
}

auto CreateCurve(Reader reader) -> std::pair<Curve, Reader>
{
    auto numControlPoints = reader.Get<int32_t>();
    auto controlPoints = std::vector<Coord>{};
    while (numControlPoints--) {
        auto x = static_cast<Coord::units>(reader.Get<int32_t>());
        auto y = static_cast<Coord::units>(reader.Get<int32_t>());
        controlPoints.emplace_back(x, y);
    }
    return { Curve{ controlPoints }, reader };
}

}
