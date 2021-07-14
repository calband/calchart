#include "catch2/catch.hpp"
#include "CalChartText.h"

TEST_CASE( "CalChartText", "basics" ) {
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
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "Hello", PSFONT::NORM });
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("~Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "Hello", PSFONT::NORM });
    CHECK(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);

    textline = CalChart::ParseTextLine("<>~Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "Hello", PSFONT::NORM });
    CHECK(textline.center);
    CHECK_FALSE(textline.on_main);
    CHECK_FALSE(textline.on_sheet);

    textline = CalChart::ParseTextLine("Hello<>~");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "Hello<>~", PSFONT::NORM });
    CHECK_FALSE(textline.center);
    CHECK(textline.on_main);
    CHECK(textline.on_sheet);
}

TEST_CASE( "CalChartTextParsing", "parsing" ) {
    auto textline = CalChart::ParseTextLine("Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "Hello", PSFONT::NORM });

    textline = CalChart::ParseTextLine("  Hello");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "  Hello", PSFONT::NORM });

    textline = CalChart::ParseTextLine("Hell\to");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "Hell", PSFONT::NORM },
        CalChart::Textchunk{ "", PSFONT::TAB },
        CalChart::Textchunk{ "o", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("Hell\\\to");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "Hell", PSFONT::NORM },
        CalChart::Textchunk{ "\\", PSFONT::NORM },
        CalChart::Textchunk{ "", PSFONT::TAB },
        CalChart::Textchunk{ "o", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("\\bsHell\\beo");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "Hell", PSFONT::BOLD },
        CalChart::Textchunk{ "o", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("Hell\\bso");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "Hell", PSFONT::NORM },
        CalChart::Textchunk{ "o", PSFONT::BOLD },
    });

    textline = CalChart::ParseTextLine("Hell\\beo");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "Hell", PSFONT::NORM },
        CalChart::Textchunk{ "o", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("\\bsHe\\isl\\iel\\beo");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "He", PSFONT::BOLD },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLD },
        CalChart::Textchunk{ "o", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\bse\\isl\\bel\\ieo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::BOLD },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\ise\\bsl\\bel\\ieo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\Bse\\Isl\\Bel\\Ieo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::BOLD },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\ise\\Bsl\\bel\\Ieo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\ise\\iel\\bsl\\beo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::NORM },
        CalChart::Textchunk{ "l", PSFONT::BOLD },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    textline = CalChart::ParseTextLine("H\\ise\\bel\\bsl\\beo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "o!", PSFONT::ITAL },
    });

    textline = CalChart::ParseTextLine("H\\ise\\Bsl\\bsl\\isl\\bel\\Ieo!");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "H", PSFONT::NORM },
        CalChart::Textchunk{ "e", PSFONT::ITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::BOLDITAL },
        CalChart::Textchunk{ "l", PSFONT::ITAL },
        CalChart::Textchunk{ "o!", PSFONT::NORM },
    });

    CHECK_THROWS(textline = CalChart::ParseTextLine("H\\iat!"));
}

TEST_CASE( "CalChartTextSymbols" ) {
    auto textline = CalChart::ParseTextLine("\\po");
    CHECK(textline.chunks.size() == 1);
    CHECK(textline.chunks.at(0) == CalChart::Textchunk{ "A", PSFONT::SYMBOL });

    textline = CalChart::ParseTextLine("\\po\\so");
    CHECK(textline.chunks == std::vector{ CalChart::Textchunk{ "A", PSFONT::SYMBOL },
        CalChart::Textchunk{ "B", PSFONT::SYMBOL },
    });

    CHECK_THROWS(textline = CalChart::ParseTextLine("\\p3"));

    textline = CalChart::ParseTextLine("hello\\po\\so");
    CHECK(textline.chunks == std::vector{
        CalChart::Textchunk{ "hello", PSFONT::NORM },
        CalChart::Textchunk{ "A", PSFONT::SYMBOL },
        CalChart::Textchunk{ "B", PSFONT::SYMBOL },
    });
}

