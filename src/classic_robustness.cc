#include "signal_tl/exception.hh"
#include "signal_tl/robustness.hh"
#include "signal_tl/utils.hh"

#include "mono_wedge/mono_wedge.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numeric>

#include <deque>

#include <cassert>

namespace signal_tl {
namespace semantics {
using namespace signal;

namespace {
constexpr double TOP    = std::numeric_limits<double>::infinity();
constexpr double BOTTOM = -TOP;

SignalPtr compute_not(const SignalPtr x) {
  auto vec = std::vector<Sample>{};
  vec.reserve(x->size());
  std::transform(x->begin(), x->end(), vec.begin(), std::negate<Sample>());
  return std::make_shared<Signal>(vec);
}

template <typename Compare>
SignalPtr compute_minmax_pair(
    const SignalPtr input_x,
    const SignalPtr input_y,
    Compare comp,
    bool synchronized) {
  const auto [x, y] = (synchronized) ? std::make_tuple(input_x, input_y)
                                     : synchronize(input_x, input_y);
  assert(x->size() == y->size());
  assert(x->begin_time() == y->begin_time());
  assert(x->end_time() == y->end_time());

  const size_t n = x->size();

  // Used to keep track of the signal from which the last minmax winner was chosen.
  enum Chosen { X, Y, NONE };
  Chosen last_chosen = NONE;

  auto out = std::make_shared<Signal>();

  for (auto [i, j] = std::make_tuple(x->begin(), y->begin());
       i != x->end() and j != y->end();
       i++, j++) {
    if (comp(i->value, j->value)) {
      if (last_chosen == Y) {
        double intercept_time = std::prev(j)->time_intersect(*std::prev(i));
        out->push_back(
            Sample{intercept_time, std::prev(j)->interpolate(intercept_time)});
      }
      out->push_back(*i);
      last_chosen = X;
    } else {
      if (last_chosen == X) {
        double intercept_time = std::prev(i)->time_intersect(*std::prev(j));
        out->push_back(
            Sample{intercept_time, std::prev(i)->interpolate(intercept_time)});
      }
      out->push_back(*j);
      last_chosen = Y;
    }
  }

  return out;
}

template <typename Compare>
SignalPtr
compute_minmax_pair(const std::vector<SignalPtr>& xs, Compare comp, bool synchronized) {
  if (xs.empty()) {
    auto out = std::make_shared<Signal>();
    out->push_back(0, -std::numeric_limits<double>::infinity());
    return out;
  } else if (xs.size() == 1) {
    return xs.at(0);
  }

  // TODO(anand): Parallel execution policy?
  SignalPtr out = std::accumulate(
      std::next(xs.cbegin()),
      xs.cend(),
      xs.at(0),
      [&comp, &synchronized](const SignalPtr a, const SignalPtr b) {
        return compute_minmax_pair(a, b, comp, synchronized);
      });
  return out;
}

template <typename Compare>
SignalPtr compute_minmax_seq(const SignalPtr x, Compare comp) {
  double opt = x->back().value;
  auto z     = std::vector<Sample>{};
  z.reserve(2 * x->size());
  z.push_back(x->back());

  for (auto i = std::next(x->rbegin()); i != x->rend(); i++) {
    opt = (comp(i->value, opt)) ? i->value : opt;
    z.push_back({i->time, opt});
  }

  std::reverse(z.begin(), z.end());
  return std::make_shared<Signal>(z);
}

template <typename Compare>
SignalPtr compute_minmax_seq(const SignalPtr x, double a, double b, Compare comp) {
  const auto width      = b - a;
  const auto begin_time = x->begin_time();
  const auto end_time   = x->end_time();

  auto x_ = (a == 0)
                ? x
                : x->resize_shift(
                      begin_time + width, end_time + width, x->back().value, -width);

  auto z       = std::make_shared<Signal>();
  auto samples = std::deque<Sample>{x_->begin(), x_->end()};
  auto window  = std::deque<Sample>{};

  auto i = std::begin(samples);
  // -- Read in values in [0, width)
  for (; i->time < x->begin_time() + width; i++) {
    mono_wedge::mono_wedge_update(window, *i, comp);
  }

  // -- Stream in the rest of the signal.
  for (; i != samples.end(); i++) {
    if (i->time - a > window.front().time) {
      // Add the sample for time M.front + a
      i = samples.insert(
          i,
          Sample{window.front().time + a,
                 std::prev(i)->interpolate(window.front().time + a),
                 0.0});
    }
    mono_wedge::mono_wedge_update(window, *i, comp);
    while (window.front().time <= i->time - a) { window.pop_front(); }
    z->push_back(i->time, i->value);
  }

  return z->simplify();
}

SignalPtr compute_and(const SignalPtr x, const SignalPtr y, bool synchronized) {
  return compute_minmax_pair(x, y, std::less_equal<double>(), synchronized);
}

SignalPtr compute_and(const std::vector<SignalPtr>& xs, bool synchronized) {
  return compute_minmax_pair(xs, std::less_equal<double>(), synchronized);
}

SignalPtr compute_or(const SignalPtr x, const SignalPtr y, bool synchronized) {
  return compute_minmax_pair(x, y, std::greater_equal<double>(), synchronized);
}

SignalPtr compute_or(const std::vector<SignalPtr>& xs, bool synchronized) {
  return compute_minmax_pair(xs, std::greater_equal<double>(), synchronized);
}

SignalPtr compute_eventually(const SignalPtr x) {
  return compute_minmax_seq(x, std::greater_equal<double>());
}

SignalPtr compute_eventually(const SignalPtr x, double a, double b) {
  return compute_minmax_seq(x, a, b, std::greater_equal<Sample>());
}

SignalPtr compute_always(const SignalPtr x) {
  return compute_minmax_seq(x, std::less_equal<double>());
}

SignalPtr compute_always(const SignalPtr x, double a, double b) {
  return compute_minmax_seq(x, a, b, std::less_equal<Sample>());
}

SignalPtr
compute_until(const SignalPtr input_x, const SignalPtr input_y, bool synchronized) {
  const auto [x, y] = (synchronized) ? std::make_tuple(input_x, input_y)
                                     : synchronize(input_x, input_y);
  assert(x->size() == y->size());
  assert(x->begin_time() == y->begin_time());
  assert(x->end_time() == y->end_time());

  auto sigstack = std::vector<Sample>();

  // TODO(anand): This doesn't handle crossing signals well...

  double prev      = TOP;
  double max_right = BOTTOM;

  for (auto [i, j] = std::make_tuple(x->rbegin(), y->rbegin());
       i != x->rend() and j != y->rend();
       i++, j++) {
    max_right = std::max(max_right, j->value);
    prev      = std::max({j->value, std::min(i->value, prev), -max_right});
    sigstack.push_back({i->time, prev});
  }
  std::reverse(sigstack.begin(), sigstack.end());
  auto out = std::make_shared<Signal>(sigstack);
  return out;
}

SignalPtr compute_until(
    const SignalPtr x,
    const SignalPtr y,
    double a,
    double b,
    bool synchronized) {
  throw not_implemented_error("Bounded compute_until has not been implemented yet.");
}

struct RobustnessOp {
  const double min_time;
  const double max_time;
  const Trace& trace;
  const bool synchronized;
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
SignalPtr compute_robustness<Semantics::CLASSIC>(
    const ast::Expr phi,
    const signal::Trace& trace,
    bool synchronized) {
  // Compute the start and end of the trace.
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

  SignalPtr out =
      std::visit(RobustnessOp{min_time, max_time, trace, synchronized}, phi);

  return out;
}

SignalPtr RobustnessOp::operator()(const ast::ConstPtr e) {
  double val   = (e->value) ? TOP : BOTTOM;
  auto samples = std::vector<Sample>{{min_time, val, 0.0}, {max_time, val, 0.0}};
  return std::make_shared<Signal>(samples);
}

SignalPtr RobustnessOp::operator()(const ast::PredicatePtr e) {
  const auto& x = trace.at(e->name);
  auto y        = std::make_shared<Signal>();
  for (const auto [t, v, d] : *x) {
    switch (e->op) {
      case ast::ComparisonOp::GE:
        y->push_back(t, v - e->lhs);
        break;
      case ast::ComparisonOp::GT:
        y->push_back(t, v - e->lhs);
        break;
      case ast::ComparisonOp::LE:
        y->push_back(t, e->lhs - v);
        break;
      case ast::ComparisonOp::LT:
        y->push_back(t, e->lhs - v);
        break;
    }
  }
  return y;
}

SignalPtr RobustnessOp::operator()(const ast::NotPtr e) {
  auto y = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
  return compute_not(y);
}

SignalPtr RobustnessOp::operator()(const ast::AndPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::CLASSIC>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_and(ys, this->synchronized);
}

SignalPtr RobustnessOp::operator()(const ast::OrPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::CLASSIC>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_or(ys, this->synchronized);
}

SignalPtr RobustnessOp::operator()(const ast::EventuallyPtr e) {
  auto y = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
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
    return compute_eventually(y, a, b);
  }
}

SignalPtr RobustnessOp::operator()(const ast::AlwaysPtr e) {
  auto y = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
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
    return compute_always(y, a, b);
  }
}

SignalPtr RobustnessOp::operator()(const ast::UntilPtr e) {
  auto y1 = compute_robustness<Semantics::CLASSIC>(e->args.first, trace);
  auto y2 = compute_robustness<Semantics::CLASSIC>(e->args.second, trace);
  if (!e->interval.has_value()) {
    return compute_until(y1, y2, this->synchronized);
  }

  const auto [a, b] = *(e->interval);
  if (std::isinf(b) and a == 0) {
    return compute_until(y1, y2, this->synchronized);
  } else {
    return compute_until(y1, y2, a, b, this->synchronized);
  }
}

} // namespace semantics
} // namespace signal_tl
