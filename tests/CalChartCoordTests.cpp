#include "catch2/catch.hpp"
#include "CalChartCoord.h"

TEST_CASE( "CalChartCoord", "[factorial]" ) {
    CalChart::Coord undertest;
    REQUIRE(0.0 == undertest.Magnitude());
    REQUIRE(0.0 == undertest.DM_Magnitude());
    REQUIRE(0.0 == undertest.Direction());
}
