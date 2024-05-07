#include "CalChartAnimationSheet.h"
#include <catch2/catch_test_macros.hpp>

/*
class Sheet {
public:
    Sheet(std::string name, unsigned numBeats, std::vector<std::vector<Animate::Command>> const& commands);

    ~Sheet() = default;
    Sheet(Sheet const&) = default;
    auto operator=(Sheet const&) -> Sheet& = default;
    Sheet(Sheet&&) noexcept = default;
    auto operator=(Sheet&&) noexcept -> Sheet& = default;

    [[nodiscard]] auto GetName() const { return mName; }
    [[nodiscard]] auto GetNumBeats() const { return mNumBeats; }

    [[nodiscard]] auto GetPositionOfMarcherAtBeat(size_t whichMarcher, unsigned beat) -> Coord;
    [[nodiscard]] auto GetAllCollisionsAtBeat(unsigned beat) -> std::set<size_t>;

private:
    std::string mName;
    unsigned mNumBeats;
    std::vector<std::vector<Animate::Command>> mCommands;
};
*/
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)

// let's construct a test case with two marchers going towards each other.
// The first mt N for 1 ,then goes E for 8, close for 7 S.
// Second is W for 8, mtrm N.

TEST_CASE("AnimationSheetTest", "Animate::Sheet")
{
    auto cont1 = std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandStill{ { 16, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree{ 0 } },
        CalChart::Animate::CommandMove{ { 16, 16 }, 8, { 0, 128 } },
        CalChart::Animate::CommandStill{ { 16, 144 }, 6, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree{ 180 } },
    };
    auto cont2 = std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandMove{ { 16, 144 }, 8, { 0, -128 } },
        CalChart::Animate::CommandStill{ { 16, 16 }, 8, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree{ 0 } },
    };
    auto uut = CalChart::Animate::Sheet{ "test", 16, { cont1, cont2 } };
    CHECK("test" == uut.GetName());
    CHECK(16 == uut.GetNumBeats());
    CHECK(uut.GetMarcherInfoAtBeat(0, 0) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Degree{ 0 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(0, 4) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Degree{ -90 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(0, 6) == CalChart::Animate::MarcherInfo{ { 16, 48 }, CalChart::Degree{ -90 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(1, 0) == CalChart::Animate::MarcherInfo{ { 16, 144 }, CalChart::Degree{ 90 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(1, 1) == CalChart::Animate::MarcherInfo{ { 16, 128 }, CalChart::Degree{ 90 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(1, 6) == CalChart::Animate::MarcherInfo{ { 16, 48 }, CalChart::Degree{ 90 }, CalChart::MarchingStyle::HighStep });
    CHECK(uut.GetMarcherInfoAtBeat(1, 8) == CalChart::Animate::MarcherInfo{ { 16, 16 }, CalChart::Degree{ 0 }, CalChart::MarchingStyle::HighStep });

    auto collisions = uut.GetAllBeatsWithCollisions();
    auto collided = uut.GetAllMarchersWithCollisionAtBeat(6);
    CHECK(collisions == std::set<CalChart::Animate::beats_t>{ 6 });
    CHECK(collided == std::set<size_t>{ 0, 1 });
}

TEST_CASE("NoCollision", "Animate::Sheet")
{
    std::cout << "doing test nocolsion\n";
    auto cont1 = std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandStill{ { 64, 16 }, 4, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree{ 0 } },
        CalChart::Animate::CommandMove{ { 64, 16 }, 8, { 0, 128 } },
        CalChart::Animate::CommandStill{ { 64, 144 }, 6, CalChart::Animate::CommandStill::Style::Close, CalChart::Degree{ 180 } },
    };
    auto cont2 = std::vector<CalChart::Animate::Command>{
        CalChart::Animate::CommandMove{ { 16, 144 }, 8, { 0, -128 } },
        CalChart::Animate::CommandStill{ { 16, 16 }, 8, CalChart::Animate::CommandStill::Style::MarkTime, CalChart::Degree{ 0 } },
    };
    auto uut = CalChart::Animate::Sheet{ "test", 16, { cont1, cont2 } };
    CHECK(uut.GetAllBeatsWithCollisions().empty());
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity)
