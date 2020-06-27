#pragma once

#if !defined(__SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__)
#define __SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__

#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace signal {

struct Sample {
  double time;
  double value;
  double derivative = 0.0;

  /**
   * Linear interpolate the Sample (given the derivative) to get the value at time `t`.
   */
  inline double interpolate(double t) const {
    return value + derivative * (t - time);
  }
  /**
   * Get the time point at which the lines associated with this Sample and the given
   * Sample intersect.
   */
  inline double time_intersect(const Sample& point) const {
    return (value - point.value + (point.derivative * point.time) -
            (derivative * time)) /
           (point.derivative - derivative);
  }
  inline double area(double t) const {
    if (t > time) {
      return (value + this->interpolate(t)) * (t - time) / 2;
    } else {
      return 0;
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const Sample& sample);
};

constexpr bool operator<(const Sample& lhs, const Sample& rhs) {
  return lhs.value < rhs.value;
}
constexpr bool operator>(const Sample& lhs, const Sample& rhs) {
  return rhs < lhs;
}
constexpr bool operator<=(const Sample& lhs, const Sample& rhs) {
  return !(lhs > rhs);
}
constexpr bool operator>=(const Sample& lhs, const Sample& rhs) {
  return !(lhs < rhs);
}

/**
 * Piecewise-linear, right-continuous signal
 */
struct Signal {
  double begin_time() const {
    return (samples.empty()) ? 0.0 : samples.front().time;
  }

  double end_time() const {
    return (samples.empty()) ? 0.0 : samples.back().time;
  }

  double interpolate(double t, size_t idx) const {
    return this->samples.at(idx).interpolate(t);
  }

  double time_intersect(const Sample& point, size_t idx) const {
    return this->samples.at(idx).time_intersect(point);
  }

  double area(double t, size_t idx) const {
    return this->samples.at(idx).area(t);
  }

  Sample front() const {
    return this->samples.front();
  }

  Sample back() const {
    return this->samples.back();
  }

  Sample at_idx(size_t i) const {
    return this->samples.at(i);
  }

  /**
   * Get the sample at time `t`.
   *
   * Does a binary search for the given time instance, and interpolates from
   * the closest sample less than `t` if necessary.
   */
  Sample at(double t) const;
  /**
   * Get const_iterator to the start of the signal
   */
  auto begin() const {
    return this->samples.cbegin();
  }

  /**
   * Get const_iterator to the end of the signal
   */
  auto end() const {
    return this->samples.cend();
  }

  /**
   * Get const_iterator to the first element of the signal that is timed at or after `s`
   */
  auto begin_at(double s) const {
    auto it = this->begin();
    while (it->time < s) it = std::next(it);
    return it;
  }

  /**
   * Get const_iterator to the element after the last element of the signal
   * that is timed at or before `t`
   */
  auto end_at(double t) const {
    if (this->end_time() <= t)
      return this->end();

    auto it = this->end();
    while (it->time > t) it = std::prev(it);
    // Now we have the pointer to the first element from the back whose .time <= t.
    // So increment by 1 and return
    return std::next(it);
  }

  /**
   * Get const reverse_iterator to the samples.
   */
  auto rbegin() const {
    return this->samples.crbegin();
  }

  /**
   * Get const reverse_iterator to the samples.
   */
  auto rend() const {
    return this->samples.crend();
  }

  size_t size() const {
    return this->samples.size();
  }

  bool empty() const {
    return this->samples.empty();
  }

  /**
   * Add a Sample to the back of the Signal
   */
  void push_back(Sample s);
  void push_back(double time, double value);
  void push_back_raw(Sample s);
  /**
   * Remove sampling points where (y, dy) is continuous
   */
  std::shared_ptr<Signal> simplify() const;
  /**
   * Restrict/extend the signal to [s,t] with default value v where not defined.
   */
  std::shared_ptr<Signal> resize(double start, double end, double fill) const;
  /**
   * Shift the signal by dt time units
   */
  std::shared_ptr<Signal> shift(double dt) const;

  /**
   * Resize and shift a signal without creating copies. We use this often, so it makes
   * sense to combine it.
   */
  std::shared_ptr<Signal>
  resize_shift(double start, double end, double fill, double dt) const;

  friend std::ostream& operator<<(std::ostream& os, const Signal& sig);

  Signal() : samples{} {}

  Signal(const Signal& other);
  /**
   * Create a Signal from a sequence of amples
   */
  Signal(const std::vector<Sample>& data);
  /**
   * Create a Signal from a sequence of data points and time stamps
   */
  Signal(const std::vector<double>& points, const std::vector<double>& times);

  /**
   * Create a Signal from the given iterators
   */
  // template <typename Iter>
  // Signal(Iter start, Iter end);

 private:
  std::vector<Sample> samples;
};

using SignalPtr = std::shared_ptr<Signal>;
using Trace     = std::map<std::string, SignalPtr>;

} // namespace signal
#endif
