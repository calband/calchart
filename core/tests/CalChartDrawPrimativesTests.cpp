#include "CalChartDrawPrimatives.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("DrawPrimatives")
{
    SECTION("Color")
    {
        auto uut1 = CalChart::Color{ "GREE" };
        auto serialized1 = uut1.ToString();
        auto uut2 = CalChart::Color::FromString(serialized1);
        CHECK(uut1 == uut2);

        auto uut3 = CalChart::Color(1, 2, 3);
        auto serialized3 = uut3.ToString();
        auto uut4 = CalChart::Color::FromString(serialized3);
        CHECK(uut3 == uut4);
        CHECK(uut1 != uut4);
    }
}