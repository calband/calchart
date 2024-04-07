#include "CalChartAnimationCommand.h"
#include <catch2/catch_test_macros.hpp>
#include <map>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)

TEST_CASE("AnimationCommandStand", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandStand{ 4, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::Close == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());

        REQUIRE(item1.NextBeat(pt1));
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 16, 16 } == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());

        REQUIRE(item1.PrevBeat(pt1));
        REQUIRE(pt2 == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());
        REQUIRE(item1 == item2);

        REQUIRE(item1.NextBeat(pt1));
        REQUIRE(item1.NextBeat(pt1));
        REQUIRE(item1.NextBeat(pt1));
        REQUIRE(!item1.NextBeat(pt1));
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "stand" },
            { "beats", 4.0 },
            { "facing", 180.0 },
            { "x", 81.0 },
            { "y", 43.0 }
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandMT", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandMT{ 4, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::HighStep == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());

        item1.NextBeat(pt1);
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 16, 16 } == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());

        item1.PrevBeat(pt1);
        REQUIRE(pt2 == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 90 } == item1.MotionDirection());
        REQUIRE(item1 == item2);

        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "mark" },
            { "beats", 4.0 },
            { "facing", 180.0 },
            { "x", 81.0 },
            { "y", 43.0 }
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandMove", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandMove{ 4, CalChart::Coord{ 64, 64 }, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::HighStep == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ -45 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 32, 32 } == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ -45 }.IsEqual(item1.MotionDirection()));

        item1.PrevBeat(pt1);
        REQUIRE(pt2 == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ -45 }.IsEqual(item1.MotionDirection()));
        REQUIRE(item1 == item2);

        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ CalChart::Coord{ 16, 16 }, CalChart::Coord{ 80, 80 } } };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "even" },
            { "beats", 4.0 },
            { "beats_per_step", 1.0 },
            { "facing", 315.0 },
            { "x1", 81.0 },
            { "y1", 43.0 },
            { "x2", 85.0 },
            { "y2", 47.0 },
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandMoveNegative", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandMove{ 4, CalChart::Coord{ -64, -64 }, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::HighStep == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 135 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 0, 0 } == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 135 }.IsEqual(item1.MotionDirection()));

        item1.PrevBeat(pt1);
        REQUIRE(pt2 == pt1);
        REQUIRE(CalChart::Degree{ 90 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 135 }.IsEqual(item1.MotionDirection()));
        REQUIRE(item1 == item2);

        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ CalChart::Coord{ 16, 16 }, CalChart::Coord{ -48, -48 } } };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "even" },
            { "beats", 4.0 },
            { "beats_per_step", 1.0 },
            { "facing", 135.0 },
            { "x1", 81.0 },
            { "y1", 43.0 },
            { "x2", 77.0 },
            { "y2", 39.0 },
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandRotate", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandRotate{ 4, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree{ 90 }, CalChart::Degree{ 180 } };
    auto pt1 = CalChart::Coord{ 128, 128 };
    item1.ApplyBackward(pt1);
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::HighStep == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Coord{ 64, 0 } == pt1);
        REQUIRE(CalChart::Degree{ 180 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 180 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 40, 5 } == pt1);
        REQUIRE(CalChart::Degree{ 202.5 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 202.5 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(CalChart::Coord{ 19, 19 } == pt1);
        REQUIRE(CalChart::Degree{ 225 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 225 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(CalChart::Coord{ 5, 40 } == pt1);
        REQUIRE(CalChart::Degree{ 247.5 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 247.5 }.IsEqual(item1.MotionDirection()));

        item1.PrevBeat(pt1);
        REQUIRE(CalChart::Coord{ 19, 19 } == pt1);
        REQUIRE(CalChart::Degree{ 225 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 225 }.IsEqual(item1.MotionDirection()));
        item1.ApplyBackward(pt1);

        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{
            CalChart::Draw::Arc{
                { 64, 0 },
                { 0, 64 },
                { 64, 64 } }
        };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "arc" },
            { "start_x", 84.0 },
            { "start_y", 42.0 },
            { "center_x", 84.0 },
            { "center_y", 46.0 },
            { "angle", -90.0 },
            { "beats", 4.0 },
            { "beats_per_step", 1.0 },
            { "facing_offset", 0.0 },
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandRotateBackward", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandRotate{ 4, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree{ 90 }, CalChart::Degree{ 180 }, true };
    auto pt1 = CalChart::Coord{ 128, 128 };
    item1.ApplyBackward(pt1);
    REQUIRE(4 == item1.NumBeats());
    REQUIRE(CalChart::MarchingStyle::HighStep == item1.StepStyle());
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Coord{ 64, 0 } == pt1);
        REQUIRE(CalChart::Degree{ 0 } == item1.FacingDirection());
        REQUIRE(CalChart::Degree{ 0 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(item1 != item2);

        REQUIRE(CalChart::Coord{ 40, 5 } == pt1);
        REQUIRE(CalChart::Degree{ 22.5 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 22.5 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(CalChart::Coord{ 19, 19 } == pt1);
        REQUIRE(CalChart::Degree{ 45 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 45 }.IsEqual(item1.MotionDirection()));

        item1.NextBeat(pt1);
        REQUIRE(CalChart::Coord{ 5, 40 } == pt1);
        REQUIRE(CalChart::Degree{ 67.5 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 67.5 }.IsEqual(item1.MotionDirection()));

        item1.PrevBeat(pt1);
        REQUIRE(CalChart::Coord{ 19, 19 } == pt1);
        REQUIRE(CalChart::Degree{ 45 }.IsEqual(item1.FacingDirection()));
        REQUIRE(CalChart::Degree{ 45 }.IsEqual(item1.MotionDirection()));
        item1.ApplyBackward(pt1);

        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item1.NextBeat(pt1);
        item2.ApplyForward(pt2);
        REQUIRE(item1 == item2);
        REQUIRE(pt1 == pt2);
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = item1.GenCC_DrawCommand(pt1);
        auto goldCmd = CalChart::Draw::DrawCommand{
            CalChart::Draw::Arc{
                { 64, 0 },
                { 0, 64 },
                { 64, 64 } }
        };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = item1.toOnlineViewerJSON(pt1);
        auto goldjson = nlohmann::json{
            { "type", "arc" },
            { "start_x", 84.0 },
            { "start_y", 42.0 },
            { "center_x", 84.0 },
            { "center_y", 46.0 },
            { "angle", -90.0 },
            { "beats", 4.0 },
            { "beats_per_step", 1.0 },
            { "facing_offset", 180.0 },
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("AnimationCommandStand0", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandStand{ 0, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    SECTION("CheckGoForward")
    {
        auto pt2 = pt1;
        REQUIRE(!item1.NextBeat(pt1));
        REQUIRE(pt2 == pt1);
    }
}

TEST_CASE("AnimationCommandMT0", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandMT{ 0, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    SECTION("CheckGoForward")
    {
        auto pt2 = pt1;
        REQUIRE(!item1.NextBeat(pt1));
        REQUIRE(pt2 == pt1);
    }
}

TEST_CASE("AnimationCommandMove0", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandMove{ 0, CalChart::Coord{ 64, 64 }, CalChart::Degree{ 90 } };
    auto pt1 = CalChart::Coord{ 16, 16 };
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(!item1.NextBeat(pt1));
        REQUIRE(pt1 == pt2);
        item2.ApplyForward(pt2);
        REQUIRE(item1 != item2);
        REQUIRE(pt1 != pt2);
    }
}

TEST_CASE("AnimationCommandRotate0", "AnimationCommand")
{
    auto item1 = CalChart::AnimationCommandRotate{ 0, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree{ 90 }, CalChart::Degree{ 180 } };
    auto pt1 = CalChart::Coord{ 128, 128 };
    item1.ApplyBackward(pt1);
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        auto pt2 = pt1;
        REQUIRE(!item1.NextBeat(pt1));
        REQUIRE(pt1 == pt2);
        item2.ApplyForward(pt2);
        REQUIRE(item1 != item2);
        REQUIRE(pt1 != pt2);
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)
