#include "CalChartAngles.h"
#include "CalChartConstants.h"
#include <catch2/catch_test_macros.hpp>
#include <map>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
using Radian = CalChart::Radian;
using Degree = CalChart::Degree;

TEST_CASE("BasicsRadians", "AnglesTest")
{
    auto uut = Radian{};
    CHECK(uut == Radian{ 0.0 });
    CHECK(Degree{ uut } == Degree{ 0.0 });
    CHECK_FALSE(Degree{ Radian{ 1 } } == Degree{ 0.0 });
    CHECK_FALSE(Degree{ Radian{ 1 } } == Degree{ 1 });
    CHECK(Radian{ Degree{ 180 } }.IsEqual(Radian{ std::numbers::pi }));
    CHECK(Radian{ Degree{ 225 } }.IsEqual(Radian{ std::numbers::pi } * 5.0 / 4.0));
    CHECK(Radian{ Degree{ 225 } }.IsEqual(5.0 / 4.0 * Radian{ std::numbers::pi }));
    CHECK(Radian{ Degree{ 360 } }.IsEqual(2 * Radian{ std::numbers::pi }));
    CHECK(Radian{ Degree{ 540 } }.IsEqual(3 * Radian{ std::numbers::pi }));

    CHECK(CalChart::BoundDirection(Degree{ 225 }) == Degree{ 225 });
    CHECK(CalChart::BoundDirection(-Degree{ 225 }) == Degree{ 135 });
    CHECK(CalChart::BoundDirection(CalChart::Degree{ 585 }).IsEqual(CalChart::Degree{ 225 }));
    CHECK(CalChart::BoundDirection(-CalChart::Degree{ 585 }).IsEqual(CalChart::Degree{ 135 }));

    CHECK(Radian{ 3.0 }.IsEqual(Radian{ 3.0 } + 2 * CalChart::pi));
    CHECK(Radian{ 3.0 }.IsEqual(Radian{ 3.0 } - 2 * CalChart::pi));
    CHECK(Radian{}.IsEqual(2 * CalChart::pi));
}

TEST_CASE("BasicsDegrees", "AnglesTest")
{
    auto uut = Degree{};
    CHECK(uut == Degree{ 0.0 });
    CHECK(Radian{ uut } == Radian{ 0.0 });
    CHECK_FALSE(Radian{ Degree{ 1 } } == Radian{ 0.0 });
    CHECK_FALSE(Radian{ Degree{ 1 } } == Radian{ 1 });
    CHECK(Degree{ Radian{ std::numbers::pi } }.IsEqual(Degree{ 180 }));
    CHECK(Degree{ Radian{ 5.0 / 4.0 * std::numbers::pi } }.IsEqual(Degree{ 180 } * 5.0 / 4.0));
    CHECK(Degree{ Radian{ 5.0 / 4.0 * std::numbers::pi } }.IsEqual(5.0 / 4.0 * Degree{ 180 }));
    CHECK(Degree{ Radian{ 2 * std::numbers::pi } }.IsEqual(2 * Degree{ 180 }));
    CHECK(Degree{ Radian{ 3 * std::numbers::pi } }.IsEqual(3 * Degree{ 180 }));

    CHECK(BoundDirection(Degree{ 225 }).IsEqual(Degree{ 225 }));
    CHECK(BoundDirection(Degree{ -225 }).IsEqual(Degree{ 135 }));
    CHECK(BoundDirection(Degree{ 585 }).IsEqual(Degree{ 225 }));
    CHECK(BoundDirection(Degree{ -585 }).IsEqual(Degree{ 135 }));

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
                CHECK(static_cast<CalChart::Direction>(AngleToQuadrant(Degree{ value } + Degree{ minorscrub } + Degree{ majorscrub })) == dir);
            }
        }
    }

    for (auto value : {
             45,
             135,
             225,
             315,
         }) {
        CHECK(IsDiagonalDirection(Degree{ value }));
        for (auto minorscrub : { 10, -10 }) {
            CHECK_FALSE(IsDiagonalDirection(Degree{ value } + Degree{ minorscrub }));
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
        auto unit = CreateCalChartUnitVector(Degree{ dir });
        CHECK(CalChart::IS_ZERO(std::get<0>(v) - std::get<0>(unit)));
        CHECK(CalChart::IS_ZERO(std::get<1>(v) - std::get<1>(unit)));
    }

    CHECK(CalChart::BoundDirection(Degree(1082.0)).IsEqual(Degree(2.0)));
    CHECK(CalChart::BoundDirection(Degree(2.0)).IsEqual(Degree(2.0)));
    CHECK(CalChart::BoundDirection(Degree(-1082.0)).IsEqual(Degree(358.0)));
    CHECK(CalChart::BoundDirection(Degree(-2.0)).IsEqual(Degree(358.0)));
    CHECK(CalChart::BoundDirectionSigned(Degree(182.0)).IsEqual(Degree(-178.0)));
    CHECK(CalChart::BoundDirectionSigned(Degree(-182.0)).IsEqual(Degree(178.0)));
    CHECK(CalChart::BoundDirection(Radian(2.0)).IsEqual(Radian(2.0)));
    CHECK(CalChart::BoundDirection(Radian(-2.0)).IsEqual(Radian(2 * (std::numbers::pi - 1.0))));
    CHECK(CalChart::BoundDirection(Degree(1082.0)).IsEqual(CalChart::BoundDirection(Degree(1082.0))));
    CHECK(CalChart::BoundDirection(Radian(-2.0)).IsEqual(CalChart::BoundDirection(Radian(-2.0))));
    CHECK(CalChart::AngleToQuadrant(Degree{ 0 }) == 0);
    CHECK(CalChart::AngleToQuadrant(Degree{ 44 }) == 7);
    CHECK(CalChart::AngleToQuadrant(Degree{ 45 }) == 7);
    CHECK(CalChart::AngleToQuadrant(Degree{ 90 }) == 6);
    CHECK(CalChart::AngleToQuadrant(Degree{ 135 }) == 5);
    CHECK(CalChart::AngleToQuadrant(Degree{ 160 }) == 4);
    CHECK(CalChart::AngleToQuadrant(Degree{ 180 }) == 4);
    CHECK(CalChart::AngleToQuadrant(Degree{ 225 }) == 3);
    CHECK(CalChart::AngleToQuadrant(Degree{ 270 }) == 2);
    CHECK(CalChart::AngleToQuadrant(Degree{ 315 }) == 1);
    CHECK(CalChart::AngleToQuadrant(Degree{ 350 }) == 0);
    CHECK(CalChart::AngleToQuadrant(Degree{ 360 }) == 0);
    CHECK(CalChart::AngleToQuadrant(Degree{ 341341 }) == 7);
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
        auto unit = CalChart::CreateUnitVector(Degree(dir));
        CHECK(CalChart::IS_ZERO(std::get<0>(v) - std::get<0>(unit)));
        CHECK(CalChart::IS_ZERO(std::get<1>(v) - std::get<1>(unit)));
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
