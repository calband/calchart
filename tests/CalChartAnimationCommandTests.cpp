#include "CalChartAnimationCommand.h"
#include <catch2/catch_test_macros.hpp>
#include <map>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)

TEST_CASE("Animate::CommandStand", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandStill{ CalChart::Coord{ 16, 16 }, 4, CalChart::Animate::CommandStill::Style::StandAndPlay, CalChart::Degree::East() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::Close == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == End(item1));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = toOnlineViewerJSON(item1);
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

TEST_CASE("Animate::CommandClose", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandStill{ CalChart::Coord{ 16, 16 }, 4, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree::East() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::Close == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == End(item1));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = toOnlineViewerJSON(item1);
        auto goldjson = nlohmann::json{
            { "type", "close" },
            { "beats", 4.0 },
            { "facing", 180.0 },
            { "x", 81.0 },
            { "y", 43.0 }
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("Animate::CommandMT", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandStill{ CalChart::Coord{ 16, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::East() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::HighStep == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::East() == MotionDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 16, 16 } == End(item1));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = toOnlineViewerJSON(item1);
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

TEST_CASE("Animate::CommandMove", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandMove{ CalChart::Coord{ 16, 16 }, 4, CalChart::Coord{ 64, 64 }, CalChart::Degree::East() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::HighStep == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::NorthWest().IsEqual(MotionDirectionAtBeat(item1, 0)));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 32, 32 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::NorthWest().IsEqual(MotionDirectionAtBeat(item1, 1)));
        REQUIRE(CalChart::Coord{ 80, 80 } == End(item1));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ CalChart::Coord{ 16, 16 }, CalChart::Coord{ 80, 80 } } };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = toOnlineViewerJSON(item1);
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

TEST_CASE("Animate::CommandMoveNegative", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandMove{ CalChart::Coord{ 16, 16 }, 4, CalChart::Coord{ -64, -64 }, CalChart::Degree::East() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::HighStep == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::SouthEast().IsEqual(MotionDirectionAtBeat(item1, 0)));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 0, 0 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::East() == FacingDirectionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree::SouthEast().IsEqual(MotionDirectionAtBeat(item1, 1)));
        REQUIRE(CalChart::Coord{ -48, -48 } == End(item1));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
        auto goldCmd = CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ CalChart::Coord{ 16, 16 }, CalChart::Coord{ -48, -48 } } };
        REQUIRE(goldCmd == drawCmd);
    }
    SECTION("CheckJSON")
    {
        auto json = toOnlineViewerJSON(item1);
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

TEST_CASE("Animate::CommandRotate", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandRotate{ 4, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree::East(), CalChart::Degree::South() };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::HighStep == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        CHECK(CalChart::Coord{ 64, 0 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::South() == FacingDirectionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::South().IsEqual(MotionDirectionAtBeat(item1, 0)));
        CHECK(CalChart::Coord{ 40, 5 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree{ 202.5 }.IsEqual(FacingDirectionAtBeat(item1, 1)));
        REQUIRE(CalChart::Degree{ 202.5 }.IsEqual(MotionDirectionAtBeat(item1, 1)));
        CHECK(CalChart::Coord{ 19, 19 } == PositionAtBeat(item1, 2));
        REQUIRE(CalChart::Degree::SouthWest().IsEqual(FacingDirectionAtBeat(item1, 2)));
        REQUIRE(CalChart::Degree::SouthWest().IsEqual(MotionDirectionAtBeat(item1, 2)));
        CHECK(CalChart::Coord{ 5, 40 } == PositionAtBeat(item1, 3));
        REQUIRE(CalChart::Degree{ 247.5 }.IsEqual(FacingDirectionAtBeat(item1, 3)));
        REQUIRE(CalChart::Degree{ 247.5 }.IsEqual(MotionDirectionAtBeat(item1, 3)));
        CHECK(CalChart::Coord{ 0, 64 } == PositionAtBeat(item1, 4));
        CHECK(CalChart::Coord{ 0, 64 } == End(item1));
        REQUIRE(CalChart::Degree::West().IsEqual(FacingDirectionAtBeat(item1, 4)));
        REQUIRE(CalChart::Degree::West().IsEqual(MotionDirectionAtBeat(item1, 4)));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
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
        auto json = toOnlineViewerJSON(item1);
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

TEST_CASE("Animate::CommandRotateBackward", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandRotate{ 4, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree::East(), CalChart::Degree::South(), true };
    REQUIRE(4 == NumBeats(item1));
    REQUIRE(CalChart::MarchingStyle::HighStep == StepStyle(item1));
    SECTION("CheckGoForward")
    {
        auto item2 = item1;
        REQUIRE(item1 == item2);
        REQUIRE(CalChart::Coord{ 64, 0 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Degree::North().IsEqual(FacingDirectionAtBeat(item1, 0)));
        REQUIRE(CalChart::Degree::North().IsEqual(MotionDirectionAtBeat(item1, 0)));
        REQUIRE(CalChart::Coord{ 40, 5 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Degree{ 22.5 }.IsEqual(FacingDirectionAtBeat(item1, 1)));
        REQUIRE(CalChart::Degree{ 22.5 }.IsEqual(MotionDirectionAtBeat(item1, 1)));
        REQUIRE(CalChart::Coord{ 19, 19 } == PositionAtBeat(item1, 2));
        REQUIRE(CalChart::Degree::NorthEast().IsEqual(FacingDirectionAtBeat(item1, 2)));
        REQUIRE(CalChart::Degree::NorthEast().IsEqual(MotionDirectionAtBeat(item1, 2)));
        REQUIRE(CalChart::Coord{ 5, 40 } == PositionAtBeat(item1, 3));
        REQUIRE(CalChart::Degree{ 67.5 }.IsEqual(FacingDirectionAtBeat(item1, 3)));
        REQUIRE(CalChart::Degree{ 67.5 }.IsEqual(MotionDirectionAtBeat(item1, 3)));
        REQUIRE(CalChart::Coord{ 0, 64 } == PositionAtBeat(item1, 4));
        REQUIRE(CalChart::Coord{ 0, 64 } == End(item1));
        REQUIRE(CalChart::Degree::East().IsEqual(FacingDirectionAtBeat(item1, 4)));
        REQUIRE(CalChart::Degree::East().IsEqual(MotionDirectionAtBeat(item1, 4)));
    }
    SECTION("CheckDrawCommand")
    {
        auto drawCmd = GenCC_DrawCommand(item1);
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
        auto json = toOnlineViewerJSON(item1);
        auto goldjson = nlohmann::json{
            { "type", "arc" },
            { "start_x", 84.0 },
            { "start_y", 42.0 },
            { "center_x", 84.0 },
            { "center_y", 46.0 },
            { "angle", -90.0 },
            { "beats", 4.0 },
            { "beats_per_step", 1.0 },
            { "facing_offset", -180.0 },
        };
        REQUIRE(json == goldjson);
    }
}

TEST_CASE("Animate::CommandStill0", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandStill{ CalChart::Coord{ 16, 16 }, 0, CalChart::Animate::CommandStill::Style::StandAndPlay, CalChart::Degree::East() };
    SECTION("CheckGoForward")
    {
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(PositionAtBeat(item1, 0) == End(item1));
    }
}

TEST_CASE("Animate::CommandMT0", "Animate::Command")
{
    auto item1 = CalChart::Animate::Command{ CalChart::Animate::CommandStill{ CalChart::Coord{ 16, 16 }, 0, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::East() } };
    SECTION("CheckGoForward")
    {
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(PositionAtBeat(item1, 0) == End(item1));
    }
}

TEST_CASE("Animate::CommandMove0", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandMove{ CalChart::Coord{ 16, 16 }, 0, CalChart::Coord{ 64, 64 }, CalChart::Degree::East() };
    SECTION("CheckGoForward")
    {
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 16, 16 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 80, 80 } == End(item1));
    }
}

TEST_CASE("Animate::CommandRotate0", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandRotate{ 0, CalChart::Coord{ 64, 64 }, 64, CalChart::Degree::East(), CalChart::Degree::South() };
    SECTION("CheckGoForward")
    {
        REQUIRE(CalChart::Coord{ 64, 0 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 64, 0 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 0, 64 } == End(item1));
    }
}

TEST_CASE("Animate::Commands", "Animate::Commands")
{
    using beats_t = CalChart::Animate::beats_t;
    auto uut = CalChart::Animate::Commands(std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandStill{ { 16, 16 }, 3, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
        CalChart::Animate::CommandMove{ { 16, 16 }, 4, { 0, 64 } },
        CalChart::Animate::CommandStill{ { 16, 80 }, 5, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree::South() },
    });
    CHECK(12 == uut.TotalBeats());
    using beatInfo = std::pair<std::pair<size_t, beats_t>, std::pair<size_t, beats_t>>;
    CHECK(beatInfo{ { 0, 0 }, { 0, 0 } } == uut.BeatToCommandOffsetAndBeat(0));
    CHECK(beatInfo{ { 0, 1 }, { 0, 1 } } == uut.BeatToCommandOffsetAndBeat(1));
    CHECK(beatInfo{ { 0, 2 }, { 0, 2 } } == uut.BeatToCommandOffsetAndBeat(2));
    CHECK(beatInfo{ { 1, 0 }, { 1, 0 } } == uut.BeatToCommandOffsetAndBeat(3));
    CHECK(beatInfo{ { 1, 1 }, { 1, 1 } } == uut.BeatToCommandOffsetAndBeat(4));
    CHECK(beatInfo{ { 1, 2 }, { 1, 2 } } == uut.BeatToCommandOffsetAndBeat(5));
    CHECK(beatInfo{ { 1, 3 }, { 1, 3 } } == uut.BeatToCommandOffsetAndBeat(6));
    CHECK(beatInfo{ { 2, 0 }, { 2, 0 } } == uut.BeatToCommandOffsetAndBeat(7));
    CHECK(beatInfo{ { 2, 1 }, { 2, 1 } } == uut.BeatToCommandOffsetAndBeat(8));
    CHECK(beatInfo{ { 2, 2 }, { 2, 2 } } == uut.BeatToCommandOffsetAndBeat(9));
    CHECK(beatInfo{ { 2, 3 }, { 2, 3 } } == uut.BeatToCommandOffsetAndBeat(10));
    CHECK(beatInfo{ { 2, 4 }, { 2, 4 } } == uut.BeatToCommandOffsetAndBeat(11));
    CHECK(beatInfo{ { 3, 0 }, { 3, 0 } } == uut.BeatToCommandOffsetAndBeat(12));
    CHECK(beatInfo{ { 3, 1 }, { 3, 1 } } == uut.BeatToCommandOffsetAndBeat(13));
    CHECK(uut.MarcherInfoAtBeat(0) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::North() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::North() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(2) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::North() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(3) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(4) == CalChart::Animate::MarcherInfo{ { 16, 32 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(5) == CalChart::Animate::MarcherInfo{ { 16, 48 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(6) == CalChart::Animate::MarcherInfo{ { 16, 64 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(7) == CalChart::Animate::MarcherInfo{ { 16, 80 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::Close });
    CHECK(uut.MarcherInfoAtBeat(8) == CalChart::Animate::MarcherInfo{ { 16, 80 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::Close });
    CHECK(uut.MarcherInfoAtBeat(9) == CalChart::Animate::MarcherInfo{ { 16, 80 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::Close });
    CHECK(uut.MarcherInfoAtBeat(10) == CalChart::Animate::MarcherInfo{ { 16, 80 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::Close });
    CHECK(uut.MarcherInfoAtBeat(11) == CalChart::Animate::MarcherInfo{ { 16, 80 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::Close });
    CHECK(uut.MarcherInfoAtBeat(12) == CalChart::Animate::MarcherInfo{});
    CHECK(uut.MarcherInfoAtBeat(13) == CalChart::Animate::MarcherInfo{});
}

TEST_CASE("Animate::CommandMove64", "Animate::Command")
{
    auto item1 = CalChart::Animate::CommandMove{ CalChart::Coord{ 0, 0 }, 64, CalChart::Coord{ 1024, 0 }, CalChart::Degree::East() };
    SECTION("CheckGoForward")
    {
        REQUIRE(CalChart::Coord{ 0, 0 } == PositionAtBeat(item1, 0));
        REQUIRE(CalChart::Coord{ 16, 0 } == PositionAtBeat(item1, 1));
        REQUIRE(CalChart::Coord{ 32, 0 } == PositionAtBeat(item1, 2));
        REQUIRE(CalChart::Coord{ 64, 0 } == PositionAtBeat(item1, 4));
        REQUIRE(CalChart::Coord{ 128, 0 } == PositionAtBeat(item1, 8));
        REQUIRE(CalChart::Coord{ 256, 0 } == PositionAtBeat(item1, 16));
        REQUIRE(CalChart::Coord{ 512, 0 } == PositionAtBeat(item1, 32));
        REQUIRE(CalChart::Coord{ 640, 0 } == PositionAtBeat(item1, 40));
        REQUIRE(CalChart::Coord{ 1008, 0 } == PositionAtBeat(item1, 63));
    }
}

TEST_CASE("Animate::CommandWith0", "Animate::Commands")
{
    using beats_t = CalChart::Animate::beats_t;
    auto uut = CalChart::Animate::Commands(std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandMove{ { -416, -416 }, 2, { 0, 46 } },
        CalChart::Animate::CommandMove{ { -416, -370 }, 0, { 2, 2 } },
        CalChart::Animate::CommandStill{ { -414, -368 }, 6, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::West() },
    });
    CHECK(8 == uut.TotalBeats());
    using beatInfo = std::pair<std::pair<size_t, beats_t>, std::pair<size_t, beats_t>>;
    CHECK(beatInfo{ { 0, 0 }, { 0, 0 } } == uut.BeatToCommandOffsetAndBeat(0));
    CHECK(beatInfo{ { 0, 1 }, { 0, 1 } } == uut.BeatToCommandOffsetAndBeat(1));
    CHECK(beatInfo{ { 2, 0 }, { 1, 0 } } == uut.BeatToCommandOffsetAndBeat(2));
    CHECK(beatInfo{ { 2, 1 }, { 2, 1 } } == uut.BeatToCommandOffsetAndBeat(3));
    CHECK(beatInfo{ { 2, 2 }, { 2, 2 } } == uut.BeatToCommandOffsetAndBeat(4));
    CHECK(beatInfo{ { 2, 3 }, { 2, 3 } } == uut.BeatToCommandOffsetAndBeat(5));
    CHECK(beatInfo{ { 2, 4 }, { 2, 4 } } == uut.BeatToCommandOffsetAndBeat(6));
    CHECK(beatInfo{ { 2, 5 }, { 2, 5 } } == uut.BeatToCommandOffsetAndBeat(7));
    CHECK(beatInfo{ { 3, 0 }, { 3, 0 } } == uut.BeatToCommandOffsetAndBeat(8));
    CHECK(uut.MarcherInfoAtBeat(0) == CalChart::Animate::MarcherInfo{ { -416, -416 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1) == CalChart::Animate::MarcherInfo{ { -416, -393 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(2) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::NorthWest() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(3) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(4) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(5) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(6) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(7) == CalChart::Animate::MarcherInfo{ { -414, -368 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(8) == CalChart::Animate::MarcherInfo{});
    CHECK(uut.MarcherInfoAtBeat(9) == CalChart::Animate::MarcherInfo{});
}

TEST_CASE("Animate::CommandWith0.2", "Animate::Commands")
{
    using beats_t = CalChart::Animate::beats_t;
    auto uut = CalChart::Animate::Commands(std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandMove{ { 960, 64 }, 0, { 0, 4 } },
        CalChart::Animate::CommandMove{ { 960, 68 }, 4, { -64, 0 } },
        CalChart::Animate::CommandStill{ { 896, 68 }, 2, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::East() },
    });
    CHECK(6 == uut.TotalBeats());
    using beatInfo = std::pair<std::pair<size_t, beats_t>, std::pair<size_t, beats_t>>;
    CHECK(beatInfo{ { 1, 0 }, { 0, 0 } } == uut.BeatToCommandOffsetAndBeat(0));
    CHECK(beatInfo{ { 1, 0 }, { 1, 0 } } == uut.BeatToCommandOffsetAndBeat(1));
    CHECK(beatInfo{ { 1, 1 }, { 1, 1 } } == uut.BeatToCommandOffsetAndBeat(2));
    CHECK(beatInfo{ { 1, 2 }, { 1, 2 } } == uut.BeatToCommandOffsetAndBeat(3));
    CHECK(beatInfo{ { 1, 3 }, { 1, 3 } } == uut.BeatToCommandOffsetAndBeat(4));
    CHECK(beatInfo{ { 2, 0 }, { 2, 0 } } == uut.BeatToCommandOffsetAndBeat(5));
    CHECK(beatInfo{ { 2, 1 }, { 2, 1 } } == uut.BeatToCommandOffsetAndBeat(6));
    CHECK(beatInfo{ { 3, 0 }, { 3, 0 } } == uut.BeatToCommandOffsetAndBeat(7));
    CHECK(uut.MarcherInfoAtBeat(0).mPosition == CalChart::Coord{ 960, 68 });
    CHECK(uut.MarcherInfoAtBeat(0).mFacingDirection.IsEqual(CalChart::Radian{ CalChart::Degree::West() }));
    CHECK(uut.MarcherInfoAtBeat(0) == CalChart::Animate::MarcherInfo{ { 960, 68 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1) == CalChart::Animate::MarcherInfo{ { 960, 68 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(2) == CalChart::Animate::MarcherInfo{ { 944, 68 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(3) == CalChart::Animate::MarcherInfo{ { 928, 68 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(4) == CalChart::Animate::MarcherInfo{ { 912, 68 }, CalChart::Radian{ CalChart::Degree::South() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(5) == CalChart::Animate::MarcherInfo{ { 896, 68 }, CalChart::Radian{ CalChart::Degree::East() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(6) == CalChart::Animate::MarcherInfo{ { 896, 68 }, CalChart::Radian{ CalChart::Degree::East() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(7) == CalChart::Animate::MarcherInfo{});
    CHECK(uut.MarcherInfoAtBeat(8) == CalChart::Animate::MarcherInfo{});
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)
