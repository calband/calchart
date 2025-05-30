#include "CalChartFileFormat.h"
#include "CalChartShapes.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

static auto trucate(CalChart::Coord c)
{
    return (c / 5) * 5;
}

TEST_CASE("CalChartShapeTests", "Shape_crosshairs")
{
    auto uut = CalChart::Shape_crosshairs({ 10, 10 }, 10);
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(20, 20, 40, 40),
              CalChart::Draw::Line(40, 20, 20, 40),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(60, 60, 80, 80),
              CalChart::Draw::Line(80, 60, 60, 80),
          });
}

TEST_CASE("Shape_line", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_line(CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 70, 70),
          });
    uut = CalChart::Shape_line(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 70, 70),
          });
}

TEST_CASE("Shape_x", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_x(CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 30, 30),
              CalChart::Draw::Line(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 70, 70),
              CalChart::Draw::Line(70, 30, 30, 70),
          });
    uut = CalChart::Shape_x(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 70, 70),
              CalChart::Draw::Line(70, 30, 30, 70),
          });
}

TEST_CASE("Shape_cross", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_cross(CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 30, 30),
              CalChart::Draw::Line(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(50, 30, 50, 70),
              CalChart::Draw::Line(30, 50, 70, 50),
          });
    uut = CalChart::Shape_cross(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(50, 30, 50, 70),
              CalChart::Draw::Line(30, 50, 70, 50),
          });
}

TEST_CASE("Shape_ellipse", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_ellipse(CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Ellipse(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Ellipse(30, 30, 70, 70),
          });
    uut = CalChart::Shape_ellipse(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Ellipse(30, 30, 70, 70),
          });
}

TEST_CASE("Shape_angline", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_angline(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 49, 49 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 69, 69),
          });
    uut.OnMove({ 51, 100 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 74, 74 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 94, 94),
          });
    uut.OnMove({ 51, 0 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 24, 24 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 44, 44),
          });
}

TEST_CASE("Shape_arc", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_arc(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 20, 20 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(40, 39, 40, 39, 30, 30),
          });
    uut.OnMove({ 51, 53 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(40, 39, 40, 39, 30, 30),
          });
    uut.OnMove({ 51, 100 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(35, 42, 40, 39, 30, 30),
          });
}

TEST_CASE("Shape_arc2", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_arc(CalChart::Coord{ 20, 20 }, CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 20, 20 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(30, 31, 30, 31, 40, 40),
          });
    uut.OnMove({ 51, 53 }, trucate);
    auto peek = uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 };
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(50, 49, 30, 31, 40, 40),
          });
    uut.OnMove({ 51, 100 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(30, 31, 44, 53, 40, 40),
          });
}

