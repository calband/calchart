#include "CalChartAngles.h"
#include "CalChartConstants.h"
#include "CalChartUtils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

TEST_CASE("CalChartUtils", "basics")
{
    CHECK_FALSE(CalChart::IS_ZERO(1.0));
    CHECK(CalChart::IS_ZERO(1.0e-9));

    CHECK(CalChart::details::Deg2Rad(0) == 0);
    CHECK_FALSE(CalChart::details::Deg2Rad(1) == 0);
    CHECK_FALSE(CalChart::details::Deg2Rad(1) == 1.0F);
    CHECK(CalChart::IS_ZERO(CalChart::details::Deg2Rad(180) - std::numbers::pi));
    CHECK(CalChart::IS_ZERO(CalChart::details::Deg2Rad(225) - 5.0 / 4.0 * std::numbers::pi));
    CHECK(CalChart::IS_ZERO(CalChart::details::Deg2Rad(360) - 2 * std::numbers::pi));
    CHECK(CalChart::IS_ZERO(CalChart::details::Deg2Rad(540) - 3 * std::numbers::pi));
    CHECK(CalChart::IS_ZERO(CalChart::details::Rad2Deg(std::numbers::pi) - 180));
    CHECK(CalChart::IS_ZERO(CalChart::details::Rad2Deg(5.0 / 4.0 * std::numbers::pi) - 225));
    CHECK(CalChart::IS_ZERO(CalChart::details::Rad2Deg(2 * std::numbers::pi) - 360));
    CHECK(CalChart::IS_ZERO(CalChart::details::Rad2Deg(3 * std::numbers::pi) - 540));

    CHECK(BoundDirection(CalChart::Degree{ 225 }).IsEqual(CalChart::Degree{ 225 }));
    CHECK(CalChart::BoundDirection(CalChart::Degree{ -225 }) == CalChart::Degree{ 135 });
    CHECK(CalChart::BoundDirection(CalChart::Degree{ 585 }).IsEqual(CalChart::Degree{ 225 }));
    CHECK(CalChart::BoundDirection(-CalChart::Degree{ 585 }).IsEqual(CalChart::Degree{ 135 }));

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
                CHECK(CalChart::AngleToDirection(CalChart::Degree{ value + minorscrub + majorscrub }) == dir);
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
                CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ value + minorscrub + majorscrub }) == dir);
            }
        }
    }
    for (auto value : {
             45,
             135,
             225,
             315,
         }) {
        CHECK(CalChart::IsDiagonalDirection(CalChart::Degree{ value }));
        for (auto minorscrub : { 10, -10 }) {
            CHECK_FALSE(CalChart::IsDiagonalDirection(CalChart::Degree{ value + minorscrub }));
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
        auto unit = CalChart::CreateCalChartUnitVector(CalChart::Degree{ dir });
        CHECK(CalChart::IS_ZERO(std::get<0>(v) - std::get<0>(unit)));
        CHECK(CalChart::IS_ZERO(std::get<1>(v) - std::get<1>(unit)));
    }

    CHECK(CalChart::BoundDirection(CalChart::Degree{ 1082.0 }).IsEqual(CalChart::Degree{ 2.0 }));
    CHECK(CalChart::BoundDirection(CalChart::Degree{ 2.0 }).IsEqual(CalChart::Degree{ 2.0 }));
    CHECK(CalChart::BoundDirection(CalChart::Degree{ -1082.0 }).IsEqual(CalChart::Degree{ 358.0 }));
    CHECK(CalChart::BoundDirection(CalChart::Degree{ -2.0 }).IsEqual(CalChart::Degree{ 358.0 }));
    CHECK(CalChart::BoundDirectionSigned(CalChart::Degree{ 182.0 }).IsEqual(CalChart::Degree{ -178.0 }));
    CHECK(CalChart::BoundDirectionSigned(CalChart::Degree{ -182.0 }).IsEqual(CalChart::Degree{ 178.0 }));
    CHECK(CalChart::BoundDirection(CalChart::Radian{ 2.0 }).IsEqual(CalChart::Radian{ 2.0 }));
    CHECK(CalChart::BoundDirection(CalChart::Radian{ -2.0 }).IsEqual(2 * (CalChart::pi - CalChart::Radian{ 1.0 })));
    CHECK(CalChart::BoundDirection(CalChart::Degree{ 1082.0 }).IsEqual(CalChart::NormalizeAngle(CalChart::Degree{ 1082.0 })));
    CHECK(CalChart::BoundDirection(CalChart::Radian{ -2.0 }).IsEqual(CalChart::NormalizeAngle(CalChart::Radian{ -2.0 })));
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 0 }) == 0);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 44 }) == 7);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 45 }) == 7);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 90 }) == 6);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 135 }) == 5);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 160 }) == 4);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 180 }) == 4);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 225 }) == 3);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 270 }) == 2);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 315 }) == 1);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 350 }) == 0);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 360 }) == 0);
    CHECK(CalChart::AngleToQuadrant(CalChart::Degree{ 341341 }) == 7);
}

TEST_CASE("CreateUnitVector", "CalChartUtils")
{
    for (auto [dir, v] : std::map<double, std::tuple<double, double>>{
             { 0, { 1, 0 } },
             { 10, { 0.984807753, -0.1736481777 } },
             { 44, { 0.7193398003, -0.6946583705 } },
             { 45, { std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2 } },
             { 90, { 0, -1 } },
             { 135, { -std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2 } },
             { 180, { -1, 0 } },
             { 225, { -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2 } },
             { 270, { 0, 1 } },
             { 315, { std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2 } },
             { 355, { 0.9961946981, 0.0871557427 } },
             { 360, { 1, 0 } },
             { 370, { 0.984807753, -0.1736481777 } },
             { -316, { 0.7193398003, -0.6946583705 } },
         }) {
        auto unit = CalChart::CreateUnitVector(CalChart::Degree{ dir });
        CHECK(CalChart::IS_ZERO(std::get<0>(v) - std::get<0>(unit)));
        CHECK(CalChart::IS_ZERO(std::get<1>(v) - std::get<1>(unit)));
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
