#include "CalChartDrawCommand.h"
#include "CalChartPoint.h"
#include <catch2/catch_test_macros.hpp>
#include <wx/dcmemory.h>

auto TestPointPlain(auto cmds, bool filled)
{
    CHECK(cmds.size() == 2);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.x1 == 3);
    CHECK(uut1.y1 == 5);
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Text>(cmds[1]);
    CHECK(uut2.x == 3);
    CHECK(uut2.y == -4);
    CHECK(uut2.text == "A");
    CHECK(!uut2.withBackground);
}

auto TestPointBksl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.x1 == 3);
    CHECK(uut1.y1 == 5);
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.x1 == (filled ? -12 : -10));
    CHECK(uut2.y1 == (filled ? -10 : -8));
    CHECK(uut2.x2 == (filled ? 18 : 16));
    CHECK(uut2.y2 == (filled ? 20 : 18));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[2]);
    CHECK(uut3.x == 3);
    CHECK(uut3.y == -4);
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointSl(auto cmds, bool filled)
{
    CHECK(cmds.size() == 3);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.x1 == 3);
    CHECK(uut1.y1 == 5);
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.x1 == (filled ? -12 : -10));
    CHECK(uut2.y1 == (filled ? 20 : 18));
    CHECK(uut2.x2 == (filled ? 18 : 16));
    CHECK(uut2.y2 == (filled ? -10 : -8));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[2]);
    CHECK(uut3.x == 3);
    CHECK(uut3.y == -4);
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

auto TestPointX(auto cmds, bool filled)
{
    CHECK(cmds.size() == 4);

    auto uut1 = std::get<CalChart::DrawCommands::Circle>(cmds[0]);
    CHECK(uut1.x1 == 3);
    CHECK(uut1.y1 == 5);
    CHECK(uut1.radius == 9);
    CHECK(uut1.filled == filled);
    auto uut2 = std::get<CalChart::DrawCommands::Line>(cmds[1]);
    CHECK(uut2.x1 == (filled ? -12 : -10));
    CHECK(uut2.y1 == (filled ? 20 : 18));
    CHECK(uut2.x2 == (filled ? 18 : 16));
    CHECK(uut2.y2 == (filled ? -10 : -8));
    auto uut4 = std::get<CalChart::DrawCommands::Line>(cmds[2]);
    CHECK(uut4.x1 == (filled ? -12 : -10));
    CHECK(uut4.y1 == (filled ? -10 : -8));
    CHECK(uut4.x2 == (filled ? 18 : 16));
    CHECK(uut4.y2 == (filled ? 20 : 18));
    auto uut3 = std::get<CalChart::DrawCommands::Text>(cmds[3]);
    CHECK(uut3.x == 3);
    CHECK(uut3.y == -4);
    CHECK(uut3.text == "A");
    CHECK(!uut3.withBackground);
}

