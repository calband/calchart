#include "CalChartFileFormat.h"
#include "CalChartShowMode.h"
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
TEST_CASE("ShowModeBasics", "CalChartShowMode")
{
    auto uut1 = CalChart::ShowMode::GetDefaultShowMode();
    auto data = uut1.Serialize();
    auto uut2 = CalChart::ShowMode::CreateShowMode(CalChart::Reader({ data.data(), data.size() }));
    CHECK(uut1 == uut2);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
