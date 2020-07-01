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

#include "signal_tl/utils.hh"

#include <algorithm>
#include <cmath>
#include <deque>
#include <execution>
#include <functional>
#include <limits>

#include <cassert>

namespace semantics {
using namespace signal;

namespace {

constexpr double TOP    = std::numeric_limits<double>::infinity();
constexpr double BOTTOM = -TOP;

namespace minmax {

template <typename Compare>
void segment_minmax(
    std::vector<Sample>& out,
    const Sample& i,
    double t,
    std::vector<Sample>::const_reverse_iterator& j,
    Compare op) {
  // PRECONDITIONS: j->time < t, i.time < t
  // POSTCONDITIONS: j->time <= i.time
  bool continued = false;
  double s       = j->time;

  // for every sample *j in (i.time, t)
  for (; s > i.time; (t = s), j++, (s = j->time)) {
    if (op(i.interpolate(t), j->interpolate(t))) {
      if (op(j->value, i.interpolate(s))) {
        t = i.time_intersect(*j);
        out.push_back({t, i.interpolate(t), i.derivative});
        out.push_back({s, j->value, j->derivative});
        continued = false;
      } else
        continued = true;
    } else if (i.interpolate(t) == j->interpolate(t)) {
      if (op(j->value, i.interpolate(s))) {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
          continued = false;
        }
        out.push_back({s, j->value, j->derivative});
      } else
        continued = true;
    } else {
      if (op(i.interpolate(s), j->value)) {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
        }
        t = i.time_intersect(*j);

        out.push_back({t, j->interpolate(t), j->derivative});
        continued = true;
      } else {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
          continued = false;
        }
        out.push_back({s, j->value, j->derivative});
      }
    }
  }

  // here we may have j->time < i.time
  // "i" values of z are no longer "continued"
  s = i.time;
  if (op(i.interpolate(t), j->interpolate(t))) {
    if (op(j->interpolate(s), i.value)) {
      t = i.time_intersect(*j);
      out.push_back({t, i.interpolate(t), i.derivative});
      out.push_back({s, j->interpolate(s), j->derivative});
    } else {
      out.push_back(i);
    }
  } else if (i.interpolate(t) == j->interpolate(t)) {
    if (op(j->interpolate(s), i.value)) {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      out.push_back({s, j->interpolate(s), j->derivative});
    } else {
      out.push_back(i);
    }
  } else {
    if (op(i.value, j->interpolate(s))) {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      t = i.time_intersect(*j);
      out.push_back({t, j->interpolate(t), j->derivative});
      out.push_back(i);

    } else {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      out.push_back({s, j->interpolate(s), j->derivative});
    }
  }
}

template <typename Compare>
void partial_comp(
    std::vector<signal::Sample>& out,
    std::vector<signal::Sample>::const_reverse_iterator& i,
    double start_time,
    double end_time,
    Compare op) {
  bool continued = false;
  double z_max   = BOTTOM;
  double s = start_time, t = end_time;
  while (i->time > s) {
    if (i->derivative >= 0) {
      if (z_max < i->interpolate(t)) {
        if (continued) {
          out.push_back(Sample{t, z_max, 0});
        }
        z_max = i->interpolate(t);
      }
      continued = true;
      // out.push_back(Sample(i->time, z_max, 0));
    } else if (i->interpolate(t) >= z_max) {
      if (continued) {
        out.push_back(Sample{t, z_max, 0});
        continued = false;
      }
      z_max = i->value;
      out.push_back(*i);
    } else if (z_max >= i->value) {
      continued = true;
      // out.push_back(Sample(i->time, z_max, 0));
    } else {
      out.push_back(Sample{i->time + (z_max - i->value) / i->derivative,
                           z_max,
                           0}); // time at which y reaches value next_z
      out.push_back(*i);
      z_max     = i->value;
      continued = false;
    }

    t = i->time;
    i++;
  }

  // leftmost sample *i may not be on s
  //"z_max" values of z are not longer "continued".
  if (i->derivative >= 0) {
    if (z_max < i->interpolate(t)) {
      if (continued) {
        out.push_back(Sample{t, z_max, 0});
      }
      z_max = i->interpolate(t);
    }
    out.push_back(Sample{s, z_max, 0});
  } else if (i->interpolate(t) >= z_max) {
    if (continued) {
      out.push_back(Sample{t, z_max, 0});
    }
    out.push_back(Sample{s, i->interpolate(s), i->derivative});
  } else if (z_max >= i->interpolate(s)) {
    out.push_back(Sample{s, z_max, 0});
  } else {
    out.push_back(Sample{s + (z_max - i->value) / i->derivative,
                         z_max,
                         0}); // time at which y reaches value next_z
    out.push_back(Sample{s, i->interpolate(s), i->derivative});
  }
}

} // namespace minmax

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

