#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_UTILS_HH__
#define __SIGNAL_TEMPORAL_LOGIC_UTILS_HH__

#include <iterator>
#include <type_traits>

namespace utils {

/**
 * Visit helper for a set of visitor lambdas.
 *
 * @see https://en.cppreference.com/w/cpp/utility/variant/visit
 */
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

} // namespace utils

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_UTILS_HH__ */
