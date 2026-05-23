#pragma once
/*
 * CalChartMeasurements.h
 * Utilities for measuring performance and other metrics.
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartTypes.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iosfwd>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

namespace CalChart {

// Statistics for a single paint component
struct PerformanceStat {
    std::string name;
    size_t callCount{ 0 };
    Seconds average{ 0.0 };
    Seconds stdDev{ 0.0 };
    Seconds min{ 0.0 };
    Seconds max{ 0.0 };
    Seconds last{ 0.0 };
    auto operator<=>(PerformanceStat const&) const = default;
};

template <size_t Size = 128>
struct MeasureDuration {
    MeasureDuration(std::string whatToMeasure)
        : whatToMeasure{ std::move(whatToMeasure) }
    {
    }

    auto doMeasurement()
    {
        return MeasureDurationTick([this](Seconds seconds) {
            addMeasure(seconds);
        });
    }

    // prefer doMeasure (RAII behavior) over addMeasure (for testing)
    void addMeasure(Seconds seconds)
    {
        auto guard = std::lock_guard{ mMutex };
        auto where = count++ % Size;
        measurements[where] = seconds;
    }

    [[nodiscard]] auto currentAverage() const
    {
        return GetStats().average;
    }

    [[nodiscard]] auto GetStats() const
    {
        auto guard = std::lock_guard{ mMutex };
        if (count == 0) {
            return PerformanceStat{ .name = whatToMeasure };
        }
        auto runningCount = count < Size ? count : Size;
        auto end = measurements.begin() + runningCount;
        // Calculate mean
        auto sum = std::accumulate(measurements.begin(), end, Seconds{}, [](auto&& total, auto&& measurement) { return total + measurement; });
        auto mean = sum / static_cast<double>(runningCount);

        // Calculate standard deviation
        auto variance = std::accumulate(measurements.begin(), end, Seconds{}, [=](auto&& total, auto&& measurement) {
            auto diff = measurement - mean;
            return total + Seconds{ diff.count() * diff.count() };
        }) / static_cast<double>(runningCount);

        // Find min and max
        auto [minIt, maxIt] = std::minmax_element(measurements.begin(), end);
        return PerformanceStat{
            whatToMeasure,
            count,
            mean,
            Seconds{ std::sqrt(variance.count()) },
            *minIt,
            *maxIt,
            measurements[(count - 1) % Size],
        };
    }

    auto WhatToMeasure() { return whatToMeasure; }

    auto clear()
    {
        auto guard = std::lock_guard{ mMutex };
        count = 0;
    }

    friend auto operator<<(std::ostream& os, MeasureDuration const& measure) -> std::ostream&
    {
        return os << "Time for " << measure.whatToMeasure << " : " << (measure.measurements.empty() ? 0.0 : measure.measurements.back().count()) << " (average: " << measure.GetStats().average << ")";
    }

private:
    template <typename Function>
    struct MeasureDurationTick {
        explicit MeasureDurationTick(Function function)
            : completion(function)
        {
        }
        ~MeasureDurationTick()
        {
            try {
                completion(std::chrono::duration_cast<Seconds>(std::chrono::steady_clock::now() - start));
            } catch (...) {
            }
        }

        std::chrono::time_point<std::chrono::steady_clock> const start = std::chrono::steady_clock::now();
        Function completion;
    };

    std::mutex mutable mMutex{};
    std::string whatToMeasure;
    std::array<Seconds, Size> measurements;
    size_t count{};
};

}