TEST_CASE("Shape_arcquad0", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 10, 20 };
    auto uut = CalChart::Shape_arc(center, p1);
    CHECK((uut.GetCC_DrawCommand()) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(p1, p1, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 20, 20 }, CalChart::Draw::Arc{ { 10, 20 }, { 17, 17 }, center } },
             std::tuple{ CalChart::Coord{ 20, 0 }, CalChart::Draw::Arc{ { 10, 20 }, { 17, 3 }, center } },
             std::tuple{ CalChart::Coord{ 0, 20 }, CalChart::Draw::Arc{ { 3, 17 }, { 10, 20 }, center } },
             std::tuple{ CalChart::Coord{ 0, 0 }, CalChart::Draw::Arc{ { 3, 3 }, { 10, 20 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad2", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 20, 10 };
    auto uut = CalChart::Shape_arc(center, p1);
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(p1, p1, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 20, 20 }, CalChart::Draw::Arc{ { 17, 17 }, { 20, 10 }, center } },
             std::tuple{ CalChart::Coord{ 20, 0 }, CalChart::Draw::Arc{ { 20, 10 }, { 17, 3 }, center } },
             std::tuple{ CalChart::Coord{ 0, 20 }, CalChart::Draw::Arc{ { 3, 17 }, { 20, 10 }, center } },
             std::tuple{ CalChart::Coord{ 0, 0 }, CalChart::Draw::Arc{ { 20, 10 }, { 3, 3 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad4", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 0, 10 };
    auto uut = CalChart::Shape_arc(center, p1);
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(p1, p1, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 20, 20 }, CalChart::Draw::Arc{ { 0, 10 }, { 17, 17 }, center } },
             std::tuple{ CalChart::Coord{ 20, 0 }, CalChart::Draw::Arc{ { 17, 3 }, { 0, 10 }, center } },
             std::tuple{ CalChart::Coord{ 0, 20 }, CalChart::Draw::Arc{ { 0, 10 }, { 3, 17 }, center } },
             std::tuple{ CalChart::Coord{ 0, 0 }, CalChart::Draw::Arc{ { 3, 3 }, { 0, 10 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad6", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 10, 0 };
    auto uut = CalChart::Shape_arc(center, p1);
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc(p1, p1, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 20, 20 }, CalChart::Draw::Arc{ { 17, 17 }, { 10, 0 }, center } },
             std::tuple{ CalChart::Coord{ 20, 0 }, CalChart::Draw::Arc{ { 17, 3 }, { 10, 0 }, center } },
             std::tuple{ CalChart::Coord{ 0, 20 }, CalChart::Draw::Arc{ { 10, 0 }, { 3, 17 }, center } },
             std::tuple{ CalChart::Coord{ 0, 0 }, CalChart::Draw::Arc{ { 10, 0 }, { 3, 3 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad1", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 20, 20 };
    auto uut = CalChart::Shape_arc(center, p1);
    auto peek = uut.GetCC_DrawCommand();
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc({ 20, 19 }, { 20, 19 }, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 10, 20 }, CalChart::Draw::Arc{ { 10, 24 }, { 20, 19 }, center } },
             std::tuple{ CalChart::Coord{ 20, 10 }, CalChart::Draw::Arc{ { 20, 19 }, { 24, 10 }, center } },
             std::tuple{ CalChart::Coord{ 0, 10 }, CalChart::Draw::Arc{ { -4, 10 }, { 20, 19 }, center } },
             std::tuple{ CalChart::Coord{ 10, 0 }, CalChart::Draw::Arc{ { 20, 19 }, { 10, -4 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad3", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 0, 20 };
    auto uut = CalChart::Shape_arc(center, p1);
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc({ 0, 19 }, { 0, 19 }, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 20, 10 }, CalChart::Draw::Arc{ { 0, 19 }, { 24, 10 }, center } },
             std::tuple{ CalChart::Coord{ 10, 20 }, CalChart::Draw::Arc{ { 0, 19 }, { 10, 24 }, center } },
             std::tuple{ CalChart::Coord{ 0, 10 }, CalChart::Draw::Arc{ { -4, 10 }, { 0, 19 }, center } },
             std::tuple{ CalChart::Coord{ 10, 0 }, CalChart::Draw::Arc{ { 10, -4 }, { 0, 19 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad5", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 20, 0 };
    auto uut = CalChart::Shape_arc(center, p1);
    auto peek = uut.GetCC_DrawCommand();
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc({ 20, 1 }, { 20, 1 }, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 10, 20 }, CalChart::Draw::Arc{ { 10, 24 }, { 20, 1 }, center } },
             std::tuple{ CalChart::Coord{ 20, 10 }, CalChart::Draw::Arc{ { 24, 10 }, { 20, 1 }, center } },
             std::tuple{ CalChart::Coord{ 0, 10 }, CalChart::Draw::Arc{ { 20, 1 }, { -4, 10 }, center } },
             std::tuple{ CalChart::Coord{ 10, 0 }, CalChart::Draw::Arc{ { 20, 1 }, { 10, -4 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_arcquad7", "CalChartShapeTests")
{
    auto const center = CalChart::Coord{ 10, 10 };
    auto const p1 = CalChart::Coord{ 0, 0 };
    auto uut = CalChart::Shape_arc(center, p1);
    auto peek = uut.GetCC_DrawCommand();
    CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Arc({ 0, 1 }, { 0, 1 }, center),
          });
    for (auto&& i : std::vector{
             std::tuple{ CalChart::Coord{ 10, 20 }, CalChart::Draw::Arc{ { 0, 1 }, { 10, 24 }, center } },
             std::tuple{ CalChart::Coord{ 20, 10 }, CalChart::Draw::Arc{ { 24, 10 }, { 0, 1 }, center } },
             std::tuple{ CalChart::Coord{ 0, 10 }, CalChart::Draw::Arc{ { 0, 1 }, { -4, 10 }, center } },
             std::tuple{ CalChart::Coord{ 10, 0 }, CalChart::Draw::Arc{ { 10, -4 }, { 0, 1 }, center } },
         }) {
        uut.OnMove(std::get<0>(i), trucate);
        auto peek = uut.GetCC_DrawCommand();
        CHECK(uut.GetCC_DrawCommand() == std::vector<CalChart::Draw::DrawCommand>{ std::get<1>(i) });
    }
}

TEST_CASE("Shape_rect", "CalChartShapeTests")
{
    auto uut = CalChart::Shape_rect(CalChart::Coord{ 10, 10 });
    CHECK(uut.GetOrigin() == CalChart::Coord{ 10, 10 });
    CHECK(uut.GetPoint() == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 30, 30),
              CalChart::Draw::Line(30, 30, 30, 30),
              CalChart::Draw::Line(30, 30, 30, 30),
              CalChart::Draw::Line(30, 30, 30, 30),
          });
    uut.OnMove({ 51, 49 }, trucate);
    CHECK(uut.GetPoint() == CalChart::Coord{ 51, 49 });
    auto result = uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 };
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 30),
              CalChart::Draw::Line(71, 30, 71, 69),
              CalChart::Draw::Line(71, 69, 30, 69),
              CalChart::Draw::Line(30, 69, 30, 30),
          });
    uut = CalChart::Shape_rect(CalChart::Coord{ 10, 10 }, CalChart::Coord{ 50, 50 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 70, 30),
              CalChart::Draw::Line(70, 30, 70, 70),
              CalChart::Draw::Line(70, 70, 30, 70),
              CalChart::Draw::Line(30, 70, 30, 30),
          });
}

TEST_CASE("Shape_lasso", "CalChartShapeTests")
{
    auto uut = CalChart::Lasso(CalChart::Coord{ 10, 10 });
    CHECK(*(uut.FirstPoint()) == CalChart::Coord{ 10, 10 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }).empty());
    uut.OnMove({ 51, 49 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 69),
          });
    CHECK(abs(CalChart::GetDistance(uut.GetPolygon()) - 56.586216) < 0.0001);
    uut.Append({ 51, 49 });
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 69),
              CalChart::Draw::Line(71, 69, 71, 69),
          });
    CHECK(abs(CalChart::GetDistance(uut.GetPolygon()) - 56.586216) < 0.0001);
    uut.OnMove({ 5, 6 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 69),
              CalChart::Draw::Line(71, 69, 71, 69),
              CalChart::Draw::Line(71, 69, 25, 26),
          });
    CHECK(abs(CalChart::GetDistance(uut.GetPolygon()) - 119.554459) < 0.0001);
}

