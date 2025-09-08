#include "CalChartShow.h"
#include <catch2/catch_test_macros.hpp>

using namespace CalChart;
using namespace CalChart::Parser;

namespace {
auto Construct_show_zero_points_zero_labels_zero_description()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(INGL_DESC, std::vector<uint8_t>(1)));
    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    return Construct_block(INGL_SHOW, show_data);
}

auto Construct_show_zero_points_zero_labels()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    return Construct_block(INGL_SHOW, show_data);
}

auto Construct_show_zero_points_zero_labels_1_sheet_and_random()
{
    std::vector<std::byte> show_data;
    Append(show_data, Construct_block(0x12345678, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(INGL_SIZE, std::vector<uint8_t>(4)));
    Append(show_data, Construct_block(0x87654321, std::vector<uint8_t>(13)));
    Append(show_data, Construct_block(INGL_LABL, std::vector<uint8_t>{}));
    Append(show_data, Construct_block(0xDEADBEEF, std::vector<uint8_t>(1)));

    std::vector<std::byte> sheet_data;
    Append(sheet_data, Construct_block(INGL_NAME, std::vector<uint8_t>{ '1', '\0' }));
    Append(sheet_data, Construct_block(INGL_DURA, std::vector<uint8_t>{ 0, 0, 0, 1 }));
    Append(sheet_data, Construct_block(INGL_PNTS, std::vector<uint8_t>{}));
    Append(sheet_data, Construct_block(INGL_CONT, std::vector<uint8_t>{}));
    Append(sheet_data, Construct_block(INGL_PCNT, std::vector<uint8_t>{ '\0', '\0' }));
    Append(show_data, Construct_block(INGL_SHET, sheet_data));

    Append(show_data, Construct_block(INGL_CURR, std::vector<uint8_t>(4)));

    return Construct_block(INGL_SHOW, show_data);
}
}

TEST_CASE("RoundTrip", "CalChartShowTests")
{
    using namespace CalChart;
    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto char_data = std::string{};
    std::transform(blank_show_data.begin(), blank_show_data.end(), std::back_inserter(char_data), [](auto a) { return std::to_integer<char>(a); });
    std::istringstream is(char_data);
    auto re_read_show = Show::Create(ShowMode::GetDefaultShowMode(), is);
    auto re_read_show_data = re_read_show->SerializeShow();
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
    (void)is_equal;
    CHECK(is_equal);
}

TEST_CASE("RoundTripWithNumberLabelDescription", "CalChartShowTests")
{
    using namespace CalChart;
    std::vector<std::byte> point_data;
    Append(point_data, uint32_t{ 1 });
    std::vector<std::byte> data;
    Append(data, Construct_block(INGL_SIZE, point_data));
    Append(data, Construct_block(INGL_LABL, std::vector<char>{ 'p', 'o', 'i', 'n', 't', '\0' }));
    Append(data, Construct_block(INGL_DESC, std::vector<char>{ 'd', 'e', 's', 'c', 'r', 'i', 'p', 't', 'i', 'o', 'n', '\0' }));
    std::vector<std::byte> curr_data;
    Append(curr_data, uint32_t{ 0 });
    Append(data, Construct_block(INGL_CURR, curr_data));
    Append(data, Construct_block(INGL_MODE, ShowMode::GetDefaultShowMode().Serialize()));
    auto show_data = Construct_block(INGL_SHOW, data);

    Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    for (auto i = 0llu; i < show1_data.size(); ++i) {
        if (show1_data.at(i) != show_data.at(i))
            std::cout << "Wrong at " << i << ", " << std::to_integer<char>(show1_data.at(i)) << "\n";
    }
    auto is_equal = show1_data.size() == show_data.size() && std::equal(show1_data.begin(), show1_data.end(), show_data.begin());
    (void)is_equal;
    CHECK(is_equal);

    // now check that things loaded correctly
    CHECK(show1.GetNumPoints() == 1);
    CHECK(show1.GetNumSheets() == 0);
    CHECK(show1.GetPointLabel(0) == "point");
}

TEST_CASE("RoundTripWithDifferentShowModes", "CalChartShowTests")
{
    using namespace CalChart;
    Show show1(ShowMode::CreateShowMode({ { 36, 52, 8, 8, 8, 8, -80, -42, 160, 84 } }, kDefaultYardLines));
    CHECK(show1.GetShowMode().HashW() == 36);
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);

    Show show2(ShowMode::GetDefaultShowMode(), Reader({ show1_data.data(), show1_data.size() }));
    CHECK(show2.GetShowMode().HashW() == 36);
}

