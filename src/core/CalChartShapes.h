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

#include "CalChartCoord.h"
#include <vector>
#include <optional>

/**
 * CalChart Shapes
 * These are general objects that represent Shapes in CalChart.
 * A shape object returns a vector of DrawCommands that represent how to draw the shape.  This keeps the implementation details
 * of "how" to draw seperate from the "what" to draw, allowing reusablity.
 */

namespace CalChart {
struct DrawCommand;

using RawPolygon_t = std::vector<Coord>;
bool Inside(Coord p, RawPolygon_t const& polygon);

class Shape {
public:
    virtual ~Shape() = default;

    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const = 0;
    virtual void OnMove(Coord p, Coord snapped_p) = 0;
    virtual std::optional<RawPolygon_t> GetPolygon() const { return {}; }
};

class Shape_1point : public Shape {
public:
    Shape_1point(Coord p)
        : origin(p)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    void MoveOrigin(Coord p) { origin = p; }
    auto GetOrigin() const { return origin; }

protected:
    Coord origin;
};

class Shape_crosshairs : public Shape_1point {
public:
    Shape_crosshairs(Coord p, Coord::units width)
        : Shape_1point(p)
        , crosshairs_width(width)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;

protected:
    Coord::units crosshairs_width;
};

class Shape_2point : public Shape_1point {
public:
    Shape_2point(Coord p)
        : Shape_1point(p)
        , point(p)
    {
    }
    Shape_2point(Coord p1, Coord p2)
        : Shape_1point(p1)
        , point(p2)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    void MovePoint(Coord p) { point = p; }
    auto GetPoint() const { return point; }
    std::optional<RawPolygon_t> GetPolygon() const override;

protected:
    Coord point;
};

class Shape_line : public Shape_2point {
public:
    Shape_line(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_line(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class Shape_x : public Shape_2point {
public:
    Shape_x(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_x(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class Shape_cross : public Shape_2point {
public:
    Shape_cross(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_cross(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class Shape_ellipse : public Shape_2point {
public:
    Shape_ellipse(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_ellipse(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class Shape_angline : public Shape_line {
public:
    Shape_angline(Coord p, Coord refvect)
        : Shape_line(p)
        , vect(refvect)
        , mag(refvect.Magnitude())
    {
    }

    virtual void OnMove(Coord p, Coord snapped_p) override;

private:
    Coord vect;
    float mag;
};

class Shape_arc : public Shape_2point {
public:
    Shape_arc(Coord c, Coord p);
    Shape_arc(Coord c, Coord p1, Coord p2);

    virtual void OnMove(Coord p, Coord snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;

    inline auto GetAngle() const { return r - r0; }

private:
    float r, r0, d;
};

class Shape_rect : public Shape_2point {
public:
    Shape_rect(Coord p)
        : Shape_2point(p)
    {
    }
    Shape_rect(Coord p1, Coord p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
};

class Lasso : public Shape {
public:
    Lasso(Coord p);

    virtual void OnMove(Coord p, Coord snapped_p) override;
    void Clear();
    void Start(Coord p);
    void End();
    void Append(Coord p);
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x, float y) const override;
    void Drag(Coord p);
    inline const Coord* FirstPoint() const
    {
        return pntlist.empty() ? nullptr : &pntlist.front();
    }
    std::vector<Coord> GetPointsOnLine(int numpnts) const;
    auto NumPoints() const { return pntlist.size(); }
    std::optional<RawPolygon_t> GetPolygon() const override { return { pntlist }; }

private:
    float GetDistance() const;
    std::vector<Coord> pntlist;
};

class Poly : public Lasso {
public:
    Poly(Coord p);

    virtual void OnMove(Coord p, Coord snapped_p) override;
};
}
