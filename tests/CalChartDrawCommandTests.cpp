#include "CalChartDrawCommand.h"
#include "CalChartPoint.h"
#include <catch2/catch_test_macros.hpp>
#include <wx/dcmemory.h>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

auto TestPointPlain(auto cmds, bool filled)
{
    CHECK(cmds.size() == 2);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Text>(cmds[1]);
    CHECK(uut2.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut2.text == "A");
    CHECK(!uut2.withBackground);
}

auto TestPointBksl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, -10 } : CalChart::Coord{ -10, -8 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, 20 } : CalChart::Coord{ 16, 18 }));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[2]);
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointSl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, 20 } : CalChart::Coord{ -10, 18 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, -10 } : CalChart::Coord{ 16, -8 }));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[2]);
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointX(auto cmds, bool filled)
{
    CHECK(cmds.size() == 4);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.c1 == CalChart::Coord{ 3, 5 });
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.c1 == (filled ? CalChart::Coord{ -12, 20 } : CalChart::Coord{ -10, 18 }));
    CHECK(uut2.c2 == (filled ? CalChart::Coord{ 18, -10 } : CalChart::Coord{ 16, -8 }));
    auto uut4 = std::get<CalChart::DrawCommands::Line>(cmds[2]);
    CHECK(uut4.c1 == (filled ? CalChart::Coord{ -12, -10 } : CalChart::Coord{ -10, -8 }));
    CHECK(uut4.c2 == (filled ? CalChart::Coord{ 18, 20 } : CalChart::Coord{ 16, 18 }));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[3]);
    CHECK(uut3.c1 == CalChart::Coord{ 3, -4 });
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestCreateOutline(auto cmds)
{
    CHECK(cmds.size() == 4);
    auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
    CHECK(uut.c1 == CalChart::Coord{ 1, 2 });
    CHECK(uut.c2 == CalChart::Coord{ 17, 2 });
    uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut.c1 == CalChart::Coord{ 17, 2 });
    CHECK(uut.c2 == CalChart::Coord{ 17, 7 });
    uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
    CHECK(uut.c1 == CalChart::Coord{ 17, 7 });
    CHECK(uut.c2 == CalChart::Coord{ 1, 7 });
    uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
    CHECK(uut.c1 == CalChart::Coord{ 1, 7 });
    CHECK(uut.c2 == CalChart::Coord{ 1, 2 });
}

