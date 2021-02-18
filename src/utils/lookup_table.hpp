/// @file     utils/lookup_table.hpp
/// @brief    C++17 compatible constexpr static map.
///

#pragma once
#ifndef ARGUS_UTILS_CONSTEXPR_MAP
#define ARGUS_UTILS_CONSTEXPR_MAP

#include <algorithm>
#include <array>
#include <map>
#include <stdexcept>
#include <string_view>

/// A constexpr map that can be used as a compile time lookup table. The code size
/// scales linearly (I think) in the size of the lookup table.
///
/// Based on [C++ Weekly - Ep 233 - std::map vs constexpr
/// map](https://www.youtube.com/watch?v=INn3xa4pMfg) with the following differences:
///
/// - C++17 compatibe by removing the call to `std::find_if` (C++20 makes it
/// `constexpr`).
/// - The above change also allows for clang to inline the search, marginally improving
///   performance.
template <typename Key, typename Value, std::size_t Size>
struct ComptimeMap {
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key& key) const {
    for (const auto& [k, v] : data) {
      if (k == key) {
        return v;
      }
    }
    throw std::range_error("Key not found in ComptimeMap");
  }
};

#endif /* end of include guard: ARGUS_UTILS_CONSTEXPR_MAP */
