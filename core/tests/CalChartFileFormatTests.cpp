#include "CalChartFileFormat.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basics", "CalChartParser")
{

    auto a = std::vector<std::byte>{
        std::byte{ 0 },
        std::byte{ 1 },
        std::byte{ 2 },
        std::byte{ 3 },
        std::byte{ 4 },
        std::byte{ 5 },
        std::byte{ 6 },
        std::byte{ 7 },
        std::byte{ 8 },
        std::byte{ 9 },
        std::byte{ 10 },
        std::byte{ 11 },
        std::byte{ 12 }
    };

    auto reader = CalChart::Reader({ a.data(), a.size() });
    CHECK(reader.Peek<uint16_t>() == 0x0001);
    CHECK(reader.Peek<uint32_t>() == 0x00010203);
    CHECK(reader.Peek<uint16_t>() == 0x0001);
    CHECK(reader.Peek<uint32_t>() == 0x00010203);

    CHECK(reader.Get<uint16_t>() == 0x0001);
    CHECK(reader.Get<uint32_t>() == 0x02030405);
    CHECK(reader.Get<uint16_t>() == 0x0607);
    CHECK(reader.Get<uint32_t>() == 0x08090a0b);
}

template <typename T>
void TestSerialize()
{
    auto uut = std::vector<std::byte>{};
    auto value = static_cast<T>(rand() - rand());
    CalChart::Parser::Append(uut, value);
    auto reader = CalChart::Reader({ uut.data(), uut.size() });
    CHECK(reader.Peek<T>() == value);
    CHECK(reader.Get<T>() == value);
    CHECK(reader.size() == 0);
}

template <typename T>
void TestSerializeVector(int num)
{
    auto uut = std::vector<std::byte>{};
    auto data = std::vector<T>{};
    while (num-- > 0) {
        data.push_back(static_cast<T>(rand()));
    }
    CalChart::Parser::Append(uut, static_cast<uint32_t>(data.size()));
    CalChart::Parser::Append(uut, data);
    auto reader = CalChart::Reader({ uut.data(), uut.size() });
    CHECK(reader.GetVector<T>() == data);
    CHECK(reader.size() == 0);
}

TEST_CASE("Advanced", "CalChartParser")
{
    auto numCases = 10;
    while (numCases-- > 0) {

        TestSerialize<uint8_t>();
        TestSerialize<char>();
        TestSerialize<uint16_t>();
        TestSerialize<int16_t>();
        TestSerialize<uint32_t>();
        TestSerialize<int32_t>();
        TestSerialize<float>();

        TestSerializeVector<uint8_t>(10);
        TestSerializeVector<char>(10);
        TestSerializeVector<uint16_t>(10);
        TestSerializeVector<int16_t>(10);
        TestSerializeVector<uint32_t>(10);
        TestSerializeVector<int32_t>(10);
        TestSerializeVector<float>(10);
    }
}
