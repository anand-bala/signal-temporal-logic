#include "signal_tl/minmax.hpp"
#include "signal_tl/mono_wedge.h" // for mono_wedge_update

#include <algorithm>  // for max, reverse
#include <deque>      // for _Deque_iterator, deque, operator-
#include <functional> // for greater_equal, less_equal
#include <iterator>   // for prev, next, begin
#include <limits>     // for numeric_limits
#include <memory>     // for __shared_ptr_access, make_shared
#include <numeric>    // for accumulate
#include <tuple>      // for make_tuple, tuple_element<>::type
#include <utility>    // for tuple_element<>::type

#include <cassert> // for assert

namespace signal_tl::minmax {
using namespace signal;

template <typename Compare>
SignalPtr compute_minmax_pair(
    const SignalPtr& input_x,
    const SignalPtr& input_y,
    Compare comp,
    bool synchronized) {
  const auto [x, y] = (synchronized) ? std::make_tuple(input_x, input_y)
                                     : synchronize(input_x, input_y);
  assert(x->size() == y->size());
  assert(x->begin_time() == y->begin_time());
  assert(x->end_time() == y->end_time());

  // Used to keep track of the signal from which the last minmax winner was chosen.
  enum struct Chosen { X, Y, NONE };
  Chosen last_chosen = Chosen::NONE;

  auto out = std::make_shared<Signal>();

  for (auto [i, j] = std::make_tuple(x->begin(), y->begin());
       i != x->end() && j != y->end();
       i++, j++) {
    if (comp(*i, *j)) {
      if (last_chosen == Chosen::Y) {
        double intercept_time = std::prev(j)->time_intersect(*std::prev(i));
        if (intercept_time > out->end_time() && intercept_time != i->time) {
          out->push_back(
              Sample{intercept_time, std::prev(j)->interpolate(intercept_time)});
        }
      }
      out->push_back(*i);
      last_chosen = Chosen::X;
    } else {
      if (last_chosen == Chosen::X) {
        double intercept_time = std::prev(i)->time_intersect(*std::prev(j));
        if (intercept_time > out->end_time() && intercept_time != j->time) {
          out->push_back(
              Sample{intercept_time, std::prev(i)->interpolate(intercept_time)});
        }
      }
      out->push_back(*j);
      last_chosen = Chosen::Y;
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
  } else if (xs.size() == 2) {
    return compute_minmax_pair(xs[0], xs[1], comp, synchronized);
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
SignalPtr compute_minmax_seq(const SignalPtr& x, Compare comp) {
  auto opt = x->back();
  auto z   = std::vector<Sample>{};
  z.reserve(2 * x->size());
  z.push_back(x->back());

  for (auto i = std::next(x->rbegin()); i != x->rend(); i++) {
    opt = (comp(*i, opt)) ? *i : opt;
    z.push_back({i->time, opt.value});
  }

  std::reverse(z.begin(), z.end());
  return std::make_shared<Signal>(z);
}

template <typename Compare>
SignalPtr compute_minmax_seq(const SignalPtr& x, double a, double b, Compare comp) {
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
          Sample{
              window.front().time + a,
              std::prev(i)->interpolate(window.front().time + a),
              0.0});
    }
    mono_wedge::mono_wedge_update(window, *i, comp);
    while (window.front().time <= i->time - a) { window.pop_front(); }
    z->push_back(i->time, i->value);
  }

  return z->simplify();
}

SignalPtr
compute_elementwise_min(const SignalPtr& x, const SignalPtr& y, bool synchronized) {
  return compute_minmax_pair(x, y, std::less_equal<>(), synchronized);
}

SignalPtr
compute_elementwise_max(const SignalPtr& x, const SignalPtr& y, bool synchronized) {
  return compute_minmax_pair(x, y, std::greater_equal<>(), synchronized);
}

SignalPtr compute_elementwise_min(const std::vector<SignalPtr>& xs, bool synchronized) {
  return compute_minmax_pair(xs, std::less_equal<>(), synchronized);
}

SignalPtr compute_elementwise_max(const std::vector<SignalPtr>& xs, bool synchronized) {
  return compute_minmax_pair(xs, std::greater_equal<>(), synchronized);
}

SignalPtr compute_max_seq(const SignalPtr& x) {
  return compute_minmax_seq(x, std::greater_equal<>());
}

SignalPtr compute_min_seq(const SignalPtr& x) {
  return compute_minmax_seq(x, std::less_equal<>());
}

SignalPtr compute_max_seq(const SignalPtr& x, double a, double b) {
  return compute_minmax_seq(x, a, b, std::greater_equal<>());
}

SignalPtr compute_min_seq(const SignalPtr& x, double a, double b) {
  return compute_minmax_seq(x, a, b, std::less_equal<>());
}

} // namespace signal_tl::minmax
