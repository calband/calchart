#include "CalChartCoord.h"
#include <catch2/catch_test_macros.hpp>
#include <map>
#include <random>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
TEST_CASE("Basic", "CalChartCoord")
{
    CalChart::Coord const undertest;
    REQUIRE(0.0 == undertest.Magnitude());
    REQUIRE(0.0 == undertest.DM_Magnitude());
    REQUIRE(CalChart::Radian{ 0.0 } == undertest.Direction());

    auto undertest2 = CalChart::Coord{ 16, 16 };
    CHECK(CalChart::IS_ZERO(std::numbers::sqrt2 - undertest2.Magnitude()));
    CHECK(1.0F == undertest2.DM_Magnitude());
    CHECK(undertest2.Direction().IsEqual(-CalChart::pi / 4));
}

TEST_CASE("CreateCalChartVectorDeg", "CalChartCoord")
{
    for (auto [dir, mag, v] : std::vector<std::tuple<double, double, CalChart::Coord>>{
             { 0, 1, CalChart::Coord{ 16, 0 } },
             { 10, 1, CalChart::Coord{ 16, -3 } },
             { 44, 1, CalChart::Coord{ 12, -11 } },
             { 45, 1, CalChart::Coord{ 16, -16 } },
             { 90, 1, CalChart::Coord{ 0, -16 } },
             { 135, 1, CalChart::Coord{ -16, -16 } },
             { 180, 1, CalChart::Coord{ -16, 0 } },
             { 225, 1, CalChart::Coord{ -16, 16 } },
             { 270, 1, CalChart::Coord{ 0, 16 } },
             { 315, 1, CalChart::Coord{ 16, 16 } },
             { 355, 1, CalChart::Coord{ 16, 1 } },
             { 360, 1, CalChart::Coord{ 16, 0 } },
         }) {
        auto unit = CalChart::CreateCalChartVector(CalChart::Degree{ dir }, 1);
        CHECK(unit == v);
    }
}

TEST_CASE("Random", "CalChartCoord")
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<CalChart::Coord::units> distrib;
    // test equality:
    static constexpr auto kNumRand = 10;
    for (size_t i = 0; i < kNumRand; ++i) {
        auto const x1 = distrib(gen);
        auto const y1 = distrib(gen);
        auto const x2 = distrib(gen);
        auto const y2 = distrib(gen);
        CalChart::Coord undertest1(x1, y1);
        CalChart::Coord undertest2(x2, y2);
        CHECK(x1 == undertest1.x);
        CHECK(y1 == undertest1.y);
        CHECK(x2 == undertest2.x);
        CHECK(y2 == undertest2.y);
        CHECK(CalChart::Coord(x1, y1) == undertest1);
        CHECK(CalChart::Coord(x2, y2) == undertest2);
        CHECK(!(CalChart::Coord(x1, y1) != undertest1));
        CHECK(!(CalChart::Coord(x2, y2) != undertest2));

        auto newValue = undertest1 + undertest2;
        auto x3 = static_cast<CalChart::Coord::units>(undertest1.x + undertest2.x);
        auto y3 = static_cast<CalChart::Coord::units>(undertest1.y + undertest2.y);
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue += undertest2;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1 - undertest2;
        x3 = static_cast<CalChart::Coord::units>(undertest1.x - undertest2.x);
        y3 = static_cast<CalChart::Coord::units>(undertest1.y - undertest2.y);
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue -= undertest2;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);

        newValue = -undertest1;
        CHECK((-undertest1.x) == newValue.x);
        CHECK((-undertest1.y) == newValue.y);

        auto const factor = distrib(gen);
        newValue = undertest1 * factor;
        x3 = static_cast<CalChart::Coord::units>(undertest1.x * factor);
        y3 = static_cast<CalChart::Coord::units>(undertest1.y * factor);
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue *= factor;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        // avoid div by 0
        if (factor != 0) {
            newValue = undertest1 / factor;
            x3 = static_cast<CalChart::Coord::units>(undertest1.x / factor);
            y3 = static_cast<CalChart::Coord::units>(undertest1.y / factor);
            CHECK((x3) == newValue.x);
            CHECK((y3) == newValue.y);
            newValue = undertest1;
            newValue /= factor;
            CHECK((x3) == newValue.x);
            CHECK((y3) == newValue.y);
        }
    }
}

