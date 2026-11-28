#include "CalChartDrawingLayout.h"
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

TEST_CASE("Stack layouts honor alignments", "[drawing-layout]")
{
    using namespace wxCalChart::Draw;
    using CalChart::Draw::StackAlign;
    auto const sizes = std::vector<StackSize>{ wxSize{ 10, 6 }, wxSize{ 20, 4 } };
    auto const surface = DrawSurface{ { 3, 5 }, { 100, 40 } };

    CHECK(HLayout(sizes, surface, StackAlign::Begin) == std::vector<wxPoint>{ { 3, 5 }, { 13, 5 } });
    CHECK(HLayout(sizes, surface, StackAlign::End) == std::vector<wxPoint>{ { 73, 5 }, { 83, 5 } });
    CHECK(HLayout(sizes, surface, StackAlign::Uniform) == std::vector<wxPoint>{ { 20, 5 }, { 65, 5 } });
    CHECK(HLayout(sizes, surface, StackAlign::Justified) == std::vector<wxPoint>{ { 3, 5 }, { 83, 5 } });
    CHECK(VLayout(sizes, surface, StackAlign::Begin) == std::vector<wxPoint>{ { 3, 5 }, { 3, 11 } });
    CHECK(VLayout(sizes, surface, StackAlign::End) == std::vector<wxPoint>{ { 3, 35 }, { 3, 41 } });
    CHECK(VLayout(sizes, surface, StackAlign::Uniform) == std::vector<wxPoint>{ { 3, 12 }, { 3, 33 } });
    CHECK(VLayout(sizes, surface, StackAlign::Justified) == std::vector<wxPoint>{ { 3, 5 }, { 3, 41 } });

    auto const single = std::vector<StackSize>{ wxSize{ 10, 6 } };
    CHECK(HLayout(single, surface, StackAlign::Uniform) == std::vector<wxPoint>{ { 48, 5 } });
    CHECK(HLayout(single, surface, StackAlign::Justified) == std::vector<wxPoint>{ { 48, 5 } });
    CHECK(VLayout(single, surface, StackAlign::Uniform) == std::vector<wxPoint>{ { 3, 22 } });
    CHECK(VLayout(single, surface, StackAlign::Justified) == std::vector<wxPoint>{ { 3, 22 } });
    CHECK(HLayout(std::vector<StackSize>{}, surface, StackAlign::Begin).empty());
    CHECK(VLayout(std::vector<StackSize>{}, surface, StackAlign::Begin).empty());

    auto const invalid = static_cast<StackAlign>(-1);
    CHECK_THROWS_AS(HLayout(sizes, surface, invalid), std::invalid_argument);
    CHECK_THROWS_AS(VLayout(sizes, surface, invalid), std::invalid_argument);
}
