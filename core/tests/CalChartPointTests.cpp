#include "CalChartPoint.h"
#include <catch2/catch_test_macros.hpp>

using namespace CalChart;

namespace {
// Test Suite stuff
struct Point_values {
    SYMBOL_TYPE mSym{ SYMBOL_PLAIN };
    Coord mPos{};
    std::array<Coord, Point::kNumRefPoints> mRef{};
    bool GetFlip{};
    bool Visable{};
};

auto Check_Point(Point const& underTest, Point_values const& values) -> bool
{
    return underTest.GetPos() == values.mPos && underTest.GetPos(1) == values.mRef.at(0) && underTest.GetPos(2) == values.mRef.at(1) && underTest.GetPos(3) == values.mRef.at(2) && underTest.GetSymbol() == values.mSym && underTest.GetFlip() == values.GetFlip && underTest.LabelIsVisible() == values.Visable;
}
}

TEST_CASE("CalChartPointTests")
{
    // test some defaults:
    Point_values values{};
    values.mSym = SYMBOL_PLAIN;
    values.Visable = true;

    // test defaults
    Point underTest;
    CHECK(Check_Point(underTest, values));

    // test flip
    underTest.Flip(false);
    CHECK(Check_Point(underTest, values));

    values.GetFlip = true;
    underTest.Flip(true);
    CHECK(Check_Point(underTest, values));

    values.GetFlip = false;
    underTest.Flip(false);
    CHECK(Check_Point(underTest, values));

    // test visability
    underTest.SetLabelVisibility(true);
    CHECK(Check_Point(underTest, values));

    values.Visable = false;
    underTest.SetLabelVisibility(false);
    CHECK(Check_Point(underTest, values));

    values.Visable = true;
    underTest.SetLabelVisibility(true);
    CHECK(Check_Point(underTest, values));
}
