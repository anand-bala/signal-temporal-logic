#pragma once

#if !defined(__SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__)
#define __SIGNAL_TEMPORAL_LOGIC_SIGNAL_HH__

#include <deque>
#include <iostream>
#include <vector>

namespace signal {

struct Sample {
  float time;
  float value;
  float derivative;

  Sample() {}
  Sample(float time, float value, float derivative) :
      time(time), value(value), derivative(derivative) {}

  Sample constant() const;

  float at(const float&) const;
  float timeIntersect(const Sample&) const;
  float area(const float&) const;

  friend std::ostream& operator<<(std::ostream&, const Sample&);
};

inline Sample Sample::constant() const {
  return Sample(time, value, 0);
}

inline float Sample::at(const float& t) const {
  return value + derivative * (t - time);
}

inline float Sample::timeIntersect(const Sample& point) const {
  return (value - point.value + (point.derivative * point.time) - (derivative * time)) /
         (point.derivative - derivative);
}

inline float Sample::area(const float& t) const {
  if (t > time) {
    return (value + valueAt(t)) * (t - time) / 2;
  } else {
    return 0;
  }
}

/**
 * Piecewise-linear, right-continuous signal
 */
struct Signal {
  float beginTime;
  float endTime;
  std::deque<Sample> data;

  Signal() : beginTime{0.0}, endTime{0.0}, data{} {}
  /**
   * Create a PWLSignal from a sequence of amples
   */
  Signal(const std::vector<Sample>& data);
  /**
   * Create a PWLSignal from a sequence of data points and time stamps
   */
  Signal(const std::vector<float>&, const std::vector<float>&);

  /**
   * Add a Sample to the front of the Signal
   */
  int push_front(Sample);
  /**
   * Add a Sample to the back of the Signal
   */
  int push_back(Sample);
  /**
   * Remove sampling points where (y, dy) is continuous
   */
  void simplify();
  /**
   * Restrict/extend the signal to [s,t) with default value v where not defined.
   */
  void resize(float, float, float);
  /**
   * Shift the signal by dt time units
   */
  void shift(float); // shifts the signal of delta_t time units

  Signal reverse();

  void print() const;
  friend std::ostream& operator<<(std::ostream&, const Signal&);
};

} // namespace signal
#endif
