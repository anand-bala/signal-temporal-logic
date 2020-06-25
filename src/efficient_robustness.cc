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

#include "mono_wedge/mono_wedge.h"
#include "signal_tl/robustness.hh"

#include "minmax.hh"
#include "utils.hh"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>

namespace semantics {
using namespace signal;

namespace {

constexpr double TOP    = std::numeric_limits<double>::infinity();
constexpr double BOTTOM = -TOP;

/**
 * Wrapper around monotonic wedge minmax function
 *
 * TODO(anand): Need to complete
 */
template <typename Compare>
SignalPtr mono_wedge_minmax(const SignalPtr y, const double a, Compare comp) {
  std::deque<Sample> M;
  auto out = std::make_shared<Signal>();

  auto samples = std::deque<Sample>{y->begin(), y->end()};
  auto i       = samples.begin();

  // -- Read values in [0, a)
  for (; i->time < y->begin_time() + a; i++) {
    mono_wedge::mono_wedge_update(M, *i, comp);
  }

  for (; i != samples.end(); i++) {
    if (i->time - a > M.front().time) {
      // Add the sample for time M.front + a
      i = samples.insert(
          i,
          Sample{
              M.front().time + a, std::prev(i)->interpolate(M.front().time + a), 0.0});
    }
    mono_wedge::mono_wedge_update(M, *i, comp);
    while (M.front().time <= i->time - a) M.pop_front();
    out->push_back(i->time, i->value);
  }

  return out->simplify();
}

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

SignalPtr compute_eventually(const SignalPtr y) {
  std::vector<Sample> sig_stack;
  sig_stack.reserve(2 * y->size());

  double begin_time = y->begin_time();
  double end_time   = y->end_time();

  // Put the last element onto the stack
  sig_stack.push_back(y->front());

  for (auto it = std::next(y->rbegin()); it != y->rend(); it++) {
    const auto t    = it->time;
    const auto y_i  = it->value;
    const auto y_i1 = std::prev(it)->value;
    const auto z_i1 = sig_stack.back().value;

    if (y_i <= y_i1) {
      sig_stack.push_back(Sample{t, std::max(y_i1, z_i1), 0.0});
    } else if (y_i > y_i1 and y_i1 >= z_i1) {
      sig_stack.push_back(Sample{t, y_i, 0.0});
    } else if (z_i1 >= y_i and y_i > y_i1) {
      sig_stack.push_back(Sample{t, z_i1, 0.0});
    } else {
      // y_i > z_i1 > y_i1
      const auto ts = t + (z_i1 - it->value) / it->derivative;
      sig_stack.push_back(Sample{ts, z_i1, 0.0});
      sig_stack.push_back(Sample{t, y_i, 0.0});
    }
  }

  auto out = std::make_shared<Signal>(std::rbegin(sig_stack), std::rend(sig_stack));
  return out->simplify();
}

SignalPtr compute_eventually(const SignalPtr y, const double a) {
  auto z1 = mono_wedge_minmax(y, a, std::greater<Sample>());
  auto z2 = y->resize_shift(y->begin_time() + a, y->end_time() + a, BOTTOM, -a);
  auto z3 = compute_or(z2, z1);
  auto z  = compute_or(y, z3);
  return z->simplify();
}

SignalPtr compute_always(const SignalPtr y) {
  return compute_not(compute_eventually(compute_not(y)));
}

SignalPtr compute_always(const SignalPtr y, double a) {
  auto z1 = mono_wedge_minmax(y, a, std::less<Sample>());
  auto z2 = y->resize_shift(y->begin_time() + a, y->end_time() + a, TOP, -a);
  auto z3 = compute_and(z2, z1);
  auto z  = compute_and(y, z3);
  return z->simplify();
}

void segment_until(
    std::vector<signal::Sample>& out,
    const signal::Sample& i,
    double t,
    std::vector<signal::Sample>::const_reverse_iterator& j,
    double z_max) {
  double s = i.time;
  if (i.derivative <= 0) {
    auto z1 = std::vector<Sample>{};
    minmax::segment_minmax(z1, i, t, j, std::less<double>());

    auto z2 = std::vector<Sample>{};
    auto k  = z1.crbegin();
    minmax::partial_comp(z2, k, s, t, std::greater<double>());

    auto l = z2.crbegin();
    minmax::segment_minmax(
        out,
        Sample{s, std::min(z_max, i.interpolate(t)), 0},
        t,
        l,
        std::greater<double>());
  } else {
    auto z1 = std::vector<Sample>{};
    minmax::partial_comp(z1, j, s, t, std::greater<double>());

    auto z2 = std::vector<Sample>{};
    auto k  = z1.crbegin();
    minmax::segment_minmax(z2, i, t, k, std::less<double>());

    z1      = std::vector<Sample>{};
    auto z3 = std::vector<Sample>{};
    z3.push_back(Sample{s, z_max, 0});
    k = z3.crbegin();
    minmax::segment_minmax(z1, i, t, k, std::less<double>());

    k      = z1.crbegin();
    auto l = z2.crbegin();
    // TODO: partialOr doesnt exist...
    // minmax::segment_minmax(out, k, l, s, t, std::greater<double>());
  }
}

SignalPtr compute_until(const SignalPtr y1, const SignalPtr y2) {
  double begin_time = std::max(y1->begin_time(), y2->begin_time());
  double end_time   = std::min(y1->end_time(), y2->end_time());

  std::vector<Sample> sig_stack;
  sig_stack.reserve(4 * std::max(y1->size(), y2->size()));

  auto i = y1->rbegin();
  auto j = y2->rbegin();

  while (i->time >= end_time) i++;
  while (j->time >= end_time) j++;

  double s     = begin_time;
  double t     = end_time;
  double z_max = BOTTOM;
  for (; i->time > begin_time; (z_max = sig_stack.back().value), (t = i->time), i++) {
    segment_until(sig_stack, *i, t, j, z_max);
    if (j->time == i->time)
      j++;
  }
  if (i->time == begin_time) {
    segment_until(sig_stack, *i, t, j, z_max);
  } else {
    segment_until(sig_stack, Sample{s, i->interpolate(s), i->derivative}, t, j, z_max);
  }

  auto out = std::make_shared<Signal>(std::rbegin(sig_stack), std::rend(sig_stack));
  return out->simplify();
}

SignalPtr compute_until(
    const SignalPtr x,
    const SignalPtr y,
    std::pair<double, double> interval) {
  auto [a, b] = interval;

  auto z2 = compute_eventually(y, b - a);
  auto z3 = compute_until(x, y);
  auto z4 = compute_and(z2, z3);
  if (a > 0) {
    auto z1 = compute_always(x, a);
    return compute_and(z1, z4->shift(-a));
  }
  return compute_and(x, z4);
}

} // namespace

template <>
SignalPtr compute_robustness<Semantics::EFFICIENT>(
    const ast::Expr phi,
    const signal::Trace& trace) {
  SignalPtr out;

  return out;
}

} // namespace semantics
