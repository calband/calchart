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

#include <chrono>
#include <iosfwd>
#include <numeric>
#include <string>
#include <vector>

namespace CalChart {

struct MeasureDuration {
    explicit MeasureDuration(std::string whatToMeasure)
        : whatToMeasure{ std::move(whatToMeasure) }
    {
    }

    auto doMeasurement()
    {
        return MeasureDurationTick([this](double count) {
            total.push_back(count);
        });
    }

    [[nodiscard]] auto currentAverage() const
    {
        return std::accumulate(total.begin(), total.end(), 0.0) / static_cast<double>(total.size());
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
                const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
                completion(diff.count());
            } catch (...) {
            }
        }

        std::chrono::time_point<std::chrono::steady_clock> const start = std::chrono::steady_clock::now();
        Function completion;
    };

    std::string whatToMeasure;
    std::vector<double> total;
    friend auto operator<<(std::ostream& os, MeasureDuration const& measure) -> std::ostream&;
};

inline auto operator<<(std::ostream& os, MeasureDuration const& measure) -> std::ostream&
{
    return os << "Time for " << measure.whatToMeasure << " : " << (measure.total.empty() ? 0.0 : measure.total.back()) << " (average: " << measure.currentAverage() << ")";
}

}