TEST_CASE("Shape_Poly", "CalChartShapeTests")
{
    auto uut = CalChart::Poly(CalChart::Coord{ 10, 10 });
    uut.OnMove({ 51, 49 }, trucate);
    uut.Append({ 51, 49 });
    uut.OnMove({ 5, 6 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 69),
              CalChart::Draw::Line(71, 69, 25, 26),
          });
    uut.OnMove({ 5, 6 }, trucate);
    CHECK((uut.GetCC_DrawCommand() + CalChart::Coord{ 20, 20 }) == std::vector<CalChart::Draw::DrawCommand>{
              CalChart::Draw::Line(30, 30, 71, 69),
              CalChart::Draw::Line(71, 69, 25, 26),
          });
}

TEST_CASE("Shape_PolyPoint", "CalChartShapeTests")
{
    auto uut = CalChart::Poly(CalChart::Coord{ 10, 10 });
    uut.Append({ 51, 49 });
    uut.Append({ 10, 49 });
    uut.Append({ 51, 10 });
    CHECK(abs(CalChart::GetDistance(uut.GetPolygon()) - 154.172424) < 0.0001);
    auto result = uut.GetPointsOnLine(1);
    CHECK(uut.GetPointsOnLine(1) == std::vector<CalChart::Coord>{ { 10, 10 } });
    result = uut.GetPointsOnLine(2);
    CHECK(uut.GetPointsOnLine(2) == std::vector<CalChart::Coord>{ { 10, 10 }, { 50, 11 } });
    result = uut.GetPointsOnLine(4);
    CHECK(uut.GetPointsOnLine(4) == std::vector<CalChart::Coord>{ { 10, 10 }, { 47, 45 }, { 13, 46 }, { 50, 11 } });
    result = uut.GetPointsOnLine(8);
    CHECK(uut.GetPointsOnLine(8) == std::vector<CalChart::Coord>{ { 10, 10 }, { 25, 25 }, { 41, 39 }, { 44, 49 }, { 22, 49 }, { 17, 43 }, { 32, 28 }, { 47, 13 } });
    result = uut.GetPointsOnLine(16);
    CHECK(uut.GetPointsOnLine(16) == std::vector<CalChart::Coord>{ { 10, 10 }, { 17, 17 }, { 24, 24 }, { 31, 30 }, { 38, 37 }, { 45, 43 }, { 50, 49 }, { 40, 49 }, { 30, 49 }, { 20, 49 }, { 10, 49 }, { 17, 42 }, { 24, 35 }, { 31, 29 }, { 38, 22 }, { 45, 16 } });
}

