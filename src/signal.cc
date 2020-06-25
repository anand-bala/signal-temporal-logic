#include "signal_tl/signal.hh"

#include "minmax.hh"
#include "utils.hh"

#include <algorithm>
#include <cmath>
#include <exception>
#include <execution>
#include <functional>
#include <iterator>
#include <numeric>

namespace signal {

inline double Sample::interpolate(double t) const {
  return value + derivative * (t - time);
}

inline double Sample::time_intersect(const Sample& point) const {
  return (value - point.value + (point.derivative * point.time) - (derivative * time)) /
         (point.derivative - derivative);
}

inline double Sample::area(double t) const {
  if (t > time) {
    return (value + this->interpolate(t)) * (t - time) / 2;
  } else {
    return 0;
  }
}

Signal::Signal(const std::vector<Sample>& data) {
  this->samples.reserve(data.size());

  for (const auto s : data) { this->push_back(s); }
}

Signal::Signal(const Signal& other) {
  this->samples.reserve(other.size());
  for (const auto s : other) { this->push_back(s); }
}

Signal::Signal(const std::vector<double>& points, const std::vector<double>& times) {
  if (points.size() != times.size()) {
    throw std::invalid_argument(
        "Number of sample points and time points need to be equal.");
  }

  size_t n = points.size();
  this->samples.reserve(n);
  for (size_t i = 0; i < n; i++) {
    this->push_back(Sample{times.at(i), points.at(i), 0.0});
  }
}

template <typename Iter>
Signal::Signal(const Iter start, const Iter end) {
  typename std::iterator_traits<Iter>::difference_type n = std::distance(start, end);
  this->samples.reserve(n);
  for (auto i = start; i != end; i = std::next(i)) { this->push_back(*i); }
}

inline double Signal::begin_time() const {
  return (samples.empty()) ? 0.0 : samples.front().time;
}

inline double Signal::end_time() const {
  return (samples.empty()) ? 0.0 : samples.back().time;
}

inline double Signal::interpolate(double t, size_t idx) const {
  return this->samples.at(idx).interpolate(t);
}

inline double Signal::time_intersect(const Sample& point, size_t idx) const {
  return this->samples.at(idx).time_intersect(point);
}

inline double Signal::area(double t, size_t idx) const {
  return this->samples.at(idx).area(t);
}

inline Sample Signal::front() const {
  return this->samples.front();
}

inline Sample Signal::back() const {
  return this->samples.back();
}

void Signal::push_back(Sample sample) {
  if (!this->samples.empty()) {
    if (sample.time < this->end_time()) {
      throw std::invalid_argument(
          "Trying to append a Sample timestamped before the Signal end_time, i.e., time is not strictly monotonically increasing");
    }
    const auto [t, v, d] = this->samples.back();
    auto last            = this->samples.back();

    last.derivative = (sample.value - v) / (sample.time - t);
  }
  this->samples.push_back(Sample{sample.time, sample.value, 0.0});
}

void Signal::push_back(double time, double value) {
  this->push_back(Sample{time, value, 0.0});
}

void Signal::push_back_raw(Sample sample) {
  if (!this->samples.empty()) {
    if (sample.time < this->end_time()) {
      throw std::invalid_argument(
          "Trying to append a Sample timestamped before the Signal end_time, i.e., time is not strictly monotonically increasing");
    }
    const auto [t, v, d] = this->samples.back();
    auto last            = this->samples.back();

    last.derivative = (sample.value - v) / (sample.time - t);
  }
  this->samples.push_back(sample);
}

SignalPtr Signal::simplify() const {
  auto sig = std::make_shared<Signal>();
  for (const auto& s : this->samples) {
    const auto [t, v, d] = s;
    if (sig->empty()) {
      sig->push_back(s);
    } else if (const auto& new_s = sig->back();
               new_s.interpolate(t) != v or new_s.derivative != d) {
      sig->push_back(s);
    }
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
  for (const auto&& [current, s] : utils::enumerate(this->samples)) {
    const auto [t, v, d] = s;
    if (t < start) {
      // If current sample is timed below start, ...
      if (current + 1 < this->samples.size() and
          this->samples.at(current + 1).time > start) {
        // and next sample is timed after `start`, append an intermediate value
        sig->push_back(Sample{start, s.interpolate(start), 0.0});
      }
      // Else we forget about the sample.
    } else if (start <= t and t <= end) {
      // If the samples are within the desired time range, keep the samples.
      sig->push_back(s);
    } else if (t > end) {
      // If we are out of the range, ...
      if (current - 1 >= 0 and this->samples.at(current - 1).time < end) {
        // and the previous sample is within the range, interpolate from the last.
        sig->push_back(Sample{end, this->interpolate(end, current - 1), 0.0});
      } else {
        // TODO Does it make sense to terminate early?
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
} // namespace signal

template <typename Compare>
SignalPtr minmax(const SignalPtr y1, const SignalPtr y2, Compare comp) {
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

  auto out = std::make_shared<Signal>(std::rbegin(sig_stack), std::rend(sig_stack));
  return out->simplify();
}

template <typename Compare>
SignalPtr minmax(const std::vector<SignalPtr>& ys, Compare comp) {
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
      [&comp](const SignalPtr a, const SignalPtr b) { return minmax(a, b, comp); });
  return out;
}

SignalPtr min(const SignalPtr y1, const SignalPtr y2) {
  return minmax(y1, y2, std::less<double>());
}

SignalPtr min(const std::vector<SignalPtr>& ys) {
  return minmax(ys, std::less<double>());
}

SignalPtr max(const SignalPtr y1, const SignalPtr y2) {
  return minmax(y1, y2, std::greater<double>());
}
SignalPtr max(const std::vector<SignalPtr>& ys) {
  return minmax(ys, std::greater<double>());
}

std::ostream& operator<<(std::ostream& out, const signal::Sample& sample) {
  return out << "{" << sample.time << ";" << sample.value << ";" << sample.derivative
             << "}";
}

std::ostream& operator<<(std::ostream& os, const signal::Signal& sig) {
  if (sig.samples.empty()) {
    return os << "(0,0)[]";
  }
  os << "[" << sig.begin_time() << "," << sig.end_time() << "]";
  os << "[";
  for (const auto& s : sig.samples) { os << s; }
  os << "]";
  return os;
}

} // namespace signal
