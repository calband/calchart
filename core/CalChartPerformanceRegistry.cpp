/*
 * CalChartPerformanceRegistry.cpp
 * Implementation of performance tracking registry
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
#include <algorithm>
#include <limits>

namespace CalChart {

void PerformanceRegistry::RegisterComponent(void const* componentPtr, std::string componentName)
{
    if (componentPtr == nullptr) {
        return;
    }

    mComponents.try_emplace(componentPtr, componentName);
}

void PerformanceRegistry::DeregisterComponent(void const* componentPtr)
{
    if (componentPtr == nullptr) {
        return;
    }

    mComponents.erase(componentPtr);
}

auto PerformanceRegistry::IsRegistered(void const* componentPtr) const -> bool
{
    return mComponents.find(componentPtr) != mComponents.end();
}

void PerformanceRegistry::RecordMeasurement(void const* componentPtr, Seconds seconds)
{
    auto it = mComponents.find(componentPtr);
    if (it != mComponents.end()) {
        it->second.measurements.addMeasure(seconds);
    }
}

auto PerformanceRegistry::GetStats(void const* componentPtr) const -> PerformanceStats
{
    auto it = mComponents.find(componentPtr);
    if (it == mComponents.end()) {
        return PerformanceStats{};
    }

    return { componentPtr, it->second.measurements.GetStats() };
}

auto PerformanceRegistry::GetAllStats() const -> std::vector<PerformanceStats>
{
    std::vector<PerformanceStats> results;
    results.reserve(mComponents.size());

    for (auto const& [ptr, data] : mComponents) {
        results.push_back(CalculateStats(ptr, data));
    }

    return results;
}

void PerformanceRegistry::ResetMeasurements()
{
    for (auto& [ptr, data] : mComponents) {
        data.measurements.clear();
    }
}

auto PerformanceRegistry::GetRegisteredCount() const -> size_t
{
    return mComponents.size();
}

auto PerformanceRegistry::CalculateStats(ComponentKey ptr, ComponentData const& data) -> PerformanceStats
{
    return { ptr, data.measurements.GetStats() };
}

} // namespace CalChart
