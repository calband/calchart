#include "CalChartCoord.h"
#include <catch2/catch_test_macros.hpp>
#include <map>
#include <random>

TEST_CASE("Basic", "CalChartCoord")
{
    CalChart::Coord undertest;
    REQUIRE(0.0 == undertest.Magnitude());
    REQUIRE(0.0 == undertest.DM_Magnitude());
    REQUIRE(0.0 == undertest.Direction());

    auto undertest2 = CalChart::Coord{ 16, 16 };
    CHECK(IS_ZERO(SQRT2 - undertest2.Magnitude()));
    CHECK(1.0f == undertest2.DM_Magnitude());
    CHECK(-45.0 == undertest2.Direction());
}

TEST_CASE("CreateVector", "CalChartCoord")
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
        auto unit = CalChart::CreateVector(dir, 1);
        CHECK(unit == v);
    }
}

TEST_CASE("Random", "CalChartCoord")
{
    std::random_device rd;
    std::mt19937 gen(rd());
    using namespace CalChart;
    std::uniform_int_distribution<Coord::units> distrib;
    // test equality:
    static constexpr auto kNumRand = 10;
    for (size_t i = 0; i < kNumRand; ++i) {
        Coord::units x1, y1, x2, y2;
        x1 = distrib(gen);
        y1 = distrib(gen);
        x2 = distrib(gen);
        y2 = distrib(gen);
        Coord undertest1(x1, y1);
        Coord undertest2(x2, y2);
        CHECK(x1 == undertest1.x);
        CHECK(y1 == undertest1.y);
        CHECK(x2 == undertest2.x);
        CHECK(y2 == undertest2.y);
        CHECK(Coord(x1, y1) == undertest1);
        CHECK(Coord(x2, y2) == undertest2);
        CHECK(!(Coord(x1, y1) != undertest1));
        CHECK(!(Coord(x2, y2) != undertest2));

        Coord::units x3, y3;

        auto newValue = undertest1 + undertest2;
        x3 = undertest1.x + undertest2.x;
        y3 = undertest1.y + undertest2.y;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue += undertest2;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1 - undertest2;
        x3 = undertest1.x - undertest2.x;
        y3 = undertest1.y - undertest2.y;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue -= undertest2;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);

        newValue = -undertest1;
        CHECK((-undertest1.x) == newValue.x);
        CHECK((-undertest1.y) == newValue.y);

        short factor = distrib(gen);
        newValue = undertest1 * factor;
        x3 = undertest1.x * factor;
        y3 = undertest1.y * factor;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        newValue = undertest1;
        newValue *= factor;
        CHECK((x3) == newValue.x);
        CHECK((y3) == newValue.y);
        // avoid div by 0
        if (factor) {
            newValue = undertest1 / factor;
            x3 = undertest1.x / factor;
            y3 = undertest1.y / factor;
            CHECK((x3) == newValue.x);
            CHECK((y3) == newValue.y);
            newValue = undertest1;
            newValue /= factor;
            CHECK((x3) == newValue.x);
            CHECK((y3) == newValue.y);
        }
    }
}
