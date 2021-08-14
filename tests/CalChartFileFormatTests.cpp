#include "catch2/catch.hpp"
#include "CalChartParser.h"

TEST_CASE( "CalChartParserBasics" ) {
    auto a = std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    auto reader = CalChart::Reader({a.data(), a.size()});
    CHECK(reader.Peek<uint16_t>() == 0x0001);
    CHECK(reader.Peek<uint32_t>() == 0x00010203);
    CHECK(reader.Peek<uint16_t>() == 0x0001);
    CHECK(reader.Peek<uint32_t>() == 0x00010203);

    CHECK(reader.Get<uint16_t>() == 0x0001);
    CHECK(reader.Get<uint32_t>() == 0x02030405);
    CHECK(reader.Get<uint16_t>() == 0x0607);
    CHECK(reader.Get<uint32_t>() == 0x08090a0b);
}
