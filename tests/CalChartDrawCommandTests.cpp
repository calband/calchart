#include "CalChartDrawCommand.h"
#include <catch2/catch_test_macros.hpp>
#include <wx/dcmemory.h>

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
        CHECK(uut.anchor == (toUType(TextAnchor::Bottom) | toUType(TextAnchor::HorizontalCenter) | toUType(TextAnchor::ScreenTop)));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[1]);
        CHECK(uut.x == 3);
        CHECK(uut.y == 8);
        CHECK(uut.text == "A");
        CHECK(uut.anchor == (toUType(TextAnchor::Top) | toUType(TextAnchor::HorizontalCenter)));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[2]);
        CHECK(uut.x == 11);
        CHECK(uut.y == 7);
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (toUType(TextAnchor::Bottom) | toUType(TextAnchor::HorizontalCenter) | toUType(TextAnchor::ScreenTop)));
        CHECK(uut.withBackground);
        uut = std::get<CalChart::DrawCommands::Text>(cmds[3]);
        CHECK(uut.x == 11);
        CHECK(uut.y == 8);
        CHECK(uut.text == "B");
        CHECK(uut.anchor == (toUType(TextAnchor::Top) | toUType(TextAnchor::HorizontalCenter)));
        CHECK(uut.withBackground);
    }
}