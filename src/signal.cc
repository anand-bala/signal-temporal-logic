#include "signal_tl/signal.hpp"
#include "signal_tl/utils.hpp"

#include "signal_tl/fmt.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>

#include <cassert>

namespace signal_tl::signal {

Sample Signal::at(double t) const {
  if (this->begin_time() > t && this->end_time() < t) {
    throw std::invalid_argument(
        fmt::format("Signal is undefined for given time instance {}", t));
  }
  constexpr auto comp_time = [](const Sample& a, const Sample& b) -> bool {
    return a.time < b.time;
  };

  auto it = std::lower_bound(
      this->begin(), this->end(), Sample{t, 0.0}, comp_time); // it->time >= t
  if (it->time == t) {
    return *it;
  }
  return Sample{t, it->interpolate(t), it->derivative};
}

void Signal::push_back(Sample sample) {
  if (!this->samples.empty()) {
    if (sample.time <= this->end_time()) {
      throw std::invalid_argument(fmt::format(
          "Trying to append a Sample timestamped at or before the Signal end_time,"
          "i.e., time is not strictly monotonically increasing."
          "Current end_time is {}, given Sample is {}.",
          this->end_time(),
          sample));
    }
    const auto t = this->samples.back().time;
    const auto v = this->samples.back().value;
    auto& last   = this->samples.back();

    last.derivative = (sample.value - v) / (sample.time - t);
  }
  this->samples.push_back(Sample{sample.time, sample.value, 0.0});
}

void Signal::push_back(double time, double value) {
  this->push_back(Sample{time, value, 0.0});
}

SignalPtr Signal::simplify() const {
  auto sig = std::make_shared<Signal>();
  for (const auto& s : this->samples) {
    const auto [t, v, d] = s;
    if ((sig->empty()) ||
        (sig->back().interpolate(t) != v || sig->back().derivative != d)) {
      sig->push_back(s);
    }
  }
  if (this->end_time() != sig->end_time()) {
    sig->push_back(this->back());
  }
  return sig;
}

SignalPtr Signal::resize(double start, double end, double fill) const {
  auto sig = std::make_shared<Signal>();

  // Check if begin_time > start, then add filled value
  if (this->begin_time() > start) {
    sig->push_back(Sample{start, fill, 0.0});
  }

  // Truncate the discard all samples where sample.time < start
  for (auto i = this->begin(); i != this->end(); i++) {
    const double t = i->time;
    // If current sample is timed below start, ...
    if (std::next(i) != this->end() && std::next(i)->time > start) {
      // and next sample is timed after `start`, append an intermediate value
      sig->push_back(Sample{start, i->interpolate(start)});
    } else if (start <= t && t <= end) {
      // If the samples are within the desired time range, keep the samples.
      sig->push_back(*i);
    } else if (t > end) {
      // If we are out of the range, ...
      if (i != this->begin() && std::prev(i)->time < end) {
        // and the previous sample is within the range, interpolate from the last.
        sig->push_back(Sample{end, std::prev(i)->interpolate(end)});
      } else {
        // TODO(anand): Does it make sense to terminate early?
        break;
      }
    }
  }

  return sig;
}

SignalPtr Signal::shift(double dt) const {
  auto sig = std::make_shared<Signal>(this->samples);
  for (auto& s : sig->samples) { s.time += dt; }

  return sig;
}

SignalPtr Signal::resize_shift(double start, double end, double fill, double dt) const {
  auto out = this->resize(start, end, fill);
  for (auto& s : out->samples) { s.time += dt; }
  return out;
}

std::tuple<std::shared_ptr<Signal>, std::shared_ptr<Signal>>
synchronize(const std::shared_ptr<Signal>& x, const std::shared_ptr<Signal>& y) {
  const double begin_time = std::max(x->begin_time(), y->begin_time());
  // const double end_time   = std::min(x->end_time(), y->end_time());

  // These will store the new series of Samples, containing a sample for every
  // time instance in x and y, and the time points where they intersect.
  auto xv = std::vector<Sample>{};
  auto yv = std::vector<Sample>{};

  // Iterator to the first element where element.time <= begin_time.
  // If the iterator begins after begin_time, get the prev (if it exists) and
  // interpolate from it.
  constexpr auto comp_time = [](const Sample& a, const Sample& b) -> bool {
    return a.time < b.time;
  };
  auto i = std::lower_bound(x->begin(), x->end(), Sample{begin_time, 0.0}, comp_time);
  if (i->time > begin_time && i != x->begin()) {
    xv.push_back(Sample{
        begin_time, std::prev(i)->interpolate(begin_time), std::prev(i)->derivative});
  }
  auto j = std::lower_bound(y->begin(), y->end(), Sample{begin_time, 0.0}, comp_time);
  if (j->time > begin_time && j != y->begin()) {
    yv.push_back(Sample{
        begin_time, std::prev(j)->interpolate(begin_time), std::prev(i)->derivative});
  }

  // Now, we have to track the timestamps.
  while (i != x->end() && j != y->end()) {
    if (i->time == j->time) {
      // The samples remain in both.
      xv.push_back(*i);
      yv.push_back(*j);
      i++;
      j++;
    } else if (i->time < j->time) {
      // Add the current point
      xv.push_back(*i);
      yv.push_back(Sample{i->time, std::prev(j)->interpolate(i->time)});
      // TODO: Intercept?
      // We need to "catch up".
      i++;
    } else if (j->time < i->time) {
      yv.push_back(*j);
      xv.push_back(Sample{j->time, std::prev(i)->interpolate(j->time)});
      j++;
    }
  }

  if (const double t = xv.back().time; yv.back().time < t) {
    yv.push_back(Sample{xv.back().time, yv.back().interpolate(t)});
  }

  if (const double t = yv.back().time; xv.back().time < t) {
    xv.push_back(Sample{yv.back().time, xv.back().interpolate(t)});
  }

  return std::make_tuple(std::make_shared<Signal>(xv), std::make_shared<Signal>(yv));
}

} // namespace signal_tl::signal
