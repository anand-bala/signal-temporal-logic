#ifndef ARGUS_UTILS_STATIC_ANALYSIS_HEPLERS_HPP
#define ARGUS_UTILS_STATIC_ANALYSIS_HEPLERS_HPP

#include <cassert>

// LCOV_EXCL_START

namespace utils {
[[noreturn]] inline void unreachable() {
  do {
    assert(false && "Unreachable code!");
#ifdef _MSC_VER
    __assume(false);
#elif defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)
    __builtin_unreachable();
#else
    abort();
#endif
  } while (false);
}

} // namespace utils

// LCOV_EXCL_STOP
#endif /* end of include guard: ARGUS_UTILS_STATIC_ANALYSIS_HEPLERS_HPP */
