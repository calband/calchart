#pragma once
/*
 * CalChartShapes.h
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

/**
 * CalChart Shapes
 * These are general objects that represent Shapes in CalChart.
 * A shape object returns a vector of DrawCommand that represent how to draw the
 * shape.  This keeps the implementation details of "how" to draw seperate from
 * the "what" to draw, allowing reusablity.
 */

#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"
#include <numeric>
#include <optional>
#include <vector>

namespace CalChart {

class Reader;

using RawPolygon_t = std::vector<Coord>;
auto Inside(Coord p, RawPolygon_t const& polygon) -> bool;
auto CrossesLine(Coord start, Coord end, Coord p) -> bool;
auto GetDistance(RawPolygon_t const& polygon) -> double;

class Shape {
public:
    Shape() = default;
    Shape(Shape const&) = default;
    Shape(Shape&&) = default;
    auto operator=(Shape const&) -> Shape& = default;
    auto operator=(Shape&&) -> Shape& = default;
    virtual ~Shape() = default;

    // Call OnMove to indicate that the movable point of the shape has moved.
    // Different shapes have different "snap" policies.  The derived classes determine
    // if they will call snap by overloading the useSnap() function and then will
    // call the provided snapper function.
    template <typename Function>
    void OnMove(Coord p, Function snapper) { OnMoveImpl(useSnap() ? snapper(p) : p); }

    [[nodiscard]] virtual auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> = 0;
    [[nodiscard]] virtual auto GetPolygon() const -> RawPolygon_t { return {}; }

private:
    // this is using the Template Pattern -- the derived classes determine if
    // they should use the snapped version of the Coord, or what move should do.
    virtual void OnMoveImpl(Coord p) = 0;
    virtual auto useSnap() -> bool { return false; }
};

class Shape_1point : public Shape {
public:
    explicit Shape_1point(Coord p)
        : origin(p)
    {
    }

    [[nodiscard]] auto GetOrigin() const { return origin; }

protected:
    void MoveOrigin(Coord p) { origin = p; }

private:
    Coord origin;
    void OnMoveImpl(Coord p) override { MoveOrigin(p); }
    auto useSnap() -> bool override { return false; }
};

class Shape_crosshairs : public Shape_1point {
public:
    Shape_crosshairs(Coord p, Coord::units width)
        : Shape_1point(p)
        , crosshairs_width(width)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

private:
    Coord::units crosshairs_width;
    void OnMoveImpl(Coord p) override { MoveOrigin(p); }
    auto useSnap() -> bool override { return true; }
};

class Shape_2point : public Shape {
public:
    explicit Shape_2point(Coord p)
        : origin(p)
        , point(p)
    {
    }
    Shape_2point(Coord p1, Coord p2)
        : origin(p1)
        , point(p2)
    {
    }