TEST_CASE("DrawCommand")
{
    SECTION("Line")
    {
        auto uut1 = CalChart::DrawCommands::Line{ 1, 2, 3, 4 };
        auto uut2 = CalChart::DrawCommands::Line{ { 1, 2 }, { 3, 4 } };
        CHECK(uut1.x1 == uut2.x1);
        CHECK(uut1.y1 == uut2.y1);
        CHECK(uut1.x2 == uut2.x2);
        CHECK(uut1.y2 == uut2.y2);
        CHECK(uut1.x1 == 1);
        CHECK(uut1.y1 == 2);
        CHECK(uut1.x2 == 3);
        CHECK(uut1.y2 == 4);
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
        auto cmds = CalChart::DrawCommands::Field::CreateOutline({ 16, 5 }, { 1, 2 });
        CHECK(cmds.size() == 4);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == 1);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 17);
        CHECK(uut.y2 == 2);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == 17);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 17);
        CHECK(uut.y2 == 7);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == 17);
        CHECK(uut.y1 == 7);
        CHECK(uut.x2 == 1);
        CHECK(uut.y2 == 7);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.x1 == 1);
        CHECK(uut.y1 == 7);
        CHECK(uut.x2 == 1);
        CHECK(uut.y2 == 2);
    }
    SECTION("CreateVerticalSolidLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateVerticalSolidLine({ 16, 5 }, { 1, 2 }, 1);
        CHECK(cmds.size() == 3);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == 1);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 1);
        CHECK(uut.y2 == 7);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == 9);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 9);
        CHECK(uut.y2 == 7);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == 17);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 17);
        CHECK(uut.y2 == 7);
    }

    SECTION("CreateVerticalDottedLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateVerticalDottedLine({ 16, 5 }, { 1, 2 }, 1);
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == 5);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 5);
        CHECK(uut.y2 == 3);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == 5);
        CHECK(uut.y1 == 4);
        CHECK(uut.x2 == 5);
        CHECK(uut.y2 == 5);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == 5);
        CHECK(uut.y1 == 6);
        CHECK(uut.x2 == 5);
        CHECK(uut.y2 == 7);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.x1 == 13);
        CHECK(uut.y1 == 2);
        CHECK(uut.x2 == 13);
        CHECK(uut.y2 == 3);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.x1 == 13);
        CHECK(uut.y1 == 4);
        CHECK(uut.x2 == 13);
        CHECK(uut.y2 == 5);
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.x1 == 13);
        CHECK(uut.y1 == 6);
        CHECK(uut.x2 == 13);
        CHECK(uut.y2 == 7);
    }
    SECTION("CreateHorizontalDottedLine")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHorizontalDottedLine({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) }, 8, 12, CalChart::Int2CoordUnits(1));
        CHECK(cmds.size() == 6);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(1));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(6));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(2));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(3));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(6));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(4));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(5));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(6));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(6));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(1));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(18));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(2));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(18));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(3));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(18));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(4));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(18));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(5));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(18));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(6));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(18));
    }
    SECTION("CreateHashes")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHashes({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) }, 8, 12, CalChart::Int2CoordUnits(1));
        CHECK(cmds.size() == 4);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(1));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(1.8));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(10));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(8.2));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(9));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(10));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == CalChart::Int2CoordUnits(1));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(1.8));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(14));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(8.2));
        CHECK(uut.y1 == CalChart::Int2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Int2CoordUnits(9));
        CHECK(uut.y2 == CalChart::Int2CoordUnits(14));
    }
    SECTION("CreateHashTicks")
    {
        auto cmds = CalChart::DrawCommands::Field::CreateHashTicks({ CalChart::Int2CoordUnits(5), CalChart::Int2CoordUnits(20) }, { CalChart::Int2CoordUnits(1), CalChart::Int2CoordUnits(2) }, 8, 12, CalChart::Int2CoordUnits(1));
        CHECK(cmds.size() == 8);
        auto uut = std::get<CalChart::DrawCommands::Line>(cmds[0]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(2.6));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(2.6));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(8.4));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[1]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(2.6));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(2.6));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(15.6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[2]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(4.2));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(4.2));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(8.4));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[3]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(4.2));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(4.2));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(15.6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[4]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(5.8));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(5.8));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(8.4));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[5]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(5.8));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(5.8));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(15.6));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[6]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(7.4));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(10));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(7.4));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(8.4));
        uut = std::get<CalChart::DrawCommands::Line>(cmds[7]);
        CHECK(uut.x1 == CalChart::Float2CoordUnits(7.4));
        CHECK(uut.y1 == CalChart::Float2CoordUnits(14));
        CHECK(uut.x2 == CalChart::Float2CoordUnits(7.4));
        CHECK(uut.y2 == CalChart::Float2CoordUnits(15.6));
    }
    SECTION("CreateYardlineLabels")
    {
        using namespace std::string_literals;
        using TextAnchor = CalChart::DrawCommands::Text::TextAnchor;
        auto testData = std::array{ "A"s, "B"s, "C"s };
        auto cmds = CalChart::DrawCommands::Field::CreateYardlineLabels(testData, { CalChart::Int2CoordUnits(14), 5 }, { 3, 5 }, 2, 1);
        CHECK(cmds.size() == 4);

        auto uut = std::get<CalChart::DrawCommands::Text>(cmds[0]);
        CHECK(uut.x == 3);
        CHECK(uut.y == 7);
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[1]);
        CHECK(uut.x == 3);
        CHECK(uut.y == 8);
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[2]);
        CHECK(uut.x == 11);
        CHECK(uut.y == 7);
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Bottom | TextAnchor::HorizontalCenter | TextAnchor::ScreenTop));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[3]);
        CHECK(uut.x == 11);
        CHECK(uut.y == 8);
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (TextAnchor::Top | TextAnchor::HorizontalCenter));
        CHECK(uut.withBackground);
    }
}