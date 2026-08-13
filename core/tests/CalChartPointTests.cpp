#include "CalChartPoint.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace CalChart;

namespace {
// Test Suite stuff
struct Point_values {
    SYMBOL_TYPE mSym{ SYMBOL_PLAIN };
    Coord mPos{};
    std::array<Coord, Point::kNumRefPoints> mRef{};
    bool GetFlip{};
    bool Visable{};
};

auto Check_Point(Point const& underTest, Point_values const& values) -> bool
{
    return underTest.GetPos() == values.mPos && underTest.GetPos(1) == values.mRef.at(0)
        && underTest.GetPos(2) == values.mRef.at(1) && underTest.GetPos(3) == values.mRef.at(2)
        && underTest.GetSymbol() == values.mSym && underTest.GetFlip() == values.GetFlip
        && underTest.LabelIsVisible() == values.Visable;
}
}

TEST_CASE("CalChartPointTests")
{
    // test some defaults:
    Point_values values{};
    values.mSym = SYMBOL_PLAIN;
    values.Visable = true;

    // test defaults
    Point underTest;
    CHECK(Check_Point(underTest, values));

    // test flip
    underTest.Flip(false);
    CHECK(Check_Point(underTest, values));

    values.GetFlip = true;
    underTest.Flip(true);
    CHECK(Check_Point(underTest, values));

    values.GetFlip = false;
    underTest.Flip(false);
    CHECK(Check_Point(underTest, values));

    // test visability
    underTest.SetLabelVisibility(true);
    CHECK(Check_Point(underTest, values));

    values.Visable = false;
    underTest.SetLabelVisibility(false);
    CHECK(Check_Point(underTest, values));

    values.Visable = true;
    underTest.SetLabelVisibility(true);
    CHECK(Check_Point(underTest, values));

    SECTION("JSON round trip")
    {
        underTest.SetPos(Coord{ 16, 32 });
        underTest.SetPos(Coord{ 48, 64 }, 1);
        underTest.SetPos(Coord{ 80, 96 }, 2);
        underTest.SetPos(Coord{ 112, 128 }, 3);
        underTest.SetSymbol(SYMBOL_SOLX);
        underTest.Flip(true);
        underTest.SetLabelVisibility(false);

        auto const json = underTest.toJSON();
        CHECK(json.contains("pos"));
        CHECK(json.at("pos").is_array());
        CHECK(json.at("pos") == nlohmann::json::array({ 16, 32 }));
        auto const fromJson = Point(json);

        CHECK(fromJson.GetPos() == underTest.GetPos());
        CHECK(fromJson.GetPos(1) == underTest.GetPos(1));
        CHECK(fromJson.GetPos(2) == underTest.GetPos(2));
        CHECK(fromJson.GetPos(3) == underTest.GetPos(3));
        CHECK(fromJson.GetSymbol() == underTest.GetSymbol());
        CHECK(fromJson.GetFlip() == underTest.GetFlip());
        CHECK(fromJson.LabelIsVisible() == underTest.LabelIsVisible());
        CHECK(fromJson.toJSON() == json);
    }

    SECTION("JSON parse failures")
    {
        auto const missingFields = nlohmann::json{
            { "pos", nlohmann::json::array({ 0, 0 }) },
            { "flip", false },
            { "label_invisible", true },
        };
        CHECK_THROWS_AS(Point(missingFields), CC_FileException);

        auto const badSymbol = nlohmann::json{
            { "pos", nlohmann::json::array({ 0, 0 }) },
            { "ref",
                nlohmann::json::array({
                    nlohmann::json::array({ 0, 0 }),
                    nlohmann::json::array({ 0, 0 }),
                    nlohmann::json::array({ 0, 0 }),
                }) },
            { "symbol", "InvalidSymbol" },
            { "flip", false },
            { "label_invisible", true },
        };
        CHECK_THROWS_AS(Point(badSymbol), CC_FileException);

        auto const objectPosition = nlohmann::json{
            { "pos", nlohmann::json{ { "x", 0 }, { "y", 0 } } },
            { "ref",
                nlohmann::json::array({
                    nlohmann::json::array({ 0, 0 }),
                    nlohmann::json::array({ 0, 0 }),
                    nlohmann::json::array({ 0, 0 }),
                }) },
            { "symbol", "Plain" },
            { "flip", false },
            { "label_invisible", true },
        };
        CHECK_THROWS_AS(Point(objectPosition), CC_FileException);

        auto const objectRefPoint = nlohmann::json{
            { "pos", nlohmann::json::array({ 0, 0 }) },
            { "ref",
                nlohmann::json::array({
                    nlohmann::json{ { "x", 0 }, { "y", 0 } },
                    nlohmann::json::array({ 0, 0 }),
                    nlohmann::json::array({ 0, 0 }),
                }) },
            { "symbol", "Plain" },
            { "flip", false },
            { "label_invisible", true },
        };
        CHECK_THROWS_AS(Point(objectRefPoint), CC_FileException);
    }

    SECTION("JSON parses floating point coordinates")
    {
        auto const floatingPointJson = nlohmann::json{
            { "pos", nlohmann::json::array({ 16.4, 31.6 }) },
            { "ref",
                nlohmann::json::array({
                    nlohmann::json::array({ 48.2, 63.9 }),
                    nlohmann::json::array({ 80.7, 96.1 }),
                    nlohmann::json::array({ 111.6, 127.5 }),
                }) },
            { "symbol", "X" },
            { "flip", true },
            { "label_invisible", true },
        };

        auto const fromJson = Point(floatingPointJson);
        CHECK(fromJson.GetPos() == Coord{ 16, 32 });
        CHECK(fromJson.GetPos(1) == Coord{ 48, 64 });
        CHECK(fromJson.GetPos(2) == Coord{ 81, 96 });
        CHECK(fromJson.GetPos(3) == Coord{ 112, 128 });
        CHECK(fromJson.GetSymbol() == SYMBOL_X);
        CHECK(fromJson.GetFlip());
        CHECK_FALSE(fromJson.LabelIsVisible());
    }
}
