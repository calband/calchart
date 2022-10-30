/*
 * CalChartMovePointsTool.cpp
 * MovePointsTool represent the shapes and transformation of Marcher moves.
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

#include "CalChartMovePointsTool.h"

#include "CalChartShapes.h"
#include "CalChartUtils.h"
#include "linmath.h"
#include <cassert>

namespace CalChart {

class MovePointsTool_Normal : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveLine : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveRotate : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
    virtual void OnClickUp(CalChart::Coord pos) override;
    virtual bool IsDone() const override;
    virtual bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveShear : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
    virtual void OnClickUp(CalChart::Coord pos) override;
    virtual bool IsDone() const override;
    virtual bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveReflect : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveSize : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
    virtual void OnClickUp(CalChart::Coord pos) override;
    virtual bool IsDone() const override;
    virtual bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveGenius : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
    virtual void OnClickUp(CalChart::Coord pos) override;
    virtual bool IsDone() const override;
    virtual bool IsReadyForMoving() const override;
};

class MovePointsTool_ShapeLine : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeEllipse : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeRectangle : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeDraw : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeX : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeCross : public MovePointsTool {
public:
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const override;
    virtual void OnClickDown(CalChart::Coord pos) override;
};

std::unique_ptr<MovePointsTool> MovePointsTool::Create(CalChart::MoveMode curr_move)
{
    switch (curr_move) {
    case CalChart::MoveMode::Normal:
        return std::make_unique<MovePointsTool_Normal>();
    case CalChart::MoveMode::ShapeLine:
        return std::make_unique<MovePointsTool_ShapeLine>();
    case CalChart::MoveMode::ShapeX:
        return std::make_unique<MovePointsTool_ShapeX>();
    case CalChart::MoveMode::ShapeCross:
        return std::make_unique<MovePointsTool_ShapeCross>();
    case CalChart::MoveMode::ShapeRectange:
        return std::make_unique<MovePointsTool_ShapeRectangle>();
    case CalChart::MoveMode::ShapeEllipse:
        return std::make_unique<MovePointsTool_ShapeEllipse>();
    case CalChart::MoveMode::ShapeDraw:
        return std::make_unique<MovePointsTool_ShapeDraw>();
    case CalChart::MoveMode::MoveLine:
        return std::make_unique<MovePointsTool_MoveLine>();
    case CalChart::MoveMode::MoveRotate:
        return std::make_unique<MovePointsTool_MoveRotate>();
    case CalChart::MoveMode::MoveShear:
        return std::make_unique<MovePointsTool_MoveShear>();
    case CalChart::MoveMode::MoveReflect:
        return std::make_unique<MovePointsTool_MoveReflect>();
    case CalChart::MoveMode::MoveSize:
        return std::make_unique<MovePointsTool_MoveSize>();
    case CalChart::MoveMode::MoveGenius:
        return std::make_unique<MovePointsTool_MoveGenius>();
    }
    // last chance fall through
    return std::make_unique<MovePointsTool_Normal>();
}

template <typename T>
static std::map<int, CalChart::Coord> GetTransformedPoints(const Matrix<T>& transmat, std::map<int, CalChart::Coord> const& select_list)
{
    std::map<int, CalChart::Coord> result;
    for (auto i : select_list) {
        auto c = i.second;
        Vector<T> v(c.x, c.y, 0);
        v = transmat * v;
        v.Homogenize();
        c = CalChart::Coord(RoundToCoordUnits(v.GetX()), RoundToCoordUnits(v.GetY()));
        result[i.first] = c;
    }
    return result;
}

static std::vector<int> get_ordered_selection(std::map<int, CalChart::Coord> const& select_list)
{
    // stuff all the points into a map, which will order them.  Then pull out the points.
    std::multimap<CalChart::Coord, int> ordered;
    for (auto&& i : select_list) {
        ordered.insert({ i.second, i.first });
    }
    std::vector<int> result;
    for (auto&& i : ordered) {
        result.push_back(i.second);
    }
    return result;
}

void MovePointsTool::BeginMoveDrag(Drag type, CalChart::Coord start)
{
    move_drag = type;
    m_shape_list.clear();
    switch (type) {
    case Drag::BOX:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_rect>(start));
        break;
    case Drag::POLY:
        AddMoveDrag(type, std::make_unique<CalChart::Poly>(start));
        break;
    case Drag::LASSO:
        AddMoveDrag(type, std::make_unique<CalChart::Lasso>(start));
        break;
    case Drag::LINE:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_line>(start));
        break;
    case Drag::CROSSHAIRS:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_crosshairs>(start, Int2CoordUnits(2)));
        break;
    case Drag::SHAPE_ELLIPSE:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_ellipse>(start));
        break;
    case Drag::SHAPE_X:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_x>(start));
        break;
    case Drag::SHAPE_CROSS:
        AddMoveDrag(type, std::make_unique<CalChart::Shape_cross>(start));
        break;
    default:
        break;
    }
}

void MovePointsTool::AddMoveDrag(Drag type, std::unique_ptr<CalChart::Shape> shape)
{
    move_drag = type;
    m_shape_list.emplace_back(std::move(shape));
}

void MovePointsTool::OnMove(CalChart::Coord p, CalChart::Coord snapped_p)
{
    if (!m_shape_list.empty()) {
        m_shape_list.back()->OnMove(p, snapped_p);
    }
}

void MovePointsTool_MoveLine::OnClickDown(CalChart::Coord pos)
{
    BeginMoveDrag(Drag::LINE, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_MoveLine::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = static_cast<CalChart::Shape_2point const*>(m_shape_list.back().get());
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    std::map<int, CalChart::Coord> result;
    auto ordered = get_ordered_selection(select_list);
    for (auto i : ordered) {
        // should this have a snap to grid?
        result[i] = curr_pos;
        curr_pos += second - start;
    }
    return result;
}

void MovePointsTool_Normal::OnClickDown(CalChart::Coord pos)
{
    BeginMoveDrag(Drag::LINE, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_Normal::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    auto* shape = static_cast<CalChart::Shape_2point const*>(m_shape_list.back().get());
    auto pos = shape->GetPoint() - shape->GetOrigin();
    std::map<int, CalChart::Coord> result;
    for (auto i = select_list.begin(); i != select_list.end(); ++i) {
        result[i->first] = i->second + pos;
    }
    return result;
}

void MovePointsTool_MoveRotate::OnClickDown(CalChart::Coord pos)
{
    // rotate is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(Drag::CROSSHAIRS, pos);
    } else if (static_cast<CalChart::Shape_1point*>(m_shape_list.back().get())->GetOrigin() != pos) {
        // and this is a point not on the origin.
        AddMoveDrag(Drag::LINE,
            std::make_unique<CalChart::Shape_arc>(
                static_cast<CalChart::Shape_1point*>(m_shape_list.back().get())->GetOrigin(), pos));
    }
}

std::map<int, CalChart::Coord> MovePointsTool_MoveRotate::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    } else {
        auto start = dynamic_cast<CalChart::Shape_1point const&>(*m_shape_list[0]).GetOrigin();
        auto r = -((CalChart::Shape_arc*)m_shape_list.back().get())->GetAngle();
        auto m = TranslationMatrix(Vector<float>(-start.x, -start.y, 0)) * ZRotationMatrix(r) * TranslationMatrix(Vector<float>(start.x, start.y, 0));
        return GetTransformedPoints(m, select_list);
    }
}

void MovePointsTool_MoveRotate::OnClickUp(CalChart::Coord pos)
{
    if (IsReadyForMoving()) {
        auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(Drag::CROSSHAIRS, pos);
        }
    }
}

bool MovePointsTool_MoveRotate::IsDone() const
{
    return IsReadyForMoving();
}

bool MovePointsTool_MoveRotate::IsReadyForMoving() const
{
    if (m_shape_list.size() > 1) {
        return true;
    }
    return false;
}

void MovePointsTool_MoveShear::OnClickDown(CalChart::Coord pos)
{
    // shear is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(Drag::CROSSHAIRS, pos);
    } else if (((CalChart::Shape_1point*)m_shape_list.back().get())->GetOrigin() != pos) {
        CalChart::Coord vect(pos - ((CalChart::Shape_1point*)m_shape_list.back().get())->GetOrigin());
        // rotate vect 90 degrees
        AddMoveDrag(Drag::LINE, std::make_unique<CalChart::Shape_angline>(pos, CalChart::Coord(-vect.y, vect.x)));
    }
}

std::map<int, CalChart::Coord> MovePointsTool_MoveShear::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    assert(m_shape_list.size() == 2);
    auto start = dynamic_cast<CalChart::Shape_1point&>(*m_shape_list[0]).GetOrigin();
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto c1 = shape->GetOrigin();
    auto c2 = shape->GetPoint();
    auto v1 = c1 - start;
    auto v2 = c2 - c1;
    float amount = v2.Magnitude() / v1.Magnitude();
    if (BoundDirectionSigned(v1.Direction() - (c2 - start).Direction()) < 0) {
        amount = -amount;
    }
    float ang = Deg2Rad(-v1.Direction());
    auto m = TranslationMatrix(Vector<float>(-start.x, -start.y, 0)) * ZRotationMatrix(-ang) * YXShearMatrix(amount) * ZRotationMatrix(ang) * TranslationMatrix(Vector<float>(start.x, start.y, 0));
    return GetTransformedPoints(m, select_list);
}

void MovePointsTool_MoveShear::OnClickUp(CalChart::Coord pos)
{
    if (IsReadyForMoving()) {
        auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(Drag::CROSSHAIRS, pos);
        }
    }
}

bool MovePointsTool_MoveShear::IsDone() const
{
    return IsReadyForMoving();
}

bool MovePointsTool_MoveShear::IsReadyForMoving() const
{
    if (m_shape_list.size() > 1) {
        return true;
    }
    return false;
}

void MovePointsTool_MoveReflect::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::LINE, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_MoveReflect::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto c1 = shape->GetOrigin();
    auto c2 = shape->GetPoint() - c1;
    float ang = Deg2Rad(-c2.Direction());
    auto m = TranslationMatrix(Vector<float>(-c1.x, -c1.y, 0)) * ZRotationMatrix(-ang) * YReflectionMatrix<float>() * ZRotationMatrix(ang) * TranslationMatrix(Vector<float>(c1.x, c1.y, 0));
    return GetTransformedPoints(m, select_list);
}

void MovePointsTool_MoveSize::OnClickDown(CalChart::Coord pos)
{
    // move is a 2-distinct mouse click action
    if (m_shape_list.empty()) {
        BeginMoveDrag(Drag::CROSSHAIRS, pos);
    } else if (((CalChart::Shape_1point*)m_shape_list.back().get())->GetOrigin() != pos) {
        AddMoveDrag(Drag::LINE, std::make_unique<CalChart::Shape_line>(pos));
    }
}

std::map<int, CalChart::Coord> MovePointsTool_MoveSize::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    if (m_shape_list.size() < 2) {
        return select_list;
    } else {
        auto& origin = dynamic_cast<CalChart::Shape_1point&>(*m_shape_list[0]);
        auto c1 = origin.GetOrigin();
        auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
        auto c2 = shape->GetPoint() - c1;
        float sx = c2.x;
        float sy = c2.y;
        c2 = shape->GetOrigin() - c1;
        if ((c2.x != 0) || (c2.y != 0)) {
            if (c2.x != 0) {
                sx /= c2.x;
            } else {
                sx = 1;
            }
            if (c2.y != 0) {
                sy /= c2.y;
            } else {
                sy = 1;
            }
            auto m = TranslationMatrix(Vector<float>(-c1.x, -c1.y, 0)) * ScaleMatrix(Vector<float>(sx, sy, 0)) * TranslationMatrix(Vector<float>(c1.x, c1.y, 0));
            return GetTransformedPoints(m, select_list);
        }
        return std::map<int, CalChart::Coord>{};
    }
}

void MovePointsTool_MoveSize::OnClickUp(CalChart::Coord pos)
{
    if (IsReadyForMoving()) {
        auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginMoveDrag(Drag::CROSSHAIRS, pos);
        }
    }
}

bool MovePointsTool_MoveSize::IsDone() const
{
    return IsReadyForMoving();
}

bool MovePointsTool_MoveSize::IsReadyForMoving() const
{
    if (m_shape_list.size() > 1) {
        return true;
    }
    return false;
}

void MovePointsTool_MoveGenius::OnClickDown(CalChart::Coord pos)
{
    // move is a 2-distinct mouse click action
    if (m_shape_list.size() < 3) {
        AddMoveDrag(Drag::LINE, std::make_unique<CalChart::Shape_line>(pos));
    }
}

std::map<int, CalChart::Coord> MovePointsTool_MoveGenius::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    if (m_shape_list.size() < 3) {
        return select_list;
    } else {
        auto* v1 = (CalChart::Shape_2point*)m_shape_list[0].get();
        auto* v2 = (CalChart::Shape_2point*)m_shape_list[1].get();
        auto* v3 = (CalChart::Shape_2point*)m_shape_list[2].get();

        auto s1 = v1->GetOrigin();
        auto e1 = v1->GetPoint();
        auto s2 = v2->GetOrigin();
        auto e2 = v2->GetPoint();
        auto s3 = v3->GetOrigin();
        auto e3 = v3->GetPoint();
        auto d = (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y + (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y + (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y;
        if (IS_ZERO(d)) {
            return std::map<int, CalChart::Coord>{};
        } else {
            auto A = Matrix(Vector<float>(e1.x, e2.x, 0, e3.x), Vector<float>(e1.y, e2.y, 0, e3.y),
                Vector<float>(0, 0, 0, 0), Vector<float>(1, 1, 0, 1));
            auto Binv = Matrix(
                Vector<float>((float)s2.y - (float)s3.y, (float)s3.x - (float)s2.x, 0,
                    (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y),
                Vector<float>((float)s3.y - (float)s1.y, (float)s1.x - (float)s3.x, 0,
                    (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y),
                Vector<float>(0, 0, 0, 0),
                Vector<float>((float)s1.y - (float)s2.y, (float)s2.x - (float)s1.x, 0,
                    (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y));
            Binv /= d;
            Matrix m = Binv * A;
            return GetTransformedPoints(m, select_list);
        }
    }
}

void MovePointsTool_MoveGenius::OnClickUp(CalChart::Coord)
{
}

bool MovePointsTool_MoveGenius::IsDone() const
{
    return IsReadyForMoving();
}

bool MovePointsTool_MoveGenius::IsReadyForMoving() const
{
    if (m_shape_list.size() > 2) {
        return true;
    }
    return false;
}

void MovePointsTool_ShapeLine::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::LINE, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeLine::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    std::map<int, CalChart::Coord> result;
    auto distance = (select_list.size() > 1) ? (second - start) / static_cast<float>(select_list.size() - 1) : start;
    auto ordered = get_ordered_selection(select_list);
    for (auto i : ordered) {
        result[i] = curr_pos;
        curr_pos += distance;
    }
    return result;
}

void MovePointsTool_ShapeEllipse::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::SHAPE_ELLIPSE, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeEllipse::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto center = start + (end - start) / 2;
    const auto a = (end.x - start.x) / 2.0;
    const auto b = (end.y - start.y) / 2.0;
    std::map<int, CalChart::Coord> result;
    auto amount = (2.0 * std::numbers::pi) / select_list.size();
    auto angle = -std::numbers::pi / 2.0;
    auto ordered = get_ordered_selection(select_list);
    for (auto i : ordered) {
        // should this have a snap to grid?
        if ((angle > std::numbers::pi / 2.0) && (angle <= 3.0 * std::numbers::pi / 2.0)) {
            result[i] = center + CalChart::Coord(-(a * b) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))), -(a * b * tan(angle)) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))));
        } else {
            result[i] = center + CalChart::Coord((a * b) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))), (a * b * tan(angle)) / (sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle), 2))));
        }
        angle += amount;
    }
    return result;
}

void MovePointsTool_ShapeRectangle::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::BOX, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeRectangle::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto a = (end.x - start.x);
    const auto b = (end.y - start.y);
    const auto perimeter = a * 2.0 + b * 2.0;
    std::map<int, CalChart::Coord> result;
    auto each_segment = perimeter / select_list.size();
    auto total_distance = 0.0;
    auto ordered = get_ordered_selection(select_list);
    for (auto i : ordered) {
        if (total_distance < a) {
            result[i] = start + CalChart::Coord(total_distance, 0);
        } else if (total_distance < (a + b)) {
            result[i] = start + CalChart::Coord(a, total_distance - a);
        } else if (total_distance < (2.0 * a + b)) {
            result[i] = start + CalChart::Coord(a - (total_distance - (a + b)), b);
        } else {
            result[i] = start + CalChart::Coord(0, b - (total_distance - (2 * a + b)));
        }
        total_distance += each_segment;
    }
    return result;
}

void MovePointsTool_ShapeDraw::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::LASSO, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeDraw::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Lasso const*)m_shape_list.back().get();
    std::map<int, CalChart::Coord> result;
    auto points = shape->GetPointsOnLine(static_cast<int>(select_list.size()));
    auto iter = points.begin();
    assert(points.size() == select_list.size());
    auto ordered = get_ordered_selection(select_list);
    for (auto i : ordered) {
        result[i] = *iter;
        ++iter;
    }
    return result;
}

void MovePointsTool_ShapeX::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::SHAPE_X, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeX::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start1 = shape->GetOrigin();
    const auto end1 = shape->GetPoint();
    const auto start2 = CalChart::Coord(start1.x, end1.y);
    const auto end2 = CalChart::Coord(end1.x, start1.y);
    auto num_squares = (select_list.size() + 2) / 4;
    auto iter = select_list.begin();

    std::map<int, CalChart::Coord> result;
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

void MovePointsTool_ShapeCross::OnClickDown(CalChart::Coord pos)
{
    // reflect is a 1 mouse click action
    BeginMoveDrag(Drag::SHAPE_CROSS, pos);
}

std::map<int, CalChart::Coord> MovePointsTool_ShapeCross::TransformPoints(std::map<int, CalChart::Coord> const& select_list) const
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto o = shape->GetOrigin();
    const auto p = shape->GetPoint();
    const auto start1 = CalChart::Coord(o.x, o.y + (p.y - o.y) / 2);
    const auto end1 = CalChart::Coord(p.x, o.y + (p.y - o.y) / 2);
    const auto start2 = CalChart::Coord(o.x + (p.x - o.x) / 2, o.y);
    const auto end2 = CalChart::Coord(o.x + (p.x - o.x) / 2, p.y);
    auto num_squares = (select_list.size() + 2) / 4;
    auto iter = select_list.begin();

    std::map<int, CalChart::Coord> result;
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

}
