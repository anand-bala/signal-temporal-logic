#include "signal_tl/robustness.hh"

#include "signal_tl/exception.hh"
#include "signal_tl/minmax.hh"

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
using namespace minmax;

namespace {
constexpr double TOP    = std::numeric_limits<double>::infinity();
constexpr double BOTTOM = -TOP;

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
       i != x->rend() && j != y->rend();
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
  auto x   = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
  auto vec = std::vector<Sample>{};
  vec.reserve(x->size());
  std::transform(x->begin(), x->end(), std::back_inserter(vec), std::negate<Sample>());
  return std::make_shared<Signal>(vec);
}

SignalPtr RobustnessOp::operator()(const ast::AndPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::CLASSIC>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_elementwise_min(ys, this->synchronized);
}

SignalPtr RobustnessOp::operator()(const ast::OrPtr e) {
  auto ys = std::vector<SignalPtr>{};
  ys.reserve(e->args.size());
  std::transform(
      e->args.begin(), e->args.end(), std::back_inserter(ys), [this](const auto arg) {
        return compute_robustness<Semantics::CLASSIC>(arg, this->trace);
      });
  assert(ys.size() == e->args.size());
  return compute_elementwise_max(ys, this->synchronized);
}

SignalPtr RobustnessOp::operator()(const ast::EventuallyPtr e) {
  auto y = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
  if (!e->interval.has_value()) {
    return compute_max_seq(y);
  }

  const auto [a, b] = *(e->interval);
  if (b - a < 0) {
    throw std::logic_error("Eventually operator: b < a in interval [a,b]");
  } else if (b - a == 0) {
    return y;
  } else if (b - a >= y->end_time() - y->begin_time()) {
    return compute_max_seq(y);
  } else {
    return compute_max_seq(y, a, b);
  }
}

SignalPtr RobustnessOp::operator()(const ast::AlwaysPtr e) {
  auto y = compute_robustness<Semantics::CLASSIC>(e->arg, trace);
  if (!e->interval.has_value()) {
    return compute_min_seq(y);
  }

  const auto [a, b] = *(e->interval);
  if (b - a < 0) {
    throw std::logic_error("Always operator: b < a in interval [a,b]");
  } else if (b - a == 0) {
    return y;
  } else if (b - a >= y->end_time() - y->begin_time()) {
    return compute_min_seq(y);
  } else {
    return compute_min_seq(y, a, b);
  }
}

SignalPtr RobustnessOp::operator()(const ast::UntilPtr e) {
  auto y1 = compute_robustness<Semantics::CLASSIC>(e->args.first, trace);
  auto y2 = compute_robustness<Semantics::CLASSIC>(e->args.second, trace);
  if (!e->interval.has_value()) {
    return compute_until(y1, y2, this->synchronized);
  }

  const auto [a, b] = *(e->interval);
  if (std::isinf(b) && a == 0) {
    return compute_until(y1, y2, this->synchronized);
  } else {
    return compute_until(y1, y2, a, b, this->synchronized);
  }
}

} // namespace semantics
} // namespace signal_tl
