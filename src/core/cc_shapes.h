#pragma once
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

#include "cc_coord.h"
#include <vector>

namespace CalChart {
struct DrawCommand;

class Shape {
public:
    virtual ~Shape() = default;

    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const = 0;
    virtual void OnMove(const Coord& p, const Coord& snapped_p) = 0;
};

class Shape_1point : public Shape {
public:
    Shape_1point(const Coord& p)
        : origin(p)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    void MoveOrigin(const Coord& p) { origin = p; }
    auto GetOrigin() const { return origin; }

protected:
    Coord origin;
};

class Shape_crosshairs : public Shape_1point {
public:
    Shape_crosshairs(const Coord& p, Coord::units width)
        : Shape_1point(p)
        , crosshairs_width(width)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;

protected:
    Coord::units crosshairs_width;
};

class Shape_2point : public Shape_1point {
public:
    Shape_2point(const Coord& p)
        : Shape_1point(p)
        , point(p)
    {
    }
    Shape_2point(const Coord& p1, const Coord& p2)
        : Shape_1point(p1)
        , point(p2)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    void MovePoint(const Coord& p) { point = p; }
    auto GetPoint() const { return point; }

protected:
    Coord point;
};

class Shape_line : public Shape_2point {
public:
    Shape_line(const Coord& p)
        : Shape_2point(p)
    {
    }
    Shape_line(const Coord& p1, const Coord& p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
};

class Shape_x : public Shape_2point {
public:
    Shape_x(const Coord& p)
        : Shape_2point(p)
    {
    }
    Shape_x(const Coord& p1, const Coord& p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
};

class Shape_cross : public Shape_2point {
public:
    Shape_cross(const Coord& p)
        : Shape_2point(p)
    {
    }
    Shape_cross(const Coord& p1, const Coord& p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
};

class Shape_ellipse : public Shape_2point {
public:
    Shape_ellipse(const Coord& p)
        : Shape_2point(p)
    {
    }
    Shape_ellipse(const Coord& p1, const Coord& p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
};

class Shape_angline : public Shape_line {
public:
    Shape_angline(const Coord& p, const Coord& refvect)
        : Shape_line(p)
        , vect(refvect)
        , mag(refvect.Magnitude())
    {
    }

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;

private:
    Coord vect;
    float mag;
};

class Shape_arc : public Shape_2point {
public:
    Shape_arc(const Coord& c, const Coord& p);
    Shape_arc(const Coord& c, const Coord& p1, const Coord& p2);

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;

    inline float GetAngle() const { return r - r0; }

private:
    float r, r0, d;
};

class Shape_rect : public Shape_2point {
public:
    Shape_rect(const Coord& p)
        : Shape_2point(p)
    {
    }
    Shape_rect(const Coord& p1, const Coord& p2)
        : Shape_2point(p1, p2)
    {
    }

    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
};

class Lasso : public Shape {
public:
    Lasso(const Coord& p);

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
    void Clear();
    void Start(const Coord& p);
    void End();
    void Append(const Coord& p);
    bool Inside(const Coord& p) const;
    virtual std::vector<DrawCommand> GetCC_DrawCommand(float x,
        float y) const override;
    void Drag(const Coord& p);
    inline const Coord* FirstPoint() const
    {
        return pntlist.empty() ? nullptr : &pntlist.front();
    }
    std::vector<Coord> GetPointsOnLine(int numpnts) const;
    auto NumPoints() const { return pntlist.size(); }

private:
    static bool CrossesLine(const Coord& start, const Coord& end,
        const Coord& p);
    float GetDistance() const;
    std::vector<Coord> pntlist;
};

class Poly : public Lasso {
public:
    Poly(const Coord& p);

    virtual void OnMove(const Coord& p, const Coord& snapped_p) override;
};
}
