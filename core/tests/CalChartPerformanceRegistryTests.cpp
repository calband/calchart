/*
 * CalChartPerformanceRegistryTests.cpp
 * Unit tests for performance tracking registry
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CalChartPerformanceRegistry.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)

TEST_CASE("PerformanceRegistry", "basic operations")
{
    CalChart::PerformanceRegistry registry;

    SECTION("Initial state")
    {
        CHECK(registry.GetRegisteredCount() == 0);
        CHECK(registry.GetAllStats().empty());
    }

    SECTION("Register and deregister components")
    {
        int component1 = 1;
        int component2 = 2;

        registry.RegisterComponent(&component1, "Component1");
        CHECK(registry.GetRegisteredCount() == 1);
        CHECK(registry.IsRegistered(&component1));
        CHECK_FALSE(registry.IsRegistered(&component2));

        registry.RegisterComponent(&component2, "Component2");
        CHECK(registry.GetRegisteredCount() == 2);
        CHECK(registry.IsRegistered(&component2));

        registry.DeregisterComponent(&component1);
        CHECK(registry.GetRegisteredCount() == 1);
        CHECK_FALSE(registry.IsRegistered(&component1));
        CHECK(registry.IsRegistered(&component2));

        registry.DeregisterComponent(&component2);
        CHECK(registry.GetRegisteredCount() == 0);
    }

    SECTION("Record measurements")
    {
        int component = 1;
        registry.RegisterComponent(&component, "TestComponent");

        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.001 }); // 1ms
        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.002 }); // 2ms
        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.003 }); // 3ms

        auto [ptr, stats] = registry.GetStats(&component);
        CHECK(ptr == &component);
        CHECK(stats.name == "TestComponent");
        CHECK(stats.callCount == 3);
        CHECK(stats.average.count() > 0.0);
        CHECK(stats.min.count() > 0.0);
        CHECK(stats.max.count() > 0.0);
        CHECK(stats.last.count() > 0.0);
    }

    SECTION("Statistics calculation")
    {
        int component = 1;
        registry.RegisterComponent(&component, "TestComponent");

        // Record some known measurements (in seconds)
        registry.RecordMeasurement(&component, CalChart::Seconds{ 1.0 });
        registry.RecordMeasurement(&component, CalChart::Seconds{ 2.0 });
        registry.RecordMeasurement(&component, CalChart::Seconds{ 3.0 });

        auto [ptr, stats] = registry.GetStats(&component);
        CHECK(stats.callCount == 3);
        CHECK(stats.average.count() == 2.0); // (1+2+3)/3 = 2ms
        CHECK(stats.min.count() == 1.0);
        CHECK(stats.max.count() == 3.0);
        CHECK(stats.last.count() == 3.0);
    }

    SECTION("Get all stats")
    {
        int component1 = 1;
        int component2 = 2;

        registry.RegisterComponent(&component1, "Component1");
        registry.RegisterComponent(&component2, "Component2");

        registry.RecordMeasurement(&component1, CalChart::Seconds{ 0.001 });
        registry.RecordMeasurement(&component2, CalChart::Seconds{ 0.002 });

        auto allStats = registry.GetAllStats();
        CHECK(allStats.size() == 2);

        // Find each component in the results
        bool found1 = false;
        bool found2 = false;
        for (auto const& [ptr, stats] : allStats) {
            if (ptr == &component1) {
                CHECK(stats.name == "Component1");
                found1 = true;
            } else if (ptr == &component2) {
                CHECK(stats.name == "Component2");
                found2 = true;
            }
        }
        CHECK(found1);
        CHECK(found2);
    }

    SECTION("Reset measurements")
    {
        int component = 1;
        registry.RegisterComponent(&component, "TestComponent");

        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.001 });
        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.002 });

        auto [ptr1, statsBefore] = registry.GetStats(&component);
        CHECK(statsBefore.callCount == 2);

        registry.ResetMeasurements();

        // Component should still be registered
        CHECK(registry.IsRegistered(&component));
        CHECK(registry.GetRegisteredCount() == 1);

        // But measurements should be cleared
        auto [ptr2, statsAfter] = registry.GetStats(&component);
        CHECK(statsAfter.callCount == 0);
    }

    SECTION("Null pointer handling")
    {
        // Should not crash
        registry.RegisterComponent(nullptr, "NullComponent");
        registry.RecordMeasurement(nullptr, CalChart::Seconds{ 0.001 });
        registry.DeregisterComponent(nullptr);
        CHECK(registry.GetRegisteredCount() == 0);
    }

    SECTION("Unregistered component measurements")
    {
        int component = 1;

        // Try to record measurement for unregistered component
        registry.RecordMeasurement(&component, CalChart::Seconds{ 0.001 });

        // Should not crash or create a registration
        CHECK(registry.GetRegisteredCount() == 0);
    }
}

TEST_CASE("PerformanceMeasure", "RAII measurement")
{
    CalChart::PerformanceRegistry registry;
    int component = 1;

    registry.RegisterComponent(&component, "TestComponent");

    SECTION("Automatic measurement recording")
    {
        {
            CalChart::PerformanceMeasure measure(registry, &component);
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } // Measurement recorded on destruction

        auto [ptr, stats] = registry.GetStats(&component);
        CHECK(stats.callCount == 1);
        CHECK(stats.average.count() > 0.0);
    }

    SECTION("Multiple measurements")
    {
        for (int i = 0; i < 5; ++i) {
            CalChart::PerformanceMeasure measure(registry, &component);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto [ptr, stats] = registry.GetStats(&component);
        CHECK(stats.callCount == 5);
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, readability-function-cognitive-complexity)
