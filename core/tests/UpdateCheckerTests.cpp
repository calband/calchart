// Use Catch2 v3 includes. Catch2::Catch2WithMain target provides main.
#include <catch2/catch_test_macros.hpp>

#include "UpdateChecker.h"

using CalChart::CompareSemVer;
using CalChart::ParseLatestReleaseTagFromJson;

TEST_CASE("CompareSemVer basic comparisons")
{
    REQUIRE(CompareSemVer("v1.2.3", "1.2.3") == 0);
    REQUIRE(CompareSemVer("1.2.3", "1.2.4") == -1);
    REQUIRE(CompareSemVer("1.3.0", "1.2.9") == 1);
    REQUIRE(CompareSemVer("1.2", "1.2.0") == 0);
    REQUIRE(CompareSemVer("2.0.0", "10.0.0") == -1);
}

TEST_CASE("ParseLatestReleaseTagFromJson extracts tag")
{
    std::string json = R"({"url":"https://api.github.com/repos/calband/calchart/releases/1","tag_name":"v3.7.2","name":"Release 3.7.2"})";
    auto tag = ParseLatestReleaseTagFromJson(json);
    REQUIRE(tag.has_value());
    REQUIRE(*tag == "v3.7.2");

    std::string bad = R"({"not_tag":"nope"})";
    auto none = ParseLatestReleaseTagFromJson(bad);
    REQUIRE(!none.has_value());
}