TEST_CASE("Shape_PolyPoint2", "CalChartShapeTests")
{
    auto uut = CalChart::Poly(CalChart::Coord{ 10, 10 });
    CHECK(abs(CalChart::GetDistance(uut.GetPolygon()) - 0) < 0.0001);
    CHECK(uut.GetPointsOnLine(1) == std::vector<CalChart::Coord>{ { 10, 10 } });
    CHECK(uut.GetPointsOnLine(2) == std::vector<CalChart::Coord>{ { 10, 10 }, { 10, 10 } });
    CHECK(uut.GetPointsOnLine(4) == std::vector<CalChart::Coord>{ { 10, 10 }, { 10, 10 }, { 10, 10 }, { 10, 10 } });
}

TEST_CASE("Shape_PolyPoint3", "CalChartShapeTests")
{
    auto uut = CalChart::Poly(CalChart::Coord{ 0, 0 });
    uut.Append({ 2, 0 });
    uut.Append({ 4, 0 });
    uut.Append({ 8, 0 });
    uut.Append({ 16, 0 });
    uut.Append({ 32, 0 });
    uut.Append({ 64, 0 });
    uut.Append({ 128, 0 });
    CHECK(CalChart::GetDistance(uut.GetPolygon()) == 128);
    auto result = uut.GetPointsOnLine(1);
    CHECK(uut.GetPointsOnLine(1) == std::vector<CalChart::Coord>{ { 0, 0 } });
    result = uut.GetPointsOnLine(2);
    CHECK(uut.GetPointsOnLine(2) == std::vector<CalChart::Coord>{ { 0, 0 }, { 128, 0 } });
    result = uut.GetPointsOnLine(3);
    CHECK(uut.GetPointsOnLine(3) == std::vector<CalChart::Coord>{ { 0, 0 }, { 64, 0 }, { 128, 0 } });
    result = uut.GetPointsOnLine(5);
    CHECK(uut.GetPointsOnLine(5) == std::vector<CalChart::Coord>{ { 0, 0 }, { 32, 0 }, { 64, 0 }, { 96, 0 }, { 128, 0 } });
}

TEST_CASE("Shape_PolyPoint4", "CalChartShapeTests")
{
    auto uut = CalChart::Poly(CalChart::Coord{ 0, 0 });
    uut.Append({ 2, 0 });
    uut.Append({ 16, 0 });
    uut.Append({ 128, 0 });
    CHECK(CalChart::GetDistance(uut.GetPolygon()) == 128);
    auto result = uut.GetPointsOnLine(1);
    CHECK(uut.GetPointsOnLine(1) == std::vector<CalChart::Coord>{ { 0, 0 } });
    result = uut.GetPointsOnLine(2);
    CHECK(uut.GetPointsOnLine(2) == std::vector<CalChart::Coord>{ { 0, 0 }, { 128, 0 } });
    result = uut.GetPointsOnLine(3);
    CHECK(uut.GetPointsOnLine(3) == std::vector<CalChart::Coord>{ { 0, 0 }, { 64, 0 }, { 128, 0 } });
    result = uut.GetPointsOnLine(5);
    CHECK(uut.GetPointsOnLine(5) == std::vector<CalChart::Coord>{ { 0, 0 }, { 32, 0 }, { 64, 0 }, { 96, 0 }, { 128, 0 } });
}

