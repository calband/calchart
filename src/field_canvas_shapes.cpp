/*
 * field_canvas_shapes.cpp
 * Canvas for the Field window
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "field_canvas_shapes.h"

#include "linmath.h"
#include "confgr.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "draw.h"
#include "math_utils.h"

#include <wx/dcbuffer.h>

class MovePoints_CC_MOVE_NORMAL : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_LINE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_ROTATE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
    virtual bool OnMouseUpDone(CC_coord const&) override;
};

class MovePoints_CC_MOVE_SHEAR : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
    virtual bool OnMouseUpDone(CC_coord const&) override;
};

class MovePoints_CC_MOVE_REFL : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SIZE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
    virtual bool OnMouseUpDone(CC_coord const&) override;
};

class MovePoints_CC_MOVE_GENIUS : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
    virtual bool OnMouseUpDone(CC_coord const&) override;
};

class MovePoints_CC_MOVE_SHAPE_LINE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SHAPE_ELLIPSE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SHAPE_RECTANGLE : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SHAPE_DRAW : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SHAPE_X : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

class MovePoints_CC_MOVE_SHAPE_CROSS : public MovePoints
{
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const override;
    virtual void OnMouseLeftDown(CC_coord pos) override;
};

std::unique_ptr<MovePoints> Create_MovePoints(CC_MOVE_MODES curr_move)
{
    switch (curr_move) {
    case CC_MOVE_NORMAL:
        return std::make_unique<MovePoints_CC_MOVE_LINE>();
    case CC_MOVE_SHAPE_LINE:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_LINE>();
    case CC_MOVE_SHAPE_X:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_X>();
    case CC_MOVE_SHAPE_CROSS:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_CROSS>();
    case CC_MOVE_SHAPE_RECTANGLE:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_RECTANGLE>();
    case CC_MOVE_SHAPE_ELLIPSE:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_ELLIPSE>();
    case CC_MOVE_SHAPE_DRAW:
        return std::make_unique<MovePoints_CC_MOVE_SHAPE_DRAW>();
    case CC_MOVE_LINE:
        return std::make_unique<MovePoints_CC_MOVE_LINE>();
    case CC_MOVE_ROTATE:
        return std::make_unique<MovePoints_CC_MOVE_ROTATE>();
    case CC_MOVE_SHEAR:
        return std::make_unique<MovePoints_CC_MOVE_SHEAR>();
    case CC_MOVE_REFL:
        return std::make_unique<MovePoints_CC_MOVE_REFL>();
    case CC_MOVE_SIZE:
        return std::make_unique<MovePoints_CC_MOVE_SIZE>();
    case CC_MOVE_GENIUS:
        return std::make_unique<MovePoints_CC_MOVE_GENIUS>();
    }
}

static std::map<int, CC_coord> GetTransformedPoints(const Matrix& transmat, std::map<int, CC_coord> const& select_list)
{
    std::map<int, CC_coord> result;
    for (auto i : select_list) {
        auto c = i.second;
        Vector v(c.x, c.y, 0);
        v = transmat * v;
        v.Homogenize();
        c = CC_coord(RoundToCoord(v.GetX()), RoundToCoord(v.GetY()));
        result[i.first] = c;
    }
    return result;
}

void FieldCanvasShapes::BeginMoveDrag(CC_DRAG_TYPES type, const CC_coord& start)
{
    move_drag = type;
    m_shape_list.clear();
    switch (type) {
    case CC_DRAG_BOX:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_rect(start)));
        break;
    case CC_DRAG_POLY:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_poly(start)));
        break;
    case CC_DRAG_LASSO:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_lasso(start)));
        break;
    case CC_DRAG_LINE:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_line(start)));
        break;
    case CC_DRAG_CROSSHAIRS:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_crosshairs(start, Int2Coord(2))));
        break;
    case CC_DRAG_SHAPE_ELLIPSE:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_ellipse(start)));
        break;
    case CC_DRAG_SHAPE_X:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_x(start)));
        break;
    case CC_DRAG_SHAPE_CROSS:
        AddMoveDrag(type, std::unique_ptr<CC_shape>(new CC_shape_cross(start)));
        break;
    default:
        break;
    }
}

void FieldCanvasShapes::AddMoveDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape)
{
    move_drag = type;
    m_shape_list.emplace_back(std::move(shape));
}

void FieldCanvasShapes::OnMove(const CC_coord& p, const CC_coord& snapped_p)
{
    if (!m_shape_list.empty()) {
        m_shape_list.back()->OnMove(p, snapped_p);
    }
}


void MovePoints_CC_MOVE_LINE::OnMouseLeftDown(CC_coord pos)
{
    BeginMoveDrag(CC_DRAG_LINE, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_LINE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    std::map<int, CC_coord> result;
    for (auto i = select_list.begin(); i != select_list.end(); ++i, curr_pos += second - start) {
        // should this have a snap to grid?
        result[i->first] = curr_pos;
    }
    return result;
}

void MovePoints_CC_MOVE_NORMAL::OnMouseLeftDown(CC_coord pos)
{
    BeginMoveDrag(CC_DRAG_LINE, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_NORMAL::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    CC_coord pos = shape->GetPoint() - shape->GetOrigin();
    std::map<int, CC_coord> result;
    for (auto i = select_list.begin(); i != select_list.end(); ++i) {
        result[i->first] = i->second + pos;
    }
    return result;
}

void MovePoints_CC_MOVE_ROTATE::OnMouseLeftDown(CC_coord pos)
{
    // rotate is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
    }
    else if (((CC_shape_1point*)m_shape_list.back().get())->GetOrigin() != pos) {
        // and this is a point not on the origin.
        AddMoveDrag(CC_DRAG_LINE,
            std::unique_ptr<CC_shape>(new CC_shape_arc(
                ((CC_shape_1point*)m_shape_list.back().get())->GetOrigin(), pos)));
    }
}

std::map<int, CC_coord> MovePoints_CC_MOVE_ROTATE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    else {
        auto start = dynamic_cast<CC_shape_1point&>(*m_shape_list[0]).GetOrigin();
        auto r = -((CC_shape_arc*)m_shape_list.back().get())->GetAngle();
        auto m = TranslationMatrix(Vector(-start.x, -start.y, 0)) * ZRotationMatrix(r) * TranslationMatrix(Vector(start.x, start.y, 0));
        return GetTransformedPoints(m, select_list);
    }
}

bool MovePoints_CC_MOVE_ROTATE::OnMouseUpDone(CC_coord const& pos)
{
    if (m_shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
            return false;
        }
        return true;
    }
    return false;
}

void MovePoints_CC_MOVE_SHEAR::OnMouseLeftDown(CC_coord pos)
{
    // shear is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
    }
    else if (((CC_shape_1point*)m_shape_list.back().get())->GetOrigin() != pos) {
        CC_coord vect(pos - ((CC_shape_1point*)m_shape_list.back().get())->GetOrigin());
        // rotate vect 90 degrees
        AddMoveDrag(CC_DRAG_LINE, std::unique_ptr<CC_shape>(new CC_shape_angline(
                                  pos, CC_coord(-vect.y, vect.x))));
    }
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHEAR::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    else {
        assert(m_shape_list.size() == 2);
        auto start = dynamic_cast<CC_shape_1point&>(*m_shape_list[0]).GetOrigin();
        const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
        auto c1 = shape->GetOrigin();
        auto c2 = shape->GetPoint();
        auto v1 = c1 - start;
        auto v2 = c2 - c1;
        float amount = v2.Magnitude() / v1.Magnitude();
        if (BoundDirectionSigned(v1.Direction() - (c2 - start).Direction()) < 0) {
            amount = -amount;
        }
        float ang = -v1.Direction() * M_PI / 180.0;
        auto m = TranslationMatrix(Vector(-start.x, -start.y, 0)) * ZRotationMatrix(-ang) * YXShearMatrix(amount) * ZRotationMatrix(ang) * TranslationMatrix(Vector(start.x, start.y, 0));
        return GetTransformedPoints(m, select_list);
    }
}

bool MovePoints_CC_MOVE_SHEAR::OnMouseUpDone(CC_coord const& pos)
{
    if (m_shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
            return false;
        }
        return true;
    }
    return false;
}

void MovePoints_CC_MOVE_REFL::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_LINE, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_REFL::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    CC_coord c1 = shape->GetOrigin();
    CC_coord c2 = shape->GetPoint() - c1;
    float ang = -c2.Direction() * M_PI / 180.0;
    auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) * ZRotationMatrix(-ang) * YReflectionMatrix() * ZRotationMatrix(ang) * TranslationMatrix(Vector(c1.x, c1.y, 0));
    return GetTransformedPoints(m, select_list);
}

void MovePoints_CC_MOVE_SIZE::OnMouseLeftDown(CC_coord pos)
{
    // move is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
    }
    else if (((CC_shape_1point*)m_shape_list.back().get())->GetOrigin() != pos) {
        AddMoveDrag(CC_DRAG_LINE,
            std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
    }
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SIZE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    else {
        auto& origin = dynamic_cast<CC_shape_1point&>(*m_shape_list[0]);
        CC_coord c1 = origin.GetOrigin();
        const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
        CC_coord c2 = shape->GetPoint() - c1;
        float sx = c2.x;
        float sy = c2.y;
        c2 = shape->GetOrigin() - c1;
        if ((c2.x != 0) || (c2.y != 0)) {
            if (c2.x != 0) {
                sx /= c2.x;
            }
            else {
                sx = 1;
            }
            if (c2.y != 0) {
                sy /= c2.y;
            }
            else {
                sy = 1;
            }
            auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) * ScaleMatrix(Vector(sx, sy, 0)) * TranslationMatrix(Vector(c1.x, c1.y, 0));
            return GetTransformedPoints(m, select_list);
        }
        return std::map<int, CC_coord>{};
    }
}

bool MovePoints_CC_MOVE_SIZE::OnMouseUpDone(CC_coord const& pos)
{
    if (m_shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(CC_DRAG_CROSSHAIRS, pos);
            return false;
        }
        return true;
    }
    return false;
}

void MovePoints_CC_MOVE_GENIUS::OnMouseLeftDown(CC_coord pos)
{
    // move is a 2-distinct mouse click action
    if (m_shape_list.size() < 2) {
        AddMoveDrag(CC_DRAG_LINE, std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
    }
}

std::map<int, CC_coord> MovePoints_CC_MOVE_GENIUS::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    else {
        CC_shape_2point* v1 = (CC_shape_2point*)m_shape_list[0].get();
        CC_shape_2point* v2 = (CC_shape_2point*)m_shape_list[1].get();
        CC_shape_2point* v3 = (CC_shape_2point*)m_shape_list[2].get();

        CC_coord s1 = v1->GetOrigin();
        CC_coord e1 = v1->GetPoint();
        CC_coord s2 = v2->GetOrigin();
        CC_coord e2 = v2->GetPoint();
        CC_coord s3 = v3->GetOrigin();
        CC_coord e3 = v3->GetPoint();
        auto d = (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y + (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y + (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y;
        if (IS_ZERO(d)) {
            return std::map<int, CC_coord>{};
        }
        else {
            Matrix A = Matrix(Vector(e1.x, e2.x, 0, e3.x), Vector(e1.y, e2.y, 0, e3.y),
                Vector(0, 0, 0, 0), Vector(1, 1, 0, 1));
            Matrix Binv = Matrix(
                Vector((float)s2.y - (float)s3.y, (float)s3.x - (float)s2.x, 0,
                    (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y),
                Vector((float)s3.y - (float)s1.y, (float)s1.x - (float)s3.x, 0,
                    (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y),
                Vector(0, 0, 0, 0),
                Vector((float)s1.y - (float)s2.y, (float)s2.x - (float)s1.x, 0,
                    (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y));
            Binv /= d;
            Matrix m = Binv * A;
            return GetTransformedPoints(m, select_list);
        }
    }
}

bool MovePoints_CC_MOVE_GENIUS::OnMouseUpDone(CC_coord const& pos)
{
    if (m_shape_list.size() > 2) {
        return true;
    }
    return false;
}

void MovePoints_CC_MOVE_SHAPE_LINE::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_LINE, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_LINE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    std::map<int, CC_coord> result;
    auto distance = (select_list.size() > 1) ? (second - start) / (select_list.size() - 1) : start;
    for (auto i = select_list.begin(); i != select_list.end(); ++i, curr_pos += distance) {
        // should this have a snap to grid?
        result[i->first] = curr_pos;
    }
    return result;
}

void MovePoints_CC_MOVE_SHAPE_ELLIPSE::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_SHAPE_ELLIPSE, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_ELLIPSE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto center = start + (end - start) / 2;
    const auto a = (end.x - start.x) / 2.0;
    const auto b = (end.y - start.y) / 2.0;
    std::map<int, CC_coord> result;
    auto amount = (2.0 * M_PI) / select_list.size();
    auto angle = -M_PI / 2.0;
    for (auto i = select_list.begin(); i != select_list.end(); ++i, angle += amount) {
        // should this have a snap to grid?
        if ((angle > M_PI / 2.0) && (angle <= 3.0 * M_PI / 2.0)) {
            result[i->first] = center + CC_coord(-(a * b) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))), -(a * b * tan(angle)) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))));
        }
        else {
            result[i->first] = center + CC_coord((a * b) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))), (a * b * tan(angle)) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))));
        }
    }
    return result;
}

void MovePoints_CC_MOVE_SHAPE_RECTANGLE::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_BOX, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_RECTANGLE::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto a = (end.x - start.x);
    const auto b = (end.y - start.y);
    const auto perimeter = a * 2.0 + b * 2.0;
    std::map<int, CC_coord> result;
    auto each_segment = perimeter / select_list.size();
    auto total_distance = 0.0;
    for (auto i = select_list.begin(); i != select_list.end(); ++i, total_distance += each_segment) {
        if (total_distance < a) {
            result[i->first] = start + CC_coord(total_distance, 0);
        }
        else if (total_distance < (a + b)) {
            result[i->first] = start + CC_coord(a, total_distance - a);
        }
        else if (total_distance < (2.0 * a + b)) {
            result[i->first] = start + CC_coord(a - (total_distance - (a + b)), b);
        }
        else {
            result[i->first] = start + CC_coord(0, b - (total_distance - (2 * a + b)));
        }
    }
    return result;
}

void MovePoints_CC_MOVE_SHAPE_DRAW::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_LASSO, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_DRAW::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CC_lasso const*)m_shape_list.back().get();
    std::map<int, CC_coord> result;
    auto points = shape->GetPointsOnLine(static_cast<int>(select_list.size()));
    auto iter = points.begin();
    assert(points.size() == select_list.size());
    for (auto i = select_list.begin(); i != select_list.end(); ++i, ++iter) {
        result[i->first] = *iter;
    }
    return result;
}

void MovePoints_CC_MOVE_SHAPE_X::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_SHAPE_X, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_X::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    const auto start1 = shape->GetOrigin();
    const auto end1 = shape->GetPoint();
    const auto start2 = CC_coord(start1.x, end1.y);
    const auto end2 = CC_coord(end1.x, start1.y);
    auto num_squares = (select_list.size() + 2) / 4;
    auto iter = select_list.begin();

    std::map<int, CC_coord> result;
    for (auto i = 0u; i < num_squares; ++i) {
        result[iter->first] = start1 + (end1 - start1) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = end2 - (end2 - start2) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = start2 + (end2 - start2) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = end1 - (end1 - start1) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
    }
    if (iter != select_list.end()) {
        result[iter->first] = start1 + (end1 - start1) / 2;
        iter++;
    }
    assert(iter == select_list.end());
    return result;
}

void MovePoints_CC_MOVE_SHAPE_CROSS::OnMouseLeftDown(CC_coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(CC_DRAG_SHAPE_CROSS, pos);
}

std::map<int, CC_coord> MovePoints_CC_MOVE_SHAPE_CROSS::TransformPoints(std::map<int, CC_coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    const CC_shape_2point* shape = (CC_shape_2point*)m_shape_list.back().get();
    const auto o = shape->GetOrigin();
    const auto p = shape->GetPoint();
    const auto start1 = CC_coord(o.x, o.y + (p.y - o.y) / 2);
    const auto end1 = CC_coord(p.x, o.y + (p.y - o.y) / 2);
    const auto start2 = CC_coord(o.x + (p.x - o.x) / 2, o.y);
    const auto end2 = CC_coord(o.x + (p.x - o.x) / 2, p.y);
    auto num_squares = (select_list.size() + 2) / 4;
    auto iter = select_list.begin();

    std::map<int, CC_coord> result;
    for (auto i = 0u; i < num_squares; ++i) {
        result[iter->first] = start1 + (end1 - start1) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = end2 - (end2 - start2) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = start2 + (end2 - start2) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
        result[iter->first] = end1 - (end1 - start1) / (2 * num_squares) * i;
        iter++;
        if (iter == select_list.end())
            break;
    }
    if (iter != select_list.end()) {
        result[iter->first] = start1 + (end1 - start1) / 2;
        iter++;
    }
    assert(iter == select_list.end());
    return result;
}