template <typename Compare>
SignalPtr signal_minmax(const SignalPtr y1, const SignalPtr y2, Compare comp) {
  // The output signal is only defined in the duration when y1 and y2 are defined.
  double begin_time = std::max(y1->begin_time(), y2->begin_time());
  double end_time   = std::min(y1->end_time(), y2->end_time());

  // We are going to build the signal in reverse as a vector of samples and then use
  // that to build the output signal. Essentially, this is equivalent to pushing the new
  // Samples in reverse onto a stack and then popping it.

  std::vector<Sample> sig_stack;
  // Since the new signal is going to have <= 4 * max(y1.size(), y2.size()) samples,
  // we reserve that space for speed.
  sig_stack.reserve(4 * std::max(y1->size(), y2->size()));

  // Get reverse_iterators for y1, y2
  auto i = y1->rbegin();
  auto j = y2->rbegin();

  // Advance the iterators up to end_time
  while (i->time >= end_time) i++;
  while (j->time >= end_time) j++;

  // Compute for segments
  double t = end_time;
  for (; i->time > begin_time; (t = i->time), i++) {
    minmax::segment_minmax(sig_stack, *i, t, j, comp);
    if (j->time == i->time)
      j++;
  }
  if (i->time == begin_time) {
    minmax::segment_minmax(sig_stack, *i, t, j, comp);
  } else {
    minmax::segment_minmax(
        sig_stack, {begin_time, i->interpolate(begin_time), i->derivative}, t, j, comp);
  }

  std::reverse(std::begin(sig_stack), std::end(sig_stack));

  auto out = std::make_shared<Signal>(sig_stack);
  return out->simplify();
}

template <typename Compare>
SignalPtr signal_minmax(const std::vector<SignalPtr>& ys, Compare comp) {
  if (ys.empty()) {
    auto out = std::make_shared<Signal>();
    out->push_back(0, -std::numeric_limits<double>::infinity());
    return out;
  } else if (ys.size() == 1) {
    return ys.at(0);
  }

  // TODO(anand): Parallel execution policy?
  SignalPtr out = std::reduce(
      std::execution::seq,
      std::next(ys.cbegin()),
      ys.cend(),
      ys.at(0),
      [&comp](const SignalPtr a, const SignalPtr b) {
        return signal_minmax(a, b, comp);
      });
  return out;
}

SignalPtr compute_not(const SignalPtr y) {
  auto out = std::make_shared<Signal>();
  for (const auto [t, v, d] : *y) { out->push_back_raw(Sample{t, -v, -d}); }
  return out;
}

SignalPtr compute_and(const SignalPtr y1, const SignalPtr y2) {
  return signal_minmax(y1, y2, std::less<double>());
}

SignalPtr compute_and(const std::vector<SignalPtr>& ys) {
  return signal_minmax(ys, std::less<double>());
}

SignalPtr compute_or(const SignalPtr y1, const SignalPtr y2) {
  return signal_minmax(y1, y2, std::greater<double>());
}

