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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1

#include "CalChartMovePointsTool.h"
#include "CalChartDrawCommand.h"
#include "CalChartShapes.h"
#include "CalChartUtils.h"
#include "linmath.h"
#include <cassert>
#include <ranges>

namespace {
template <typename T>
auto CoordPair_cast(CalChart::Coord const coord) -> std::pair<T, T>
{
    return { static_cast<T>(coord.x), static_cast<T>(coord.y) };
}
}

namespace CalChart {

class MovePointsTool_Normal : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveLine : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveRotate : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
    void OnClickUp(CalChart::Coord pos) override;
    bool IsDone() const override;
    bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveShear : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
    void OnClickUp(CalChart::Coord pos) override;
    bool IsDone() const override;
    bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveReflect : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_MoveSize : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
    void OnClickUp(CalChart::Coord pos) override;
    bool IsDone() const override;
    bool IsReadyForMoving() const override;
};

class MovePointsTool_MoveGenius : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
    void OnClickUp(CalChart::Coord pos) override;
    bool IsDone() const override;
    bool IsReadyForMoving() const override;
};

class MovePointsTool_ShapeLine : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeEllipse : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeRectangle : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeDraw : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeX : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
};

class MovePointsTool_ShapeCross : public MovePointsTool {
public:
    auto TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition override;
    void OnClickDown(CalChart::Coord pos) override;
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
static auto GetTransformedPoints(const Matrix<T>& transmat, MarcherToPosition const& select_list) -> MarcherToPosition
{
    MarcherToPosition result;
    for (auto i : select_list) {
        auto [c_x, c_y] = CoordPair_cast<T>(i.second);
        Vector<T> v{ c_x, c_y, 0 };
        v = transmat * v;
        v.Homogenize();
        auto c = CalChart::Coord(RoundToCoordUnits(v.GetX()), RoundToCoordUnits(v.GetY()));
        result[i.first] = c;
    }
    return result;
}

static auto get_ordered_selection(MarcherToPosition const& select_list) -> std::vector<MarcherIndex>
{
    // stuff all the points into a map, which will order them.  Then pull out the points.
    std::multimap<CalChart::Coord, MarcherIndex> ordered;
    for (auto&& i : select_list) {
        ordered.insert({ i.second, i.first });
    }
    std::vector<MarcherIndex> result;
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

auto MovePointsTool::GenerateDrawCommands() const -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Draw::toDrawCommands(m_shape_list
        | std::views::transform([](auto&& shape) { return shape->GetCC_DrawCommand(); })
        | std::views::join);
}

void MovePointsTool::AddMoveDrag(Drag type, std::unique_ptr<CalChart::Shape> shape)
{
    move_drag = type;
    m_shape_list.emplace_back(std::move(shape));
}

void MovePointsTool_MoveLine::OnClickDown(CalChart::Coord pos)
{
    BeginMoveDrag(Drag::LINE, pos);
}

auto MovePointsTool_MoveLine::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = static_cast<CalChart::Shape_2point const*>(m_shape_list.back().get());
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    MarcherToPosition result;
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

auto MovePointsTool_Normal::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    auto* shape = static_cast<CalChart::Shape_2point const*>(m_shape_list.back().get());
    auto pos = shape->GetPoint() - shape->GetOrigin();
    MarcherToPosition result;
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

auto MovePointsTool_MoveRotate::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    auto start = dynamic_cast<CalChart::Shape_1point const&>(*m_shape_list[0]).GetOrigin();
    auto [start_x, start_y] = CoordPair_cast<double>(start);
    auto r = -((CalChart::Shape_arc*)m_shape_list.back().get())->GetAngle();
    auto m = TranslationMatrix{ Vector<double>{ -start_x, -start_y, 0 } }
        * ZRotationMatrix(r.getValue())
        * TranslationMatrix{ Vector<double>{ start_x, start_y, 0 } };
    return GetTransformedPoints(m, select_list);
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

auto MovePointsTool_MoveShear::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
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
    double amount = v2.Magnitude() / v1.Magnitude();
    if (BoundDirectionSigned(v1.Direction() - (c2 - start).Direction()) < CalChart::Radian{}) {
        amount = -amount;
    }
    auto ang = -v1.Direction();
    auto [start_x, start_y] = CoordPair_cast<double>(start);
    auto m = TranslationMatrix{ Vector<double>{ -start_x, -start_y, 0 } }
        * ZRotationMatrix(-ang.getValue())
        * YXShearMatrix(amount)
        * ZRotationMatrix(ang.getValue())
        * TranslationMatrix{ Vector<double>{ start_x, start_y, 0 } };
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

auto MovePointsTool_MoveReflect::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto c1 = shape->GetOrigin();
    auto [c1_x, c1_y] = CoordPair_cast<double>(c1);
    auto c2 = shape->GetPoint() - c1;
    auto ang = -c2.Direction();
    auto m = TranslationMatrix{ Vector<double>{ -c1_x, -c1_y, 0 } }
        * ZRotationMatrix(-ang.getValue())
        * YReflectionMatrix<double>()
        * ZRotationMatrix(ang.getValue())
        * TranslationMatrix{ Vector<double>{ c1_x, c1_y, 0 } };
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

auto MovePointsTool_MoveSize::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    if (m_shape_list.size() < 2) {
        return select_list;
    }
    auto& origin = dynamic_cast<CalChart::Shape_1point&>(*m_shape_list[0]);
    auto c1 = origin.GetOrigin();
    auto [c1_x, c1_y] = CoordPair_cast<float>(c1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto c2 = shape->GetPoint() - c1;
    auto [sx, sy] = CoordPair_cast<float>(c2);
    c2 = shape->GetOrigin() - c1;
    if ((c2.x != 0) || (c2.y != 0)) {
        if (c2.x != 0) {
            sx = sx / static_cast<float>(c2.x);
        } else {
            sx = 1;
        }
        if (c2.y != 0) {
            sy = sy / static_cast<float>(c2.y);
        } else {
            sy = 1;
        }
        auto m = TranslationMatrix{ Vector<float>{ -c1_x, -c1_y, 0 } }
            * ScaleMatrix{ Vector<float>{ sx, sy, 0 } }
            * TranslationMatrix{ Vector<float>{ c1_x, c1_y, 0 } };
        return GetTransformedPoints(m, select_list);
    }
    return {};
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

auto MovePointsTool_MoveSize::IsDone() const -> bool
{
    return IsReadyForMoving();
}

auto MovePointsTool_MoveSize::IsReadyForMoving() const -> bool
{
    return m_shape_list.size() > 1;
}

void MovePointsTool_MoveGenius::OnClickDown(CalChart::Coord pos)
{
    // move is a 2-distinct mouse click action
    if (m_shape_list.size() < 3) {
        AddMoveDrag(Drag::LINE, std::make_unique<CalChart::Shape_line>(pos));
    }
}

auto MovePointsTool_MoveGenius::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    if (m_shape_list.size() < 3) {
        return select_list;
    }

    auto* v1 = (CalChart::Shape_2point*)m_shape_list[0].get();
    auto* v2 = (CalChart::Shape_2point*)m_shape_list[1].get();
    auto* v3 = (CalChart::Shape_2point*)m_shape_list[2].get();

    auto [s1_x, s1_y] = CoordPair_cast<float>(v1->GetOrigin());
    auto [e1_x, e1_y] = CoordPair_cast<float>(v1->GetPoint());
    auto [s2_x, s2_y] = CoordPair_cast<float>(v2->GetOrigin());
    auto [e2_x, e2_y] = CoordPair_cast<float>(v2->GetPoint());
    auto [s3_x, s3_y] = CoordPair_cast<float>(v3->GetOrigin());
    auto [e3_x, e3_y] = CoordPair_cast<float>(v3->GetPoint());
    auto d = s1_x * s2_y
        - s2_x * s1_y
        + s3_x * s1_y
        - s1_x * s3_y
        + s2_x * s3_y
        - s3_x * s2_y;
    if (IS_ZERO(d)) {
        return {};
    }
    auto A = Matrix{
        Vector<float>{ e1_x, e2_x, 0, e3_x },
        Vector<float>{ e1_y, e2_y, 0, e3_y },
        Vector<float>{ 0, 0, 0, 0 },
        Vector<float>{ 1, 1, 0, 1 }
    };
    auto Binv = Matrix{
        Vector<float>{ s2_y - s3_y, s3_x - s2_x, 0, s2_x * s3_y - s3_x * s2_y },
        Vector<float>{ s3_y - s1_y, s1_x - s3_x, 0, s3_x * s1_y - s1_x * s3_y },
        Vector<float>{ 0, 0, 0, 0 },
        Vector<float>{ s1_y - s2_y, s2_x - s1_x, 0, s1_x * s2_y - s2_x * s1_y }
    };
    Binv /= d;
    Matrix m = Binv * A;
    return GetTransformedPoints(m, select_list);
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

auto MovePointsTool_ShapeLine::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    auto start = shape->GetOrigin();
    auto second = shape->GetPoint();
    auto curr_pos = start;
    MarcherToPosition result;
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

auto MovePointsTool_ShapeEllipse::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto center = start + (end - start) / 2;
    const auto a = (end.x - start.x) / 2.0;
    const auto b = (end.y - start.y) / 2.0;
    MarcherToPosition result;
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

auto MovePointsTool_ShapeRectangle::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start = shape->GetOrigin();
    const auto end = shape->GetPoint();
    const auto a = (end.x - start.x);
    const auto b = (end.y - start.y);
    const auto perimeter = a * 2.0 + b * 2.0;
    MarcherToPosition result;
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

auto MovePointsTool_ShapeDraw::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Lasso const*)m_shape_list.back().get();
    MarcherToPosition result;
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

auto MovePointsTool_ShapeX::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
{
    assert(m_shape_list.size() == 1);
    auto* shape = (CalChart::Shape_2point const*)m_shape_list.back().get();
    const auto start1 = shape->GetOrigin();
    const auto end1 = shape->GetPoint();
    const auto start2 = CalChart::Coord(start1.x, end1.y);
    const auto end2 = CalChart::Coord(end1.x, start1.y);
    auto num_squares = (select_list.size() + 2) / 4;
    auto iter = select_list.begin();

    MarcherToPosition result;
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

auto MovePointsTool_ShapeCross::TransformPoints(MarcherToPosition const& select_list) const -> MarcherToPosition
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

    MarcherToPosition result;
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
