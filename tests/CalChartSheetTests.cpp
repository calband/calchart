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
