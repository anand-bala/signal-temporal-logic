#include "signal_tl/signal.hh"
#include "utils.hh"

#include <algorithm>
#include <cmath>
#include <deque>
#include <exception>
#include <iterator>
#include <limits>

namespace signal {

inline float Sample::interpolate(float t) const {
  return value + derivative * (t - time);
}

inline float Sample::time_intersect(const Sample& point) const {
  return (value - point.value + (point.derivative * point.time) - (derivative * time)) /
         (point.derivative - derivative);
}

inline float Sample::area(float t) const {
  if (t > time) {
    return (value + this->interpolate(t)) * (t - time) / 2;
  } else {
    return 0;
  }
}

Signal::Signal(const std::vector<Sample>& data) {
  this->samples.reserve(data.size());

  for (const auto s : data) {
    this->push_back(s);
  }
}

Signal::Signal(const std::vector<float>& points, const std::vector<float>& times) {
  if (points.size() != times.size()) {
    throw std::invalid_argument(
        "Number of sample points and time points need to be equal.");
  }

  size_t n = points.size();
  this->samples.reserve(n);
  if (n > 0) {
    for (size_t i = 0; i < n; i++) {
      this->push_back(Sample{times.at(i), points.at(i), 0.0});
    }
  }
}

inline float Signal::begin_time() const {
  return (samples.empty()) ? 0.0 : samples.front().time;
}

inline float Signal::end_time() const {
  return (samples.empty()) ? 0.0 : samples.back().time;
}

inline float Signal::interpolate(float t, size_t idx) const {
  return this->samples.at(idx).interpolate(t);
}

inline float Signal::time_intersect(const Sample& point, size_t idx) const {
  return this->samples.at(idx).time_intersect(point);
}

inline float Signal::area(float t, size_t idx) const {
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

Signal Signal::simplify() const {
  std::vector<Sample> sig;
  for (const auto& s : this->samples) {
    const auto [t, v, d] = s;
    if (sig.empty()) {
      sig.push_back(s);
    } else if (const auto& new_s = sig.back();
               new_s.interpolate(t) != v || new_s.derivative != d) {
      sig.push_back(s);
    }
  }

  return Signal{sig};
}

Signal Signal::resize(float start, float end, float fill) const {
  Signal sig;

  // Check if begin_time > start, then add filled value
  if (this->begin_time() > start) {
    sig.push_back(Sample{start, fill, 0.0});
  }

  // Truncate the discard all samples where sample.time < start
  size_t current = 0; // Index to the current sample
  for (const auto& s : this->samples) {
    const auto [t, v, d] = s;
    if (t < start) {
      // If current sample is timed below start, ...
      if (current + 1 < this->samples.size() &&
          this->samples.at(current + 1).time > start) {
        // and next sample is timed after `start`, append an intermediate value
        sig.push_back(Sample{start, s.interpolate(start), 0.0});
      }
      // Else we forget about the sample.
    } else if (start <= t && t <= end) {
      // If the samples are within the desired time range, keep the samples.
      sig.push_back(s);
    } else if (t > end) {
      // If we are out of the range, ...
      if (current - 1 >= 0 && this->samples.at(current - 1).time < end) {
        // and the previous sample is within the range, interpolate from the last.
        sig.push_back(Sample{end, this->interpolate(end, current - 1), 0.0});
      } else {
        // TODO Does it make sense to terminate early?
        break;
      }
    }

    current++;
  }

  return sig;
}

Signal Signal::shift(float dt) const {
  Signal sig{*this};
  for (auto& s : sig.samples) {
    s.time += dt;
  }

  return sig;
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
  for (const auto& s : sig.samples) {
    os << s;
  }
  os << "]";
  return os;
}

} // namespace signal
