#include "CalChartAnimationSheet.h"
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)

// let's construct a test case with two marchers going towards each other.
// The first mt N for 1 ,then goes E for 8, close for 7 S.
// Second is W for 8, mtrm N.

TEST_CASE("AnimationSheetTest", "Animate::Sheet")
{
    auto cont1 = CalChart::Animate::CompileResult{
        { CalChart::Animate::CommandStill{ { 16, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
            CalChart::Animate::CommandMove{ { 16, 16 }, 8, { 0, 128 } },
            CalChart::Animate::CommandStill{ { 16, 144 }, 6, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree::South() } },
        {}
    };
    auto cont2 = CalChart::Animate::CompileResult{
        { CalChart::Animate::CommandMove{ { 16, 144 }, 8, { 0, -128 } },
            CalChart::Animate::CommandStill{ { 16, 16 }, 8, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() } },
        {}
    };
    auto uut = CalChart::Animate::Sheet{ "test", 16, { cont1, cont2 } };
    CHECK("test" == uut.GetName());
    CHECK(16 == uut.GetNumBeats());
    CHECK(uut.MarcherInfoAtBeat(0, 0) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::North() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(0, 4) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(0, 6) == CalChart::Animate::MarcherInfo{ { 16, 48 }, CalChart::Radian{ CalChart::Degree::West() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1, 0) == CalChart::Animate::MarcherInfo{ { 16, 144 }, CalChart::Radian{ CalChart::Degree::East() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1, 1) == CalChart::Animate::MarcherInfo{ { 16, 128 }, CalChart::Radian{ CalChart::Degree::East() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1, 6) == CalChart::Animate::MarcherInfo{ { 16, 48 }, CalChart::Radian{ CalChart::Degree::East() }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.MarcherInfoAtBeat(1, 8) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Radian{ CalChart::Degree::North() }, CalChart::MarchingStyle::HighStep });

    auto collisions = uut.GetAllBeatsWithCollisions();
    auto collided = uut.GetAllMarchersWithCollisionAtBeat(6);
    CHECK(collisions == std::set<CalChart::beats_t>{ 6 });
    CHECK(collided == CalChart::SelectionList{ 0, 1 });
}

TEST_CASE("NoCollision", "Animate::Sheet")
{
    std::cout << "doing test nocolsion\n";
    auto cont1 = CalChart::Animate::CompileResult{
        {
            CalChart::Animate::CommandStill{ { 64, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
            CalChart::Animate::CommandMove{ { 64, 16 }, 8, { 0, 128 } },
            CalChart::Animate::CommandStill{ { 64, 144 }, 6, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree::South() },
        },
        {}
    };
    auto cont2 = CalChart::Animate::CompileResult{
        {
            CalChart::Animate::CommandMove{ { 16, 144 }, 8, { 0, -128 } },
            CalChart::Animate::CommandStill{ { 16, 16 }, 8, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
        },
        {}
    };
    auto uut = CalChart::Animate::Sheet{ "test", 16, { cont1, cont2 } };
    CHECK(uut.GetAllBeatsWithCollisions().empty());
}

TEST_CASE("Animate::Sheets", "Animate::Sheets")
{
    using beats_t = CalChart::beats_t;
    auto cont1 = CalChart::Animate::CompileResult{
        {
            CalChart::Animate::CommandStill{ { 64, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
            CalChart::Animate::CommandMove{ { 64, 16 }, 8, { 0, 128 } },
            CalChart::Animate::CommandStill{ { 64, 144 }, 6, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree::South() },
        },
        {}
    };
    auto cont2 = CalChart::Animate::CompileResult{
        {
            CalChart::Animate::CommandMove{ { 16, 144 }, 8, { 0, -128 } },
            CalChart::Animate::CommandStill{ { 16, 16 }, 2, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree::North() },
        },
        {}
    };
    auto sheet1 = CalChart::Animate::Sheet{ "test", 16, { cont1 } };
    auto sheet2 = CalChart::Animate::Sheet{ "test2", 10, { cont2 } };
    auto uut = CalChart::Animate::Sheets{
        { sheet1, sheet2 }
    };
    CHECK(26 == uut.TotalBeats());
    CHECK(std::tuple<size_t, beats_t>{ 0, 0 } == uut.BeatToSheetOffsetAndBeat(0));
    CHECK(std::tuple<size_t, beats_t>{ 0, 1 } == uut.BeatToSheetOffsetAndBeat(1));
    CHECK(std::tuple<size_t, beats_t>{ 0, 12 } == uut.BeatToSheetOffsetAndBeat(12));
    CHECK(std::tuple<size_t, beats_t>{ 0, 13 } == uut.BeatToSheetOffsetAndBeat(13));
    CHECK(std::tuple<size_t, beats_t>{ 0, 14 } == uut.BeatToSheetOffsetAndBeat(14));
    CHECK(std::tuple<size_t, beats_t>{ 0, 15 } == uut.BeatToSheetOffsetAndBeat(15));
    CHECK(std::tuple<size_t, beats_t>{ 1, 0 } == uut.BeatToSheetOffsetAndBeat(16));
    CHECK(std::tuple<size_t, beats_t>{ 1, 1 } == uut.BeatToSheetOffsetAndBeat(17));
    CHECK(std::tuple<size_t, beats_t>{ 1, 2 } == uut.BeatToSheetOffsetAndBeat(18));
    CHECK(std::tuple<size_t, beats_t>{ 1, 7 } == uut.BeatToSheetOffsetAndBeat(23));
    CHECK(std::tuple<size_t, beats_t>{ 1, 8 } == uut.BeatToSheetOffsetAndBeat(24));
    CHECK(std::tuple<size_t, beats_t>{ 1, 9 } == uut.BeatToSheetOffsetAndBeat(25));
    CHECK(std::tuple<size_t, beats_t>{ 2, 0 } == uut.BeatToSheetOffsetAndBeat(26));
    CHECK(std::tuple<size_t, beats_t>{ 2, 1 } == uut.BeatToSheetOffsetAndBeat(27));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)
