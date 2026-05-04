#pragma once
/*
 * CalChartPerformanceRegistry.h
 * Registry for tracking performance metrics
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

#include "CalChartMeasure.h"
#include <chrono>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace CalChart {

// Statistics for a single paint component
using PerformanceStats = std::pair<void const*, PerformanceStat>;

// Registry for tracking performance of OnPaint/OnDraw functions
// Thread-safe for registration/deregistration and recording measurements
class PerformanceRegistry {
public:
    // Get the performance registry for tracking OnPaint/OnDraw performance
    static PerformanceRegistry& GetGlobalPerformanceRegistry()
    {
        static PerformanceRegistry sPerformanceRegistry;
        return sPerformanceRegistry;
    }

    PerformanceRegistry() = default;
    ~PerformanceRegistry() = default;

    // Non-copyable to avoid accidental copies
    PerformanceRegistry(PerformanceRegistry const&) = delete;
    PerformanceRegistry& operator=(PerformanceRegistry const&) = delete;
    PerformanceRegistry(PerformanceRegistry&&) = default;
    PerformanceRegistry& operator=(PerformanceRegistry&&) = default;

    // Register a component that will be tracked
    // componentPtr: unique identifier (typically 'this' pointer)
    // componentName: human-readable name (e.g., "FieldCanvas::OnPaint")
    void RegisterComponent(void const* componentPtr, std::string componentName);

    // Deregister a component (typically called in destructor)
    void DeregisterComponent(void const* componentPtr);

    // Check if a component is registered
    [[nodiscard]] auto IsRegistered(void const* componentPtr) const -> bool;

    // Record a measurement for a component
    // Should be called after timing a paint operation
    void RecordMeasurement(void const* componentPtr, Seconds seconds);

    // Get performance statistics for a specific component
    [[nodiscard]] auto GetStats(void const* componentPtr) const -> PerformanceStats;

    // Get performance statistics for all registered components
    [[nodiscard]] auto GetAllStats() const -> std::vector<PerformanceStats>;

    // Reset all measurements (keep registrations)
    void ResetMeasurements();

    // Get total number of registered components
    [[nodiscard]] auto GetRegisteredCount() const -> size_t;

private:
    struct ComponentData {
        MeasureDuration<> measurements; // in seconds
    };

    using ComponentKey = void const*;
    std::unordered_map<ComponentKey, ComponentData> mComponents;

    // Calculate statistics from a vector of measurements
    [[nodiscard]] static auto CalculateStats(ComponentKey ptr, ComponentData const& data) -> PerformanceStats;
};

// RAII helper for measuring paint performance
// This class manages the timing and automatic recording to the registry
class PerformanceMeasure {
public:
    PerformanceMeasure(PerformanceRegistry& registry, void const* componentPtr)
        : mRegistry(registry)
        , mComponentPtr(componentPtr)
        , mStart(std::chrono::steady_clock::now())
    {
    }

    ~PerformanceMeasure()
    {
        try {
            mRegistry.RecordMeasurement(mComponentPtr, std::chrono::duration_cast<Seconds>(std::chrono::steady_clock::now() - mStart));
        } catch (...) {
            // Ignore exceptions in destructor
        }
    }

    // Non-copyable, non-movable
    PerformanceMeasure(PerformanceMeasure const&) = delete;
    PerformanceMeasure& operator=(PerformanceMeasure const&) = delete;
    PerformanceMeasure(PerformanceMeasure&&) = delete;
    PerformanceMeasure& operator=(PerformanceMeasure&&) = delete;

private:
    PerformanceRegistry& mRegistry;
    void const* mComponentPtr;
    std::chrono::time_point<std::chrono::steady_clock> mStart;
};

// RAII for perf
class ScopedPerformanceRegistry {
public:
    ScopedPerformanceRegistry(PerformanceRegistry& registry, void const* componentPtr, std::string componentName)
        : mRegistry(registry)
        , mPtr(componentPtr)
    {
        mRegistry.RegisterComponent(mPtr, std::move(componentName));
    }

    ~ScopedPerformanceRegistry()
    {
        mRegistry.DeregisterComponent(mPtr);
    }

    auto doMeasure() -> PerformanceMeasure
    {
        return { mRegistry, mPtr };
    }

private:
    PerformanceRegistry& mRegistry;
    void const* mPtr;
};

} // namespace CalChart
