#pragma once
/*
 * CalChartDataRegistry.hpp
 */

/*
   Copyright (C) 2017-2026  Richard Michael Powell

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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace CalChart {

// Strongly-typed key so it can't accidentally be mixed up with a plain
// int/size_t elsewhere in your code.
struct DataKey {
    std::size_t value = 0;

    auto operator==(const DataKey& other) const noexcept -> bool { return value == other.value; }
    auto operator!=(const DataKey& other) const noexcept -> bool { return !(*this == other); }
};
} // namespace CalChart

namespace std {
template <> struct hash<CalChart::DataKey> {
    std::size_t operator()(const CalChart::DataKey& k) const noexcept { return std::hash<std::size_t>{}(k.value); }
};
}

namespace CalChart {
// DataRegistry<DataT, HashFn, EqFn>
//
// Register a data item and get back a stable key. Registering the same data
// content twice returns the *same* key instead of storing a duplicate.
// Asking for a data item by key gives you back the stored data.
//
//   DataT  - your data type (must be copyable/movable)
//   HashFn  - callable: std::size_t operator()(const DataT&) const
//   EqFn    - callable: bool operator()(const DataT&, const DataT&) const
//
// Dedup strategy: hash the content first, then confirm with a full equality
// check so hash collisions can't cause two different images to collapse
// into one key.
//
// Not thread-safe as written; wrap calls in a mutex if you need concurrent
// access.
template <typename DataT, typename HashFn = std::hash<DataT>, typename EqFn = std::equal_to<DataT>> class DataRegistry {
public:
    // Register a data item. Returns the existing key if this exact data is
    // already registered, otherwise stores it and returns a new key.
    auto Register(DataT image) -> DataKey
    {
        std::size_t h = hash_fn_(image);

        auto range = by_hash_.equal_range(h);
        for (auto it = range.first; it != range.second; ++it) {
            std::size_t idx = it->second;
            if (eq_fn_(data_[idx], image)) {
                return DataKey{ idx };
            }
        }

        std::size_t idx = data_.size();
        data_.push_back(std::move(image));
        by_hash_.emplace(h, idx);
        return DataKey{ idx };
    }

    // Retrieve a previously registered image by key. Throws std::out_of_range
    // if the key is invalid.
    auto Get(DataKey key) const -> DataT const&
    {
        RequireValid(key);
        return data_[key.value];
    }

    auto Contains(DataKey key) const noexcept -> bool { return key.value < data_.size(); }

    auto Size() const noexcept -> std::size_t { return data_.size(); }

    auto Empty() const noexcept -> bool { return data_.empty(); }

private:
    void RequireValid(DataKey key) const
    {
        if (key.value >= data_.size()) {
            throw std::out_of_range("DataRegistry::Get - invalid key");
        }
    }

    std::vector<DataT> data_;
    std::unordered_multimap<std::size_t, std::size_t> by_hash_; // content hash -> index
    HashFn hash_fn_;
    EqFn eq_fn_;
};

} // namespace CalChart