SignalPtr compute_or(const std::vector<SignalPtr>& ys) {
  return signal_minmax(ys, std::greater<double>());
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

  std::reverse(std::begin(sig_stack), std::end(sig_stack));
  auto out = std::make_shared<Signal>(sig_stack);
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

  std::reverse(std::begin(sig_stack), std::end(sig_stack));
  auto out = std::make_shared<Signal>(sig_stack);
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

struct RobustnessOp {
  const double min_time;
  const double max_time;
  const Trace& trace;
  SignalPtr operator()(const ast::ConstPtr e);
  SignalPtr operator()(const ast::PredicatePtr e);
  SignalPtr operator()(const ast::NotPtr e);
  SignalPtr operator()(const ast::AndPtr e);
  SignalPtr operator()(const ast::OrPtr e);
  SignalPtr operator()(const ast::EventuallyPtr e);
  SignalPtr operator()(const ast::AlwaysPtr e);
  SignalPtr operator()(const ast::UntilPtr e);
};

} // namespace

template <>
SignalPtr compute_robustness<Semantics::EFFICIENT>(
    const ast::Expr phi,
    const signal::Trace& trace,
    bool synchronized) {
  struct MinMaxTime {
    double begin{TOP};
    double end{BOTTOM};
    void operator()(const std::pair<std::string, SignalPtr>& entry) {
      auto s = entry.second;
      begin  = std::min(begin, s->begin_time());
      end    = std::max(end, s->end_time());
    }
  };

  const MinMaxTime minmaxtime =
      std::for_each(trace.cbegin(), trace.cend(), MinMaxTime{});
  double min_time = minmaxtime.begin;
  double max_time = minmaxtime.end;

  SignalPtr out = std::visit(RobustnessOp{min_time, max_time, trace}, phi);

  return out;
}

SignalPtr RobustnessOp::operator()(const ast::ConstPtr e) {
  double val   = (e->value) ? TOP : BOTTOM;
  auto samples = std::vector<Sample>{{min_time, val, 0.0}, {max_time, val, 0.0}};
  return std::make_shared<Signal>(samples);
}

SignalPtr RobustnessOp::operator()(const ast::PredicatePtr e) {
  return trace.at(e->name);
}

SignalPtr RobustnessOp::operator()(const ast::NotPtr e) {
  auto y = compute_robustness<Semantics::EFFICIENT>(e->arg, trace);
  return compute_not(y);
}

SignalPtr RobustnessOp::operator()(const ast::AndPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::EFFICIENT>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_and(ys);
}

SignalPtr RobustnessOp::operator()(const ast::OrPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::EFFICIENT>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_or(ys);
}

SignalPtr RobustnessOp::operator()(const ast::EventuallyPtr e) {
  auto y = compute_robustness<Semantics::EFFICIENT>(e->arg, trace);
  if (!e->interval.has_value()) {
    return compute_eventually(y);
  }

  const auto [a, b] = *(e->interval);
  if (b - a < 0) {
    throw std::logic_error("Eventually operator: b < a in interval [a,b]");
  } else if (b - a == 0) {
    return y;
  } else if (b - a >= y->end_time() - y->begin_time()) {
    return compute_eventually(y);
  } else {
    return compute_eventually(y, b - a);
  }
}

SignalPtr RobustnessOp::operator()(const ast::AlwaysPtr e) {
  auto y = compute_robustness<Semantics::EFFICIENT>(e->arg, trace);
  if (!e->interval.has_value()) {
    return compute_always(y);
  }

  const auto [a, b] = *(e->interval);
  if (b - a < 0) {
    throw std::logic_error("Always operator: b < a in interval [a,b]");
  } else if (b - a == 0) {
    return y;
  } else if (b - a >= y->end_time() - y->begin_time()) {
    return compute_always(y);
  } else {
    return compute_always(y, b - a);
  }
}

SignalPtr RobustnessOp::operator()(const ast::UntilPtr e) {
  auto y1 = compute_robustness<Semantics::EFFICIENT>(e->args.first, trace);
  auto y2 = compute_robustness<Semantics::EFFICIENT>(e->args.second, trace);
  if (!e->interval.has_value()) {
    return compute_until(y1, y2);
  }

  const auto [a, b] = *(e->interval);
  if (std::isinf(b)) {
    if (a == 0) {
      return compute_until(y1, y2);
    } else {
      auto yalw1  = compute_always(y1, a);
      auto yuntmp = compute_until(y1, y2)->shift(-a);
      return compute_and(yalw1, yuntmp);
    }
  } else {
    return compute_until(y1, y2, *(e->interval));
  }
}

} // namespace semantics