    [[nodiscard]] auto GetOrigin() const { return origin; }
    [[nodiscard]] auto GetPoint() const { return point; }
    [[nodiscard]] auto GetPolygon() const -> RawPolygon_t override;

protected:
    void MovePoint(Coord p) { point = p; }

private:
    Coord origin;
    Coord point;
    void OnMoveImpl(Coord p) override { MovePoint(p); }
};

class Shape_line : public Shape_2point {
public:
    explicit Shape_line(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_line(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

private:
    void OnMoveImpl(Coord p) override { MovePoint(p); }
    auto useSnap() -> bool override { return true; }
};

class Shape_x : public Shape_2point {
public:
    explicit Shape_x(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_x(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

private:
    void OnMoveImpl(Coord p) override { MovePoint(p); }
    auto useSnap() -> bool override { return true; }
};

class Shape_cross : public Shape_2point {
public:
    explicit Shape_cross(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_cross(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

private:
    void OnMoveImpl(Coord p) override { MovePoint(p); }
    auto useSnap() -> bool override { return true; }
};

class Shape_ellipse : public Shape_2point {
public:
    explicit Shape_ellipse(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_ellipse(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

private:
    void OnMoveImpl(Coord p) override { MovePoint(p); }
    auto useSnap() -> bool override { return true; }
};

class Shape_angline : public Shape_line {
public:
    Shape_angline(Coord p, Coord refvect)
        : Shape_line(p)
        , vect(refvect)
    {
    }

private:
    Coord vect;
    void OnMoveImpl(Coord p) override
    {
        auto d = vect.Dot(p - GetOrigin()) / std::pow(vect.Distance({ 0, 0 }), 2);
        MovePoint(GetOrigin() + (vect * d));
    }
    auto useSnap() -> bool override { return true; }
};

class Shape_arc : public Shape_1point {
public:
    Shape_arc(Coord c, Coord p)
        : Shape_1point(c)
        , angle(c.Direction(p))
        , angle0(c.Direction(p))
        , d(c.Distance(p))
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;
    [[nodiscard]] auto GetAngle() const { return angle - angle0; }

private:
    CalChart::Radian angle;
    CalChart::Radian angle0;
    double d;
    void OnMoveImpl(Coord p) override { angle = GetOrigin().Direction(p); }

    auto useSnap() -> bool override { return true; }
};

class Shape_rect : public Shape_2point {
public:
    explicit Shape_rect(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_rect(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;
};

class Lasso : public Shape {
public:
    explicit Lasso(Coord p) { Append(p); }

    void Append(Coord p) { pntlist.push_back(p); }
    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

protected:
    void Drag(Coord p);

public:
    [[nodiscard]] auto FirstPoint() const -> Coord const*
    {
        return pntlist.empty() ? nullptr : &pntlist.front();
    }
    [[nodiscard]] auto GetPointsOnLine(int numpnts) const -> std::vector<Coord>;
    [[nodiscard]] auto GetPolygon() const -> RawPolygon_t override { return { pntlist }; }

private:
    std::vector<Coord> pntlist;

    void OnMoveImpl(Coord p) override { Append(p); }
};

class Poly : public Lasso {
public:
    explicit Poly(Coord p);

private:
    void OnMoveImpl(Coord p) override { Drag(p); }
};

inline auto CrossesLine(Coord start, Coord end, Coord p) -> bool
{
    if (start.y <= end.y) {
        if ((p.y > end.y) || (p.y <= start.y)) {
            return false;
        }
    } else {
        if ((p.y > start.y) || (p.y <= end.y)) {
            return false;
        }
    }
    return (p.x >= ((end.x - start.x) * (p.y - start.y) / (end.y - start.y) + start.x));
}

// Test if inside polygon using odd-even rule
inline auto Inside(Coord p, RawPolygon_t const& polygon) -> bool
{
    auto parity = false;
    if (polygon.size() < 2) {
        return parity;
    }
    // use inner product to do adjacent comparisons.
    return std::inner_product(
        polygon.begin(), polygon.end() - 1,
        polygon.begin() + 1,
        CrossesLine(polygon.back(), polygon.front(), p),
        [](auto acc, auto next) {
            return next ^ acc;
        },
        [p](auto a, auto b) {
            return CrossesLine(a, b, p);
        });
}

inline auto GetDistance(RawPolygon_t const& polygon) -> double
{
    if (polygon.empty()) {
        return {};
    }
    return std::inner_product(polygon.begin(), polygon.end() - 1, polygon.begin() + 1, 0.F, std::plus(), [](auto a, auto b) {
        return b.Distance(a);
    });
}

// A curve is described by Control Points, which are appended.
class Curve : public Shape {
    static constexpr auto kNumberSegments = 10;

public:
    explicit Curve(Coord p)
        : mControlPoints{ p }
    {
        Regenerate();
    }
    explicit Curve(std::vector<Coord> points)
        : mControlPoints{ points }
    {
        Regenerate();
    }

    void cleanMove()
    {
        mMovingPoint = std::nullopt;
        Regenerate();
    }

    void Append(Coord p);
    [[nodiscard]] auto GetCC_DrawCommand() const -> std::vector<Draw::DrawCommand> override;

    [[nodiscard]] auto GetPointsOnLine(int numpnts) const -> std::vector<Coord>;
    [[nodiscard]] auto GetControlPoints() const -> std::vector<Coord> { return mControlPoints; }
    [[nodiscard]] auto LowerControlPointOnLine(Coord point, Coord::units searchBound) const -> std::optional<std::tuple<size_t, double>>;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte>;

    void Regenerate();

    [[nodiscard]] auto operator==(Curve const& other) const -> bool
    {
        return mControlPoints == other.mControlPoints
            && mMovingPoint == other.mMovingPoint;
    }

private:
    std::vector<Coord> mControlPoints;
    std::optional<Coord> mMovingPoint;

    std::vector<Coord> mSegmentPoints; // this is an "memoization"; keep this coherenet with mControlPoints

    void OnMoveImpl(Coord p) override
    {
        mMovingPoint = p;
        Regenerate();
    }
};

auto CreateCurve(Reader) -> std::pair<Curve, Reader>;

}
