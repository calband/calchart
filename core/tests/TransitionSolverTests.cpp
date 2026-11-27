#include "CalChartAnimationCommand.h"
#include "CalChartAnimationCompile.h"
#include "CalChartSheet.h"
#include "e7_transition_solver.h"
#include "e7_transition_solver_internal.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <limits>

TEST_CASE("Transition solver checks conversion of movement step counts", "[transition-solver]")
{
    using CalChart::TransitionSolverInternal::CheckedStepCount;
    using CalChart::TransitionSolverInternal::InvalidStepCount;

    CHECK(CheckedStepCount(0.0f) == 0);
    CHECK(CheckedStepCount(12.75f) == 12);
    CHECK(CheckedStepCount(-0.75f) == 0);
    auto const firstOutOfRange = std::ldexp(1.0f, std::numeric_limits<unsigned>::digits);
    auto const largestInRange = std::nextafter(firstOutOfRange, 0.0f);
    CHECK(CheckedStepCount(largestInRange) == static_cast<unsigned>(largestInRange));
    CHECK_THROWS_AS(CheckedStepCount(-1.0f), InvalidStepCount);
    CHECK_THROWS_AS(CheckedStepCount(firstOutOfRange), InvalidStepCount);
    CHECK_THROWS_AS(CheckedStepCount(std::numeric_limits<float>::infinity()), InvalidStepCount);
    CHECK_THROWS_AS(CheckedStepCount(-std::numeric_limits<float>::infinity()), InvalidStepCount);
    CHECK_THROWS_AS(CheckedStepCount(std::numeric_limits<float>::quiet_NaN()), InvalidStepCount);
}

TEST_CASE("Transition solver preserves step counts for movement patterns", "[transition-solver]")
{
    using namespace CalChart;
    using Pattern = TransitionSolverParams::MarcherInstruction::Pattern;

    for (auto pattern : { Pattern::EWNS, Pattern::NSEW, Pattern::DMHS, Pattern::HSDM }) {
        for (auto destination :
            { Coord{ 0, 0 }, Coord{ 6, 0 }, Coord{ 0, -4 }, Coord{ 6, 4 }, Coord{ -6, -4 }, Coord{ -4, 6 } }) {
            INFO("pattern " << static_cast<int>(pattern) << " destination " << destination.x << "," << destination.y);
            auto start = Sheet{ 1 };
            start.SetBeats(16);
            start.SetPosition({ 0, 0 }, 0);
            auto end = Sheet{ 1 };
            end.SetPosition({ Int2CoordUnits(destination.x), Int2CoordUnits(destination.y) }, 0);
            auto params = TransitionSolverParams{};
            params.algorithm = TransitionSolverParams::E7_ALGORITHM__CHIU_ZAMORA_MALANI;
            params.availableInstructions[0] = { pattern, 0 };
            params.availableInstructionsMask[0] = true;
            auto result = runTransitionSolver(start, end, params, nullptr);
            auto const diagonal = pattern == Pattern::DMHS || pattern == Pattern::HSDM;
            auto const expected = diagonal ? std::max(std::abs(destination.x), std::abs(destination.y))
                                           : std::abs(destination.x) + std::abs(destination.y);
            REQUIRE(result.successfullySolved);
            CHECK(result.numBeatsOfMovement == static_cast<unsigned>(expected));
            REQUIRE(result.finalPositions.size() == 1);
            CHECK(result.finalPositions[0] == end.GetMarcher(0).GetPos());
        }
    }
}

TEST_CASE("Default animation uses remaining beats", "[animation]")
{
    using namespace CalChart;
    auto sheet = Sheet{ 1 };
    sheet.SetPosition({ 0, 0 }, 0);
    auto const end = Coord{ Int2CoordUnits(8), 0 };
    auto data = Animate::AnimationData{ 0, sheet.GetMarcher(0), end, 8, false };
    auto variables = Animate::Variables{};
    auto const [commands, errors] = Animate::CreateCompileResult(data, nullptr, variables);
    CHECK(errors.empty());
    REQUIRE(commands.size() == 1);
    CHECK(commands[0] == Animate::Command{ Animate::CommandMove{ { 0, 0 }, 8, end } });
}