TEST_CASE("BlankDescription", "CalChartShowTests")
{
    using namespace CalChart;
    auto show_zero_points_zero_labels_zero_description = Construct_show_zero_points_zero_labels_zero_description();
    Show show1(ShowMode::GetDefaultShowMode(),
        Reader({ show_zero_points_zero_labels_zero_description.data(),
            show_zero_points_zero_labels_zero_description.size() }));
    auto show1_data = show1.SerializeShow();
    // eat header
    show1_data.erase(show1_data.begin(), show1_data.begin() + 8);
    bool is_equal = show1_data.size() == show_zero_points_zero_labels_zero_description.size() && std::equal(show1_data.begin(), show1_data.end(), show_zero_points_zero_labels_zero_description.begin());
    (void)is_equal;
    CHECK(!is_equal);
    CHECK(show1.GetNumPoints() == 0);
    CHECK(show1.GetNumSheets() == 0);

    // now remove the description and they should be equal
    auto show_zero_points_zero_labels = Construct_show_zero_points_zero_labels();
    Show show2(ShowMode::GetDefaultShowMode(), Reader({ show_zero_points_zero_labels.data(), show_zero_points_zero_labels.size() }));
    auto show2_data = show2.SerializeShow();
    show2_data.erase(show2_data.begin(), show2_data.begin() + 8);
    is_equal = show2_data.size() == show_zero_points_zero_labels.size() && std::equal(show2_data.begin(), show2_data.end(), show_zero_points_zero_labels.begin());
    CHECK(is_equal);
    CHECK(show1.GetNumPoints() == 0);
    CHECK(show1.GetNumSheets() == 0);
}

// confirm we try to handle shows from the future
TEST_CASE("FutureShow", "CalChartShowTests")
{
    using namespace CalChart;
    // how?  By creating a show from scratch, then modifying the version; make
    // sure that we load it, and it looks the same
    // except the data gets reverted
    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto char_data = std::vector<char>{};
    std::transform(blank_show_data.begin(), blank_show_data.end(), std::back_inserter(char_data), [](auto a) { return std::to_integer<char>(a); });
    CHECK(char_data.at(6) - '0' == CC_MAJOR_VERSION);
    CHECK(char_data.at(7) - '0' == CC_MINOR_VERSION);
    ++char_data.at(6);
    ++char_data.at(7);
    std::istringstream is(std::string{ char_data.data(), char_data.size() });
    auto re_read_show = Show::Create(ShowMode::GetDefaultShowMode(), is);
    auto re_read_show_data = blank_show->SerializeShow();
    --char_data.at(6);
    --char_data.at(7);
    bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
    (void)is_equal;
    CHECK(is_equal);
}

TEST_CASE("WrongSize", "CalChartShowTests")
{
    using namespace CalChart;
    auto points_3(Construct_block(INGL_SIZE, std::vector<uint8_t>(3)));
    auto show_data = Construct_block(INGL_SHOW, points_3);
    bool hit_exception = false;
    try {
        Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    CHECK(hit_exception);
}

// too large, and too small
TEST_CASE("WrongSizeLabel", "CalChartShowTests")
{
    using namespace CalChart;
    {
        std::vector<std::byte> point_data(4);
        details::put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto no_labels(Construct_block(INGL_LABL, std::vector<uint8_t>{}));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
        } catch (CC_FileException const&) {
            hit_exception = true;
        }
        (void)hit_exception;
        CHECK(hit_exception);
    }
    {
        std::vector<std::byte> point_data(4);
        details::put_big_long(point_data.data(), 1);
        auto points(Construct_block(INGL_SIZE, point_data));
        auto labels(Construct_block(INGL_LABL, std::vector<char>{ 'a', '\0', 'b', '\0' }));
        auto t_show_data = points;
        t_show_data.insert(t_show_data.end(), labels.begin(), labels.end());
        auto show_data = Construct_block(INGL_SHOW, t_show_data);
        bool hit_exception = false;
        try {
            Show show1(ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }));
        } catch (CC_FileException const&) {
            hit_exception = true;
        }
        (void)hit_exception;
        CHECK(hit_exception);
    }
}