TEST_CASE("DrawCommand")
{
    SECTION("Line")
    {
        auto uut1 = CalChart::DrawCommands::Line{ 1, 2, 3, 4 };
        auto uut2 = CalChart::DrawCommands::Line{ { 1, 2 }, { 3, 4 } };
        CHECK(uut1.c1 == uut2.c1);
        CHECK(uut1.c2 == uut2.c2);
        CHECK(uut1.c1 == CalChart::Coord{ 1, 2 });
        CHECK(uut1.c2 == CalChart::Coord{ 3, 4 });
    }

    SECTION("CreatePoint.plain")
    {
        TestPointPlain(CalChart::DrawCommands::Point::CreatePoint(
                           CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_PLAIN, 1.2, 1.4, 1.6),
            false);
    }

    SECTION("CreatePoint.sol")
    {
        TestPointPlain(CalChart::DrawCommands::Point::CreatePoint(
                           CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_SOL, 1.2, 1.4, 1.6),
            true);
    }

    SECTION("CreatePoint.bksl")
    {
        TestPointBksl(CalChart::DrawCommands::Point::CreatePoint(
                          CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_BKSL, 1.2, 1.4, 1.6),
            false);
    }

    SECTION("CreatePoint.solbksl")
    {
        TestPointBksl(CalChart::DrawCommands::Point::CreatePoint(
                          CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_SOLBKSL, 1.2, 1.4, 1.6),
            true);
    }

    SECTION("CreatePoint.sl")
    {
        TestPointSl(CalChart::DrawCommands::Point::CreatePoint(
                        CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_SL, 1.2, 1.4, 1.6),
            false);
    }

    SECTION("CreatePoint.solsl")
    {
        TestPointSl(CalChart::DrawCommands::Point::CreatePoint(
                        CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_SOLSL, 1.2, 1.4, 1.6),
            true);
    }

    SECTION("CreatePoint.x")
    {
        TestPointX(CalChart::DrawCommands::Point::CreatePoint(
                       CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_X, 1.2, 1.4, 1.6),
            false);
    }

    SECTION("CreatePoint.solx")
    {
        TestPointX(CalChart::DrawCommands::Point::CreatePoint(
                       CalChart::Point{ { 1, 2 } }, { 3, 5 }, "A", CalChart::SYMBOL_SOLX, 1.2, 1.4, 1.6),
            true);
    }

    SECTION("CreateOutline")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateOutline({ 16, 5 }) + CalChart::Coord{ 1, 2 };
        TestCreateOutline(cmds);
    }

    SECTION("CreateVerticalSolidLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateVerticalSolidLine({ 16, 5 }, 1) + CalChart::Coord{ 1, 2 };
        CHECK(cmds.size() == 3);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ 1, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 1, 7 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ 9, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 9, 7 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ 17, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 17, 7 });
    }

    SECTION("CreateVerticalDottedLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateVerticalDottedLine({ 16, 5 }, 1) + CalChart::Coord{ 1, 2 };
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ 5, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 3 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ 5, 4 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 5 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ 5, 6 });
        CHECK(uut.c2 == CalChart::Coord{ 5, 7 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.c1 == CalChart::Coord{ 13, 2 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 3 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.c1 == CalChart::Coord{ 13, 4 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 5 });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.c1 == CalChart::Coord{ 13, 6 });
        CHECK(uut.c2 == CalChart::Coord{ 13, 7 });
    }
    SECTION("CreateHorizontalDottedLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHorizontalDottedLine({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(2), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(3), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(4), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(6) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(6), CalChart::Int2CoordUnits(6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(2), CalChart::Int2CoordUnits(18) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(3), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(4), CalChart::Int2CoordUnits(18) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(18) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(6), CalChart::Int2CoordUnits(18) });
    }
    SECTION("CreateHashes")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHashes({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 4);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(1.8), CalChart::Int2CoordUnits(10) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(8.2), CalChart::Int2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(9), CalChart::Int2CoordUnits(10) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(1.8), CalChart::Int2CoordUnits(14) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(8.2), CalChart::Int2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Int2CoordUnits(9), CalChart::Int2CoordUnits(14) });
    }
    SECTION("CreateHashTicks")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHashTicks({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, 8, 12, CalChart::Int2CoordUnits(1)) + CalChart::Coord{ CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) };
        CHECK(cmds.size() == 8);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(2.6), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(4.2), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(5.8), CalChart::Float2CoordUnits(15.6) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[6]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(10) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(8.4) });
        uut = std::get<CalChart::DrawCommands::Line>(cmds[7]);
        CHECK(uut.c1 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(14) });
        CHECK(uut.c2 == CalChart::Coord{ CalChart::Float2CoordUnits(7.4), CalChart::Float2CoordUnits(15.6) });
    }
    SECTION("CreateYardlineLabels")
    {
        using namespace std::string_literals;
        using TextAnchor = CalChart::DrawCommands::Text::TextAnchor;
        auto testData = std::vector{ "A"s, "B"s, "C"s };
        auto cmds = CalChart::DrawCommands::Field::CreateYardlineLabels(testData, { CalChart::Int2CoordUnits(14), 5 }, 2, 1) + CalChart::Coord{ 3, 5 };
        CHECK(cmds.size() == 4);

        auto uut = std::get<CalChart::DrawCommands::Text>(cmds[0]);
        CHECK(uut.c1 == CalChart::Coord{ 3, 7 });
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[1]);
        CHECK(uut.c1 == CalChart::Coord{ 3, 8 });
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[2]);
        CHECK(uut.c1 == CalChart::Coord{ 11, 7 });
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[3]);
        CHECK(uut.c1 == CalChart::Coord{ 11, 8 });
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
    }

    SECTION("CreateOutlineWithOffset")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateOutline({ 16, 5 }) + CalChart::Coord{ 1, 2 };
        TestCreateOutline(cmds);
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
