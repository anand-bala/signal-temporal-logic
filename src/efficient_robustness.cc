/**
 * A lot of this code is based on the [Breach Matlab Toolbox][breach] but is
 * modified as follows:
 *
 * 1. The following code does not use a std::deque for the Signal (to enable
 *    vectorization by the compiler).
 * 2. We use C++17 standards and features (structured bindings, etc.).
 *
 * [breach]: https://github.com/decyphir/breach/
 */

#include "signal_tl/robustness.hh"

#include "utils.hh"

#include <cmath>
#include <limits>

namespace semantics {
using namespace signal;

namespace {

SignalPtr compute_not(const SignalPtr y) {
  auto out = std::make_shared<Signal>();
  for (const auto [t, v, d] : *y) { out->push_back_raw(Sample{t, -v, -d}); }
  return out;
}

SignalPtr compute_and(const SignalPtr y1, const SignalPtr y2) {
  return min(y1, y2);
}

SignalPtr compute_and(const std::vector<SignalPtr>& ys) {
  return min(ys);
}

SignalPtr compute_or(const SignalPtr y1, const SignalPtr y2) {
  return max(y1, y2);
}

SignalPtr compute_or(const std::vector<SignalPtr>& ys) {
  return max(ys);
}

SignalPtr compute_eventually(const SignalPtr y) {}
SignalPtr compute_eventually(const SignalPtr y, double a) {}
SignalPtr compute_always(const SignalPtr y) {}
SignalPtr compute_always(const SignalPtr y, double a) {}
SignalPtr compute_until(const SignalPtr y1, const SignalPtr y2) {}
SignalPtr compute_until(
    const SignalPtr y1,
    const SignalPtr y2,
    std::pair<double, double> interval) {}

} // namespace

template <>
SignalPtr compute_robustness<Semantics::EFFICIENT>(
    const ast::Expr phi,
    const signal::Trace& trace) {
  SignalPtr out;

  return out;
}

} // namespace semantics