// too large, and too small
TEST_CASE("WrongSizeDescription", "CalChartShowTests")
{
    using namespace CalChart;
    auto no_points = Construct_block(INGL_SIZE, std::vector<uint8_t>(4));
    auto no_labels = Construct_block(INGL_LABL, std::vector<uint8_t>{});
    auto descr = Construct_block(INGL_DESC, std::vector<char>{ 'a', 'b', 'c', '\0' });
    descr.at(9) = std::byte{};
    auto t_show_data = no_points;
    t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
    t_show_data.insert(t_show_data.end(), descr.begin(), descr.end());
    auto show_data = Construct_block(INGL_SHOW, t_show_data);
    bool hit_exception = false;
    try {
        auto show1 = Show{ ShowMode::GetDefaultShowMode(), Reader({ show_data.data(), show_data.size() }) };
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    CHECK(hit_exception);
}

// extra cruft ok
TEST_CASE("ExtraCruftOk", "CalChartShowTests")
{
    using namespace CalChart;
    // now remove the description and they should be equal
    auto extra_cruft = Construct_show_zero_points_zero_labels_1_sheet_and_random();
    auto show1 = Show{ ShowMode::GetDefaultShowMode(), Reader({ extra_cruft.data(), extra_cruft.size() }) };
    auto show1_data = show1.SerializeShow();

    auto blank_show = Show::Create(ShowMode::GetDefaultShowMode());
    auto blank_show_data = blank_show->SerializeShow();
    auto is_equal = blank_show_data.size() == show1_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), show1_data.begin());
    (void)is_equal;
    CHECK(is_equal);
}

// show with nothing should fail:
TEST_CASE("WithNothing", "CalChartShowTests")
{
    using namespace CalChart;
    std::vector<std::byte> empty{};
    bool hit_exception = false;
    try {
        auto show = Show{ ShowMode::GetDefaultShowMode(), Reader({ empty.data(), empty.size() }) };
    } catch (CC_FileException const&) {
        hit_exception = true;
    }
    (void)hit_exception;
    CHECK(hit_exception);
}

// show with nothing should fail:
TEST_CASE("DefaultEqual", "CalChartShowTests")
{
    auto empty_show = CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode());
    CHECK(empty_show != nullptr);
}

// show with nothing should fail:
TEST_CASE("Remapping", "CalChartShowTests")
{
    using namespace CalChart;
    auto show1 = Show::Create(ShowMode::GetDefaultShowMode());
    show1->Create_SetupMarchersCommand({ { "A", "A" }, { "B", "B" }, { "C", "C" }, { "D", "D" } }, 1, 0).first(*show1);
    {
        auto sheet = Sheet(4);
        sheet.SetPosition({ 0, 0 }, 0);
        sheet.SetPosition({ 1, 1 }, 1);
        sheet.SetPosition({ 2, 2 }, 2);
        sheet.SetPosition({ 3, 3 }, 3);
        show1->Create_AddSheetsCommand({ sheet }, 0).first(*show1);
    }
    auto show2 = Show::Create(ShowMode::GetDefaultShowMode());
    show2->Create_SetupMarchersCommand({ { "A", "A" }, { "B", "B" }, { "C", "C" }, { "D", "D" } }, 1, 0).first(*show2);
    {
        auto sheet = Sheet(4);
        sheet.SetPosition({ 0, 0 }, 0);
        sheet.SetPosition({ 1, 1 }, 1);
        sheet.SetPosition({ 2, 2 }, 2);
        sheet.SetPosition({ 3, 3 }, 3);
        show2->Create_AddSheetsCommand({ sheet }, 0).first(*show2);
    }
    auto mapping = show1->GetRelabelMapping(show1->GetAllMarcherPositions(0), show2->GetAllMarcherPositions(0), 1);
    CHECK(*mapping == std::vector<MarcherIndex>{ 0, 1, 2, 3 });

    {
        auto sheet = Sheet(4);
        sheet.SetPosition({ 0, 0 }, 3);
        sheet.SetPosition({ 1, 1 }, 2);
        sheet.SetPosition({ 2, 2 }, 1);
        sheet.SetPosition({ 3, 3 }, 0);
        show1->Create_AddSheetsCommand({ sheet }, 0).first(*show1);
    }
    mapping = show1->GetRelabelMapping(show1->GetAllMarcherPositions(0), show2->GetAllMarcherPositions(0), 1);
    CHECK(*mapping == std::vector<MarcherIndex>{ 3, 2, 1, 0 });

    {
        auto sheet = Sheet(4);
        sheet.SetPosition({ 0, 0 }, 0);
        sheet.SetPosition({ 1, 1 }, 1);
        sheet.SetPosition({ 2, 2 }, 2);
        sheet.SetPosition({ 13, 13 }, 3);
        show1->Create_AddSheetsCommand({ sheet }, 0).first(*show1);
    }
    mapping = show1->GetRelabelMapping(show1->GetAllMarcherPositions(0), show2->GetAllMarcherPositions(0), 1);
    CHECK(mapping.has_value() == false);
}
