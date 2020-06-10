#pragma once

#if !defined(__SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__)
#define __SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__

#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace signal {

struct Sample {
  float time;
  float value;
  float derivative;

  float interpolate(float t) const;
  float time_intersect(const Sample& point) const;
  float area(float t) const;

  friend std::ostream& operator<<(std::ostream& out, const Sample& sample);
};

/**
 * Piecewise-linear, right-continuous signal
 */
struct Signal {
  Signal() : samples{} {}
  /**
   * Create a Signal from a sequence of amples
   */
  Signal(const std::vector<Sample>& data);
  /**
   * Create a Signal from a sequence of data points and time stamps
   */
  Signal(const std::vector<float>& points, const std::vector<float>& times);

  float begin_time() const;
  float end_time() const;
  float interpolate(float t, size_t idx) const;
  float time_intersect(const Sample& point, size_t idx) const;
  float area(float t, size_t idx) const;

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
    while (it->time < s)
      it++;
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
    while (it->time > t)
      it--;
    // Now we have the pointer to the first element from the back whose .time <= t.
    // So increment by 1 and return
    it++;
    return it;
  }

  /**
   * Add a Sample to the back of the Signal
   */
  void push_back(Sample s);
  void push_back_raw(Sample s);
  /**
   * Remove sampling points where (y, dy) is continuous
   */
  Signal simplify() const;
  /**
   * Restrict/extend the signal to [s,t] with default value v where not defined.
   */
  Signal resize(float start, float end, float fill) const;
  /**
   * Shift the signal by dt time units
   */
  Signal shift(float dt) const;

  friend std::ostream& operator<<(std::ostream& os, const Signal& sig);

 private:
  std::vector<Sample> samples;
};

using Trace = std::map<std::string, std::shared_ptr<Signal>>;

} // namespace signal
#endif
