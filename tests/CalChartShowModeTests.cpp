#include "CalChartFileFormat.h"
#include "CalChartShowMode.h"
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity, misc-use-anonymous-namespace)

TEST_CASE("ShowModeBasics", "CalChartShowMode")
{
    auto uut1 = CalChart::ShowMode::GetDefaultShowMode();
    auto data = uut1.Serialize();
    auto uut2 = CalChart::ShowMode::CreateShowMode(CalChart::Reader({ data.data(), data.size() }));
    CHECK(uut1 == uut2);
}

static auto TestCreateOutline(auto cmds)
{
    CHECK(cmds.size() == 4);
    CHECK(cmds[0] == CalChart::DrawCommand{ CalChart::Draw::Line{ { 1, 2 }, { 2561, 2 } } });
    CHECK(cmds[1] == CalChart::DrawCommand{ CalChart::Draw::Line{ { 2561, 2 }, { 2561, 1346 } } });
    CHECK(cmds[2] == CalChart::DrawCommand{ CalChart::Draw::Line{ { 2561, 1346 }, { 1, 1346 } } });
    CHECK(cmds[3] == CalChart::DrawCommand{ CalChart::Draw::Line{ { 1, 1346 }, { 1, 2 } } });
}

TEST_CASE("Draw", "CalChartShowMode")
{
    auto uut1 = CalChart::ShowMode::GetDefaultShowMode();
    auto cmds = CalChart::CreateFieldLayout(uut1, false) + CalChart::Coord{ 1, 2 };
    CHECK(cmds.size() == 105);
    TestCreateOutline(std::vector(cmds.begin(), cmds.begin() + 4));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity, misc-use-anonymous-namespace)
