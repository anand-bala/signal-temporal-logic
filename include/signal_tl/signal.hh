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
  double derivative;

  /**
   * Linear interpolate the Sample (given the derivative) to get the value at time `t`.
   */
  double interpolate(double t) const;
  /**
   * Get the time point at which the lines associated with this Sample and the given
   * Sample intersect.
   */
  double time_intersect(const Sample& point) const;
  double area(double t) const;

  friend std::ostream& operator<<(std::ostream& out, const Sample& sample);
};

/**
 * Piecewise-linear, right-continuous signal
 */
struct Signal {
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
  template <typename Iter>
  Signal(const Iter start, const Iter end);

  double begin_time() const;
  double end_time() const;
  double interpolate(double t, size_t idx) const;
  double time_intersect(const Sample& point, size_t idx) const;
  double area(double t, size_t idx) const;

  Sample front() const;
  Sample back() const;

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

  friend std::ostream& operator<<(std::ostream& os, const Signal& sig);

 private:
  std::vector<Sample> samples;
};

using SignalPtr = std::shared_ptr<Signal>;
using Trace     = std::map<std::string, SignalPtr>;

/**
 * Compute the Signal that represents the minimum values of two signals. The output
 * signals will have the points where the two signals intersect (if they are
 * significant)
 */
SignalPtr min(const SignalPtr y1, const SignalPtr y2);
/**
 * Computes the Signal representing the minimum of multiple signals.
 */
SignalPtr min(const std::vector<SignalPtr> ys);

/**
 * Compute the Signal that represents the maximum values of two signals. The output
 * signals will have the points where the two signals intersect (if they are
 * significant)
 */
SignalPtr max(const SignalPtr y1, const SignalPtr y2);
/**
 * Computes the Signal representing the maximum of multiple signals.
 */
SignalPtr max(const std::vector<SignalPtr> ys);

} // namespace signal
#endif
