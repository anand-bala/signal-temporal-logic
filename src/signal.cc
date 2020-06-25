#include "signal_tl/signal.hh"

#include "signal_tl/utils.hh"

#include <cmath>
#include <exception>
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

double Signal::begin_time() const {
  return (samples.empty()) ? 0.0 : samples.front().time;
}

double Signal::end_time() const {
  return (samples.empty()) ? 0.0 : samples.back().time;
}

double Signal::interpolate(double t, size_t idx) const {
  return this->samples.at(idx).interpolate(t);
}

double Signal::time_intersect(const Sample& point, size_t idx) const {
  return this->samples.at(idx).time_intersect(point);
}

double Signal::area(double t, size_t idx) const {
  return this->samples.at(idx).area(t);
}

Sample Signal::front() const {
  return this->samples.front();
}

Sample Signal::back() const {
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
