#include "CalChartMeasure.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

using namespace CalChart;
TEST_CASE("CalChartMeasure", "basics")
{
    MeasureDuration<4> uut{ "test" };
    auto stats = uut.GetStats();
    CHECK(stats == PerformanceStat{
              .name = "test",
              .callCount = 0,
              .average = Seconds{ 0 },
              .stdDev = Seconds{ 0 },
              .min = Seconds{ 0 },
              .max = Seconds{ 0 },
              .last = Seconds{ 0 },
          });

    uut.addMeasure(CalChart::Seconds{ 1 });
    stats = uut.GetStats();
    CHECK(stats == PerformanceStat{
              .name = "test",
              .callCount = 1,
              .average = Seconds{ 1 },
              .stdDev = Seconds{ 0 },
              .min = Seconds{ 1 },
              .max = Seconds{ 1 },
              .last = Seconds{ 1 },
          });

    uut.addMeasure(CalChart::Seconds{ 9 });
    uut.addMeasure(CalChart::Seconds{ 5 });
    stats = uut.GetStats();
    CHECK(stats == PerformanceStat{
              .name = "test",
              .callCount = 3,
              .average = Seconds{ 5 },
              .stdDev = Seconds{ 3.265986323710904 },
              .min = Seconds{ 1 },
              .max = Seconds{ 9 },
              .last = Seconds{ 5 },
          });

    uut.addMeasure(CalChart::Seconds{ 13 });
    uut.addMeasure(CalChart::Seconds{ 9 });
    stats = uut.GetStats();
    CHECK(stats == PerformanceStat{
              .name = "test",
              .callCount = 5,
              .average = Seconds{ 9 },
              .stdDev = Seconds{ 2.8284271247461903 },
              .min = Seconds{ 5 },
              .max = Seconds{ 13 },
              .last = Seconds{ 9 },
          });
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
