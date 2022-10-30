#include "CalChartUtils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>

TEST_CASE("CalChartUtils", "basics")
{
    using namespace CalChart;
    CHECK_FALSE(IS_ZERO(1.0));
    CHECK(IS_ZERO(1.0e-9));

    CHECK(Deg2Rad(0) == 0);
    CHECK_FALSE(Deg2Rad(1) == 0);
    CHECK_FALSE(Deg2Rad(1) == 1.0f);
    CHECK(IS_ZERO(Deg2Rad(180) - std::numbers::pi));
    CHECK(IS_ZERO(Deg2Rad(225) - 5.0 / 4.0 * std::numbers::pi));
    CHECK(IS_ZERO(Deg2Rad(360) - 2 * std::numbers::pi));
    CHECK(IS_ZERO(Deg2Rad(540) - 3 * std::numbers::pi));
    CHECK(IS_ZERO(Rad2Deg(std::numbers::pi) - 180));
    CHECK(IS_ZERO(Rad2Deg(5.0 / 4.0 * std::numbers::pi) - 225));
    CHECK(IS_ZERO(Rad2Deg(2 * std::numbers::pi) - 360));
    CHECK(IS_ZERO(Rad2Deg(3 * std::numbers::pi) - 540));

    CHECK(BoundDirection(225) == 225);
    CHECK(BoundDirection(-225) == 135);
    CHECK(BoundDirection(585) == 225);
    CHECK(BoundDirection(-585) == 135);

    for (auto [value, dir] : std::vector<std::tuple<int, CalChart::Direction>>{
             { 0, CalChart::Direction::North },
             { 45, CalChart::Direction::NorthWest },
             { 90, CalChart::Direction::West },
             { 135, CalChart::Direction::SouthWest },
             { 180, CalChart::Direction::South },
             { 225, CalChart::Direction::SouthEast },
             { 270, CalChart::Direction::East },
             { 315, CalChart::Direction::NorthEast },
             { 360, CalChart::Direction::North },
             { -45, CalChart::Direction::NorthEast },
             { -90, CalChart::Direction::East },
             { -135, CalChart::Direction::SouthEast },
             { -180, CalChart::Direction::South },
             { -225, CalChart::Direction::SouthWest },
             { -270, CalChart::Direction::West },
             { -315, CalChart::Direction::NorthWest },
             { -360, CalChart::Direction::North },
         }) {
        for (auto minorscrub : { 0, 10, -10 }) {
            for (auto majorscrub : { 0, 360, -360, 720, -720 }) {
                CHECK(AngleToDirection(value + minorscrub + majorscrub) == dir);
            }
        }
    }
    for (auto [value, dir] : std::vector<std::tuple<int, int>>{
             { 0, 0 },
             { 45, 7 },
             { 90, 6 },
             { 135, 5 },
             { 180, 4 },
             { 225, 3 },
             { 270, 2 },
             { 315, 1 },
             { 360, 0 },
             { -45, 1 },
             { -90, 2 },
             { -135, 3 },
             { -180, 4 },
             { -225, 5 },
             { -270, 6 },
             { -315, 7 },
             { -360, 0 },
         }) {
        for (auto minorscrub : { 0, 10, -10 }) {
            for (auto majorscrub : { 0, 360, -360, 720, -720 }) {
                CHECK(AngleToQuadrant(value + minorscrub + majorscrub) == dir);
            }
        }
    }
    for (auto value : {
             45,
             135,
             225,
             315,
         }) {
        CHECK(IsDiagonalDirection(value));
        for (auto minorscrub : { 10, -10 }) {
            CHECK_FALSE(IsDiagonalDirection(value + minorscrub));
        }
    }

    for (auto [dir, v] : std::map<double, std::tuple<double, double>>{
             { 0, { 1, 0 } },
             { 10, { 0.984807753, -0.1736481777 } },
             { 44, { 0.7193398003, -0.6946583705 } },
             { 45, { 1.0, -1.0 } },
             { 90, { 0, -1 } },
             { 135, { -1, -1 } },
             { 180, { -1, 0 } },
             { 225, { -1, 1 } },
             { 270, { 0, 1 } },
             { 315, { 1, 1 } },
             { 355, { 0.9961946981, 0.0871557427 } },
             { 360, { 1, 0 } },
         }) {
        auto unit = CreateUnitVector(dir);
        CHECK(IS_ZERO(std::get<0>(v) - std::get<0>(unit)));
        CHECK(IS_ZERO(std::get<1>(v) - std::get<1>(unit)));
    }
}
