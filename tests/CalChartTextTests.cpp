#include "CalChartText.h"
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity, misc-use-anonymous-namespace)

using text_vector = std::vector<std::variant<CalChart::TextChunk, CalChart::TabChunk, CalChart::SymbolChunk>>;

TEST_CASE("CalChartText", "basics")
{
    auto textline = CalChart::ParseTextLine("");
    CHECK(textline.chunks.empty());
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("<");
    CHECK(textline.chunks.empty());
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK_FALSE(textline.on_sheet);

    textline = CalChart::ParseTextLine(">");
    CHECK(textline.chunks.empty());
    CHECK_FALSE(textline.center);
    CHECK_FALSE(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("~");
    CHECK(textline.chunks.empty());
    CHECK(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("<>~");
    CHECK(textline.chunks.empty());
    CHECK(textline.center);
    CHECK_FALSE(textline.on_main);
    CHECK_FALSE(textline.on_sheet);

    textline = CalChart::ParseTextLine("Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("~Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
    CHECK(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("<>~Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
    CHECK(textline.center);
    CHECK_FALSE(textline.on_main);
    CHECK_FALSE(textline.on_sheet);

    textline = CalChart::ParseTextLine("Hello<>~");
    CHECK(textline.chunks.size() == 1);
    CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello<>~", CalChart::PSFONT::NORM });
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);
}

TEST_CASE("CalChartPrintContinuityLayout")
{
    SECTION("symbols")
    {
        auto textline = CalChart::ParseTextLine("\\po");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL });

        textline = CalChart::ParseTextLine("\\po\\so");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL },
                  CalChart::TextChunk{ "B", CalChart::PSFONT::SYMBOL },
              });

        CHECK_THROWS(textline = CalChart::ParseTextLine("\\p3"));

        textline = CalChart::ParseTextLine("hello\\po\\so");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "hello", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL },
                  CalChart::TextChunk{ "B", CalChart::PSFONT::SYMBOL },
              });
    }

    SECTION("basics")
    {
        auto textline = CalChart::ParseTextLine("");
        CHECK(textline.chunks.empty());
        CHECK_FALSE(textline.center);
        CHECK(textline.on_main);
        CHECK(textline.on_sheet);

        textline = CalChart::ParseTextLine("<");
        CHECK(textline.chunks.empty());
        CHECK_FALSE(textline.center);
        CHECK(textline.on_main);
        CHECK_FALSE(textline.on_sheet);

        textline = CalChart::ParseTextLine(">");
        CHECK(textline.chunks.empty());
        CHECK_FALSE(textline.center);
        CHECK_FALSE(textline.on_main);
        CHECK(textline.on_sheet);

        textline = CalChart::ParseTextLine("~");
        CHECK(textline.chunks.empty());
        CHECK(textline.center);
        CHECK(textline.on_main);
        CHECK(textline.on_sheet);

        textline = CalChart::ParseTextLine("<>~");
        CHECK(textline.chunks.empty());
        CHECK(textline.center);
        CHECK_FALSE(textline.on_main);
        CHECK_FALSE(textline.on_sheet);

        textline = CalChart::ParseTextLine("Hello");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
        CHECK_FALSE(textline.center);
        CHECK(textline.on_main);
        CHECK(textline.on_sheet);

        textline = CalChart::ParseTextLine("~Hello");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
        CHECK(textline.center);
        CHECK(textline.on_main);
        CHECK(textline.on_sheet);

        textline = CalChart::ParseTextLine("<>~Hello");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });
        CHECK(textline.center);
        CHECK_FALSE(textline.on_main);
        CHECK_FALSE(textline.on_sheet);

        textline = CalChart::ParseTextLine("Hello<>~");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello<>~", CalChart::PSFONT::NORM });
        CHECK_FALSE(textline.center);
        CHECK(textline.on_main);
        CHECK(textline.on_sheet);
    }

    SECTION("parsing")
    {
        auto textline = CalChart::ParseTextLine("Hello");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "Hello", CalChart::PSFONT::NORM });

        textline = CalChart::ParseTextLine("  Hello");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "  Hello", CalChart::PSFONT::NORM });

        textline = CalChart::ParseTextLine("Hell\to");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "Hell", CalChart::PSFONT::NORM },
                  CalChart::TabChunk{},
                  CalChart::TextChunk{ "o", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("Hell\\\to");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "Hell", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "\\", CalChart::PSFONT::NORM },
                  CalChart::TabChunk{},
                  CalChart::TextChunk{ "o", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("\\bsHell\\beo");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "Hell", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "o", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("Hell\\bso");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "Hell", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "o", CalChart::PSFONT::BOLD },
              });

        textline = CalChart::ParseTextLine("Hell\\beo");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "Hell", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "o", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("\\bsHe\\isl\\iel\\beo");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "He", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "o", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\bse\\isl\\bel\\ieo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\ise\\bsl\\bel\\ieo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\Bse\\Isl\\Bel\\Ieo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\ise\\Bsl\\bel\\Ieo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\ise\\iel\\bsl\\beo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLD },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        textline = CalChart::ParseTextLine("H\\ise\\bel\\bsl\\beo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::ITAL },
              });

        textline = CalChart::ParseTextLine("H\\ise\\Bsl\\bsl\\isl\\bel\\Ieo!");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "H", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "e", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::BOLDITAL },
                  CalChart::TextChunk{ "l", CalChart::PSFONT::ITAL },
                  CalChart::TextChunk{ "o!", CalChart::PSFONT::NORM },
              });

        CHECK_THROWS(textline = CalChart::ParseTextLine("H\\iat!"));
    }

    SECTION("CalChartTextSymbols")
    {
        auto textline = CalChart::ParseTextLine("\\po");
        CHECK(textline.chunks.size() == 1);
        CHECK(std::get<CalChart::TextChunk>(textline.chunks.at(0)) == CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL });

        textline = CalChart::ParseTextLine("\\po\\so");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL },
                  CalChart::TextChunk{ "B", CalChart::PSFONT::SYMBOL },
              });

        CHECK_THROWS(textline = CalChart::ParseTextLine("\\p3"));

        textline = CalChart::ParseTextLine("hello\\po\\so");
        CHECK(textline.chunks == text_vector{
                  CalChart::TextChunk{ "hello", CalChart::PSFONT::NORM },
                  CalChart::TextChunk{ "A", CalChart::PSFONT::SYMBOL },
                  CalChart::TextChunk{ "B", CalChart::PSFONT::SYMBOL },
              });
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-do-while, readability-magic-numbers, readability-function-cognitive-complexity, misc-use-anonymous-namespace)