TEST_CASE("Directions", "CalChartCoord")
{
    CHECK(CalChart::Coord{ 0, 0 }.Direction().IsEqual(CalChart::Radian{ 0.0 }));
    CHECK(CalChart::Coord{ 1, 0 }.Direction().IsEqual(CalChart::Radian{ 0.0 }));
    CHECK(CalChart::Coord{ 1, 1 }.Direction().IsEqual(-CalChart::pi / 4.0));
    CHECK(CalChart::Coord{ 0, 1 }.Direction().IsEqual(-CalChart::pi / 2.0));
    CHECK(CalChart::Coord{ -1, 1 }.Direction().IsEqual(-3 * CalChart::pi / 4.0));
    CHECK(CalChart::Coord{ -1, 0 }.Direction().IsEqual(CalChart::pi));
    CHECK(CalChart::Coord{ -1, -1 }.Direction().IsEqual(3 * CalChart::pi / 4.0));
    CHECK(CalChart::Coord{ 0, -1 }.Direction().IsEqual(CalChart::pi / 2.0));
    CHECK(CalChart::Coord{ 1, -1 }.Direction().IsEqual(CalChart::pi / 4.0));
}

TEST_CASE("DirectionsTo", "CalChartCoord")
{
    auto uut = CalChart::Coord{ 1, 1 };
    CHECK(uut.Direction(CalChart::Coord{ 1 + 0, 1 + 0 }).IsEqual(CalChart::Radian{ 0.0 }));
    CHECK(uut.Direction(CalChart::Coord{ 1 + 1, 1 + 0 }).IsEqual(CalChart::Radian{ 0.0 }));
    CHECK(uut.Direction(CalChart::Coord{ 1 + 1, 1 + 1 }).IsEqual(-CalChart::pi / 4.0));
    CHECK(uut.Direction(CalChart::Coord{ 1 + 0, 1 + 1 }).IsEqual(-CalChart::pi / 2.0));
    CHECK(uut.Direction(CalChart::Coord{ 1 + -1, 1 + 1 }).IsEqual(-3 * CalChart::pi / 4.0));
    CHECK(uut.Direction(CalChart::Coord{ 1 + -1, 1 + 0 }).IsEqual(CalChart::pi));
    CHECK(uut.Direction(CalChart::Coord{ 1 + -1, 1 + -1 }).IsEqual(3 * CalChart::pi / 4.0));
    CHECK(uut.Direction(CalChart::Coord{ 1 + 0, 1 + -1 }).IsEqual(CalChart::pi / 2.0));
    CHECK(uut.Direction(CalChart::Coord{ 1 + 1, 1 + -1 }).IsEqual(CalChart::pi / 4.0));
}

TEST_CASE("CoordMath", "CalChartCoord")
{
    // auto tester = static_cast<short>(-1280);
    // CHECK(CalChart::CoordUnits2Int(tester) == -80);
    CHECK(CalChart::Int2CoordUnits(static_cast<short>(1)) == 16);
    CHECK(CalChart::Int2CoordUnits(static_cast<short>(-1)) == -16);
    CHECK(CalChart::CoordUnits2Int(static_cast<short>(16)) == 1);
    CHECK(CalChart::CoordUnits2Int(static_cast<short>(-16)) == -1);
    CHECK(CalChart::CoordUnits2Int(static_cast<short>(17)) == 1);
    CHECK(CalChart::CoordUnits2Int(static_cast<int>(-17)) == -1);
    CHECK(CalChart::Int2CoordUnits(static_cast<int>(1)) == 16);
    CHECK(CalChart::Int2CoordUnits(static_cast<int>(-1)) == -16);
    CHECK(CalChart::CoordUnits2Int(static_cast<int>(16)) == 1);
    CHECK(CalChart::CoordUnits2Int(static_cast<int>(-16)) == -1);
    CHECK(CalChart::CoordUnits2Int(static_cast<int>(17)) == 1);
    CHECK(CalChart::CoordUnits2Int(static_cast<int>(-17)) == -1);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