TEST_CASE("Test_CrossesLine", "CalChartShapeTests")
{
    CHECK(!CalChart::CrossesLine({ 5, 5 }, { 10, 10 }, { 0, 0 }));
    CHECK(!CalChart::CrossesLine({ 5, 5 }, { 10, 10 }, { 15, 15 }));
    CHECK(!CalChart::CrossesLine({ 10, 10 }, { 5, 5 }, { 0, 0 }));
    CHECK(!CalChart::CrossesLine({ 10, 10 }, { 5, 5 }, { 15, 15 }));
    CHECK(!CalChart::CrossesLine({ 5, 5 }, { 10, 10 }, { 0, 7 }));
    CHECK(CalChart::CrossesLine({ 5, 5 }, { 10, 10 }, { 15, 7 }));
    CHECK(!CalChart::CrossesLine({ 10, 10 }, { 5, 5 }, { 0, 7 }));
    CHECK(CalChart::CrossesLine({ 10, 10 }, { 5, 5 }, { 15, 7 }));
}

TEST_CASE("Test_Inside", "CalChartShapeTests")
{
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t()));
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 } }));
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 } }));
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 } }));
    CHECK(CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 10, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 0, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 5, 0 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 1, 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 1, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 1, 9 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 5, 9 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 9, 9 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 9, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 9, 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 5, 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 9, 1 } }));
    // a more unusual shape with inner blank spots.  Using math here to make it clear where we are going into boundaries
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 1 + 1, 1 + 2 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 1 + 2, 1 + 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 9 - 1, 9 - 2 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 9 - 2, 9 - 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 1 + 1, 9 - 2 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 1 + 2, 9 - 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(!CalChart::Inside({ 9 - 1, 1 + 2 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));
    CHECK(CalChart::Inside({ 9 - 2, 1 + 1 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 9, 9 }, CalChart::Coord{ 1, 9 }, CalChart::Coord{ 9, 1 } }));

    // a star pattern
    CHECK(!CalChart::Inside({ 5, 5 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 5, 9 }, CalChart::Coord{ 9, 1 }, CalChart::Coord{ 1, 6 }, CalChart::Coord{ 1, 8 }, CalChart::Coord{ 9, 8 }, CalChart::Coord{ 9, 6 } }));
    CHECK(CalChart::Inside({ 2, 7 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 5, 9 }, CalChart::Coord{ 9, 1 }, CalChart::Coord{ 1, 6 }, CalChart::Coord{ 1, 8 }, CalChart::Coord{ 9, 8 }, CalChart::Coord{ 9, 6 } }));
    CHECK(CalChart::Inside({ 8, 7 }, CalChart::RawPolygon_t{ CalChart::Coord{ 1, 1 }, CalChart::Coord{ 5, 9 }, CalChart::Coord{ 9, 1 }, CalChart::Coord{ 1, 6 }, CalChart::Coord{ 1, 8 }, CalChart::Coord{ 9, 8 }, CalChart::Coord{ 9, 6 } }));
}

