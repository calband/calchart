#include "CalChartDrawCommand.h"
#include "CalChartPoint.h"
#include <catch2/catch_test_macros.hpp>
#include <wx/dcmemory.h>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

auto TestPointPlain(auto cmds, bool filled)
{
    CHECK(cmds.size() == 2);

    auto uut1 = std::get<CalChart::Draw::Circle>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
    CHECK(uut2.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut2.text == "A");
    CHECK(!uut2.withBackground);
}

auto TestPointBksl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::Draw::Circle>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, -10 } : CalChart::Coord{ -10, -8 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, 20 } : CalChart::Coord{ 16, 18 }));
    auto uut3 = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointSl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::Draw::Circle>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, 20 } : CalChart::Coord{ -10, 18 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, -10 } : CalChart::Coord{ 16, -8 }));
    auto uut3 = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointX(auto cmds, bool filled)
{
    CHECK(cmds.size() == 4);

    auto uut1 = std::get<CalChart::Draw::Circle>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, 20 } : CalChart::Coord{ -10, 18 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, -10 } : CalChart::Coord{ 16, -8 }));
    auto uut4 = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
    CHECK(uut4.c1 == (filled ? CalChart::Coord{ -12, -10 } : CalChart::Coord{ -10, -8 }));
    CHECK(uut4.c2 == (filled ? CalChart::Coord{ 18, 20 } : CalChart::Coord{ 16, 18 }));
    auto uut3 = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestCreateOutline(auto cmds)
{
    CHECK(cmds.size() == 4);
    CHECK(cmds[0] == CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ { 1, 2 }, { 17, 2 } } });
    CHECK(cmds[1] == CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ { 17, 2 }, { 17, 7 } } });
    CHECK(cmds[2] == CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ { 17, 7 }, { 1, 7 } } });
    CHECK(cmds[3] == CalChart::Draw::DrawCommand{ CalChart::Draw::Line{ { 1, 7 }, { 1, 2 } } });
}

