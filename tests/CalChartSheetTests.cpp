#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("RoundTrip1", "CalChartSheetTests")
{
    using namespace CalChart;
    auto blank_sheet = Sheet(0);
    auto blank_sheet_data = blank_sheet.SerializeSheet();
    // need to pull out the sheet data
    auto reader = Reader({ blank_sheet_data.data(), blank_sheet_data.size() });
    auto table = reader.ParseOutLabels();
    CHECK(table.size() == 1);
    CHECK(std::get<0>(table.front()) == INGL_SHET);
    auto re_read_sheet = Sheet(0, std::get<1>(table.front()));
    auto re_read_sheet_data = re_read_sheet.SerializeSheet();
    bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(), re_read_sheet_data.begin());
    (void)is_equal;
    CHECK(is_equal);
}

TEST_CASE("RoundTrip2", "CalChartSheetTests")
{
    using namespace CalChart;
    auto blank_sheet = Sheet(0, "new_sheet");
    auto blank_sheet_data = blank_sheet.SerializeSheet();
    // need to pull out the sheet data
    auto reader = Reader({ blank_sheet_data.data(), blank_sheet_data.size() });
    auto table = reader.ParseOutLabels();
    CHECK(table.size() == 1);
    CHECK(std::get<0>(table.front()) == INGL_SHET);
    auto re_read_sheet = Sheet(0, std::get<1>(table.front()));
    auto re_read_sheet_data = re_read_sheet.SerializeSheet();
    bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(), re_read_sheet_data.begin());
    (void)is_equal;
    CHECK(is_equal);
}

TEST_CASE("RoundTrip3", "CalChartSheetTests")
{
    using namespace CalChart;
    auto blank_sheet = Sheet(1, "new_sheet");
    blank_sheet.SetName("new_name");
    blank_sheet.SetPosition(Coord(10, 10), 0);
    blank_sheet.SetPosition(Coord(20, 10), 0, 1);
    blank_sheet.SetPosition(Coord(30, 40), 0, 2);
    blank_sheet.SetPosition(Coord(52, 50), 0, 3);
    blank_sheet.SetBeats(13);
    blank_sheet.SetContinuity(SYMBOL_PLAIN, Continuity{ "MT E REM" });
    blank_sheet.SetPrintableContinuity("number 1", "duuuude, writing this testing is boring");
    auto blank_sheet_data = blank_sheet.SerializeSheet();
    // need to pull out the sheet data
    auto reader = Reader({ blank_sheet_data.data(), blank_sheet_data.size() });
    auto table = reader.ParseOutLabels();
    CHECK(table.size() == 1);
    CHECK(std::get<0>(table.front()) == INGL_SHET);
    auto re_read_sheet = Sheet(1, std::get<1>(table.front()));
    auto re_read_sheet_data = re_read_sheet.SerializeSheet();
    bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(), re_read_sheet_data.begin());
    //		auto mismatch_at = std::mismatch(blank_sheet_data.begin(),
    // blank_sheet_data.end(), re_read_sheet_data.begin());
    //		std::cout<<"mismatch at
    //"<<std::distance(blank_sheet_data.begin(),
    // mismatch_at.first)<<"\n";
    (void)is_equal;
    CHECK(is_equal);
}

TEST_CASE("AssigningToCurves", "CalChartSheetTests")
{
    using namespace CalChart;
    auto uut = Sheet(4);
    uut.SetPosition({ 0, 0 }, 0);
    uut.SetPosition({ 1, 1 }, 1);
    uut.SetPosition({ 2, 2 }, 2);
    uut.SetPosition({ 3, 3 }, 3);
    CHECK(uut.GetMarcherPosition(0) == Coord{ 0, 0 });
    CHECK(uut.GetNumberCurves() == 0);

    {
        auto allMarchers = uut.GetCurveAssignments();
        CHECK(allMarchers.empty());
    }

    // create a curve
    CHECK(uut.GetNumberCurves() == 0);
    auto curve = Curve{ { { 0, 8 }, { 8, 8 }, { 16, 8 } } };
    uut.AddCurve(curve, 0);
    CHECK(uut.GetNumberCurves() == 1);

    CHECK(!uut.FindCurveControlPoint({ 4, 8 }, 1).has_value());
    CHECK(!uut.FindCurve({ 4, 0 }, 1).has_value());
    CHECK(uut.FindCurve({ 4, 8 }, 1).has_value());
    {
        auto point = uut.FindCurveControlPoint({ 0, 8 }, 1);
        CHECK(point.has_value());
        CHECK(std::get<0>(*point) == 0);
        CHECK(std::get<1>(*point) == 0);
    }
    {
        auto point = uut.FindCurveControlPoint({ 8, 8 }, 1);
        CHECK(point.has_value());
        CHECK(std::get<0>(*point) == 0);
        CHECK(std::get<1>(*point) == 1);
    }

    {
        auto point = uut.FindCurve({ 4, 8 }, 1);
        CHECK(point.has_value());
        CHECK(std::get<0>(*point) == 0);
        CHECK(std::get<1>(*point) == 0);
    }
    {
        auto point = uut.FindCurve({ 12, 8 }, 1);
        CHECK(point.has_value());
        CHECK(std::get<0>(*point) == 0);
        CHECK(std::get<1>(*point) == 1);
    }

    {
        auto allMarchers = uut.GetCurveAssignments();
        CHECK(allMarchers.at(0) == std::vector<MarcherIndex>{});
    }

    uut.SetCurveAssignment(uut.GetCurveAssignmentsWithNewAssignments(0, { 0 }));
    {
        auto allMarchers = uut.GetCurveAssignments();
        CHECK(allMarchers.at(0) == std::vector<MarcherIndex>{ 0 });
    }
    CHECK(uut.GetMarcherPosition(0) == Coord{ 0, 8 });
    uut.SetCurveAssignment(uut.GetCurveAssignmentsWithNewAssignments(0, { 2, 3, 1, 0 }));
    {
        auto allMarchers = uut.GetCurveAssignments();
        CHECK(allMarchers.size() == 1);
        CHECK(allMarchers.at(0) == std::vector<MarcherIndex>{ 2, 3, 1, 0 });
    }
    CHECK(uut.GetMarcherPosition(0) == Coord{ 15, 7 });
    CHECK(uut.GetMarcherPosition(1) == Coord{ 9, 8 });
    CHECK(uut.GetMarcherPosition(2) == Coord{ 0, 8 });
    CHECK(uut.GetMarcherPosition(3) == Coord{ 5, 8 });

    uut.SetPosition({ 0, 0 }, 3);
    {
        auto allMarchers = uut.GetCurveAssignments();
        CHECK(allMarchers.size() == 1);
        CHECK(allMarchers.at(0) == std::vector<MarcherIndex>{ 2, 1, 0 });
    }
    CHECK(uut.GetMarcherPosition(0) == Coord{ 16, 8 });
    CHECK(uut.GetMarcherPosition(1) == Coord{ 8, 8 });
    CHECK(uut.GetMarcherPosition(2) == Coord{ 0, 8 });
    CHECK(uut.GetMarcherPosition(3) == Coord{ 0, 0 });

    // replace the curve, that should cause the points to have their positions change
    uut.ReplaceCurve(Curve{ { { 0, 4 }, { 8, 4 } } }, 0);
    CHECK(uut.GetMarcherPosition(0) == Coord{ 7, 3 });
    CHECK(uut.GetMarcherPosition(1) == Coord{ 3, 3 });
    CHECK(uut.GetMarcherPosition(2) == Coord{ 0, 4 });
}