TEST_CASE("Shape_Curve", "CalChartShapeTests")
{
    auto controlPoints = std::vector<CalChart::Coord>{
        { 10, 10 },
        { 500, 400 },
        { 10, 400 },
        { 500, 10 }
    };
    auto uut = CalChart::Curve(controlPoints);
    CHECK(controlPoints == uut.GetControlPoints());
    auto drawCommands = uut.GetCC_DrawCommand();
    // for reference, this is how we would print out the draw commands if they need to be regenerated:
    // for (auto&& cmd : drawCommands) {
    //     std::visit(
    //         CalChart::overloaded{
    //             [](CalChart::Draw::DrawItems item) {
    //                 std::visit(
    //                     CalChart::overloaded{
    //                         [](CalChart::Draw::Line line) { std::cout << "CalChart::Draw::Line{ CalChart::Coord{ " << line.c1.x << ", " << line.c1.y << "}, CalChart::Coord{ " << line.c2.x << ", " << line.c2.y << "} },\n"; },
    //                         [](auto&&) {},
    //                     },
    //                     item);
    //             },
    //             [](auto&&) {},
    //         },
    //         cmd);
    // }
    auto uut_draw = std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::Line{ CalChart::Coord{ 10, 10 }, CalChart::Coord{ 43, 35 } },
        CalChart::Draw::Line{ CalChart::Coord{ 43, 35 }, CalChart::Coord{ 93, 70 } },
        CalChart::Draw::Line{ CalChart::Coord{ 93, 70 }, CalChart::Coord{ 152, 111 } },
        CalChart::Draw::Line{ CalChart::Coord{ 152, 111 }, CalChart::Coord{ 218, 156 } },
        CalChart::Draw::Line{ CalChart::Coord{ 218, 156 }, CalChart::Coord{ 286, 205 } },
        CalChart::Draw::Line{ CalChart::Coord{ 286, 205 }, CalChart::Coord{ 352, 254 } },
        CalChart::Draw::Line{ CalChart::Coord{ 352, 254 }, CalChart::Coord{ 409, 299 } },
        CalChart::Draw::Line{ CalChart::Coord{ 409, 299 }, CalChart::Coord{ 457, 340 } },
        CalChart::Draw::Line{ CalChart::Coord{ 457, 340 }, CalChart::Coord{ 488, 374 } },
        CalChart::Draw::Line{ CalChart::Coord{ 488, 374 }, CalChart::Coord{ 500, 400 } },
        CalChart::Draw::Line{ CalChart::Coord{ 500, 400 }, CalChart::Coord{ 486, 417 } },
        CalChart::Draw::Line{ CalChart::Coord{ 486, 417 }, CalChart::Coord{ 448, 431 } },
        CalChart::Draw::Line{ CalChart::Coord{ 448, 431 }, CalChart::Coord{ 394, 441 } },
        CalChart::Draw::Line{ CalChart::Coord{ 394, 441 }, CalChart::Coord{ 328, 447 } },
        CalChart::Draw::Line{ CalChart::Coord{ 328, 447 }, CalChart::Coord{ 255, 450 } },
        CalChart::Draw::Line{ CalChart::Coord{ 255, 450 }, CalChart::Coord{ 181, 447 } },
        CalChart::Draw::Line{ CalChart::Coord{ 181, 447 }, CalChart::Coord{ 116, 441 } },
        CalChart::Draw::Line{ CalChart::Coord{ 116, 441 }, CalChart::Coord{ 60, 431 } },
        CalChart::Draw::Line{ CalChart::Coord{ 60, 431 }, CalChart::Coord{ 23, 417 } },
        CalChart::Draw::Line{ CalChart::Coord{ 23, 417 }, CalChart::Coord{ 10, 400 } },
        CalChart::Draw::Line{ CalChart::Coord{ 10, 400 }, CalChart::Coord{ 21, 374 } },
        CalChart::Draw::Line{ CalChart::Coord{ 21, 374 }, CalChart::Coord{ 53, 340 } },
        CalChart::Draw::Line{ CalChart::Coord{ 53, 340 }, CalChart::Coord{ 101, 299 } },
        CalChart::Draw::Line{ CalChart::Coord{ 101, 299 }, CalChart::Coord{ 159, 254 } },
        CalChart::Draw::Line{ CalChart::Coord{ 159, 254 }, CalChart::Coord{ 224, 205 } },
        CalChart::Draw::Line{ CalChart::Coord{ 224, 205 }, CalChart::Coord{ 293, 156 } },
        CalChart::Draw::Line{ CalChart::Coord{ 293, 156 }, CalChart::Coord{ 358, 111 } },
        CalChart::Draw::Line{ CalChart::Coord{ 358, 111 }, CalChart::Coord{ 418, 70 } },
        CalChart::Draw::Line{ CalChart::Coord{ 418, 70 }, CalChart::Coord{ 466, 35 } },
        CalChart::Draw::Line{ CalChart::Coord{ 466, 35 }, CalChart::Coord{ 500, 10 } },
    };
    CHECK(uut_draw == drawCommands);

    {
        auto found = uut.LowerControlPointOnLine({ 10, 10 }, 1);
        CHECK(found.has_value());
        REQUIRE(std::get<0>(*found) == 0);
        REQUIRE(std::get<1>(*found) == Catch::Approx(0.0));
    }
    {
        auto found = uut.LowerControlPointOnLine({ 159, 254 }, 1);
        CHECK(found.has_value());
        REQUIRE(std::get<0>(*found) == 2);
        REQUIRE(std::get<1>(*found) == Catch::Approx(0.722436));
    }
}

TEST_CASE("Shape_Curve_SerializeDeserialize", "CalChartShapeTests")
{
    {
        auto uut = CalChart::Curve(std::vector<CalChart::Coord>{
            { 10, 10 },
            { 500, 400 },
            { 10, 400 },
            { 500, 10 } });
        auto serialized = uut.Serialize();
        auto reader = CalChart::Reader({ serialized.data(), serialized.size() });
        auto [curve, newReader] = CalChart::CreateCurve(reader);
        CHECK(curve == uut);
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