TEST_CASE("DrawCommand")
{
    SECTION("Line")
    {
        auto uut1 = CalChart::Draw::Line{ 1, 2, 3, 4 };
        auto uut2 = CalChart::Draw::Line{ { 1, 2 }, { 3, 4 } };
        CHECK(uut1.c1 == uut2.c1);
        CHECK(uut1.c2 == uut2.c2);
        CHECK(uut1.c1 == CalChart::Coord{ 1, 2 });
        CHECK(uut1.c2 == CalChart::Coord{ 3, 4 });
    }

    SECTION("CreatePoint.plain")
    {
        TestPointPlain(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_PLAIN }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            false);
    }

    SECTION("CreatePoint.sol")
    {
        TestPointPlain(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_SOL }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            true);
    }

    SECTION("CreatePoint.bksl")
    {
        TestPointBksl(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_BKSL }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            false);
    }

    SECTION("CreatePoint.solbksl")
    {
        TestPointBksl(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_SOLBKSL }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            true);
    }

    SECTION("CreatePoint.sl")
    {
        TestPointSl(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_SL }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            false);
    }

    SECTION("CreatePoint.solsl")
    {
        TestPointSl(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_SOLSL }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            true);
    }

    SECTION("CreatePoint.x")
    {
        TestPointX(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_X }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            false);
    }

    SECTION("CreatePoint.solx")
    {
        TestPointX(CalChart::Point{ { 0, 0 }, CalChart::SYMBOL_SOLX }.GetDrawCommands("A", 1.2, 1.4, 1.6)
                + CalChart::Coord{ 3, 5 },
            true);
    }

    SECTION("CreateOutline")
    {
        auto cmds = CalChart::Draw::Field::CreateOutline({ 16, 5 }) + CalChart::Coord{ 1, 2 };
        TestCreateOutline(cmds);
    }

    SECTION("CreateVerticalSolidLine")
    {
        auto cmds = CalChart::Draw::Field::CreateVerticalSolidLine({ 16, 5 }, 1) + CalChart::Coord{ 1, 2 };
        CHECK(cmds.size() == 3);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ 1, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 1, 7 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ 9, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 9, 7 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ 17, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 17, 7 });
    }

    SECTION("CreateVerticalDottedLine")
    {
        auto cmds = CalChart::Draw::Field::CreateVerticalDottedLine({ 16, 5 }, 1) + CalChart::Coord{ 1, 2 };
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ 5, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 3 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ 5, 4 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 5 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ 5, 6 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 7 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
        CHECK(uut.c1 == CalChart::Coord{ 13, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 3 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[4]));
        CHECK(uut.c1 == CalChart::Coord{ 13, 4 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 5 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[5]));
        CHECK(uut.c1 == CalChart::Coord{ 13, 6 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 7 });
    }
    SECTION("CreateHorizontalDottedLine")
    {
        auto cmds = CalChart::Draw::Field::CreateHorizontalDottedLine({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(2), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(3), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(4), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(6), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(2), CalChart::Int2CoordUnits(18) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[4]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(3), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(4), CalChart::Int2CoordUnits(18) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[5]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(6), CalChart::Int2CoordUnits(18) });
    }
    SECTION("CreateHashes")
    {
        auto cmds = CalChart::Draw::Field::CreateHashes({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 4);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(1.8), CalChart::Int2CoordUnits(10) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(8.2), CalChart::Int2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(9), CalChart::Int2CoordUnits(10) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(1.8), CalChart::Int2CoordUnits(14) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(8.2), CalChart::Int2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(9), CalChart::Int2CoordUnits(14) });
    }
    SECTION("CreateHashTicks")
    {
        auto cmds = CalChart::Draw::Field::CreateHashTicks({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 8);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[4]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[5]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[6]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[7]));
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(15.6) });
    }
    SECTION("CreateYardlineLabels")
    {
        using namespace std::string_literals;
        using TextAnchor = CalChart::Draw::Text::TextAnchor;
        auto testData = std::vector{ "A"s, "B"s, "C"s };
        auto cmds = CalChart::Draw::Field::CreateYardlineLabels(testData, { CalChart::Int2CoordUnits(14), 5 }, 2, 1) + CalChart::Coord{ 3, 5 };
        CHECK(cmds.size() == 4);

        auto uut = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ 3, 7 });
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ 3, 8 });
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[2]));
        CHECK(uut.c1 == CalChart::Coord{ 11, 7 });
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::Draw::Text>(std::get<CalChart::Draw::DrawItems>(cmds[3]));
        CHECK(uut.c1 == CalChart::Coord{ 11, 8 });
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
    }

    SECTION("CreateOutlineWithOffset")
    {
        TestCreateOutline(CalChart::Draw::Field::CreateOutline({ 16, 5 }) + CalChart::Coord{ 1, 2 });
        TestCreateOutline(CalChart::Coord{ 1, 2 } + CalChart::Draw::Field::CreateOutline({ 16, 5 }));
        TestCreateOutline(CalChart::Draw::Field::CreateOutline({ 16, 5 }) - CalChart::Coord{ 1, 2 } + CalChart::Coord{ 2, 4 });
    }

    SECTION("TestCoordMinusVector")
    {
        auto drawLines = std::vector<CalChart::Draw::DrawCommand>{
            CalChart::Draw::Line{ CalChart::Coord(0, 0), CalChart::Coord(10, 0) },
            CalChart::Draw::Line{ CalChart::Coord(3, 3), CalChart::Coord(1, 1) },
        };
        auto coord = CalChart::Coord{ 5, 5 };
        auto cmds = coord - drawLines;
        CHECK(cmds.size() == 2);
        auto uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[0]));
        CHECK(uut.c1 == CalChart::Coord{ 5, 5 });
        CHECK(uut.c2 == CalChart::Coord{ -5, 5 });
        uut = std::get<CalChart::Draw::Line>(std::get<CalChart::Draw::DrawItems>(cmds[1]));
        CHECK(uut.c1 == CalChart::Coord{ 2, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 4, 4 });
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
