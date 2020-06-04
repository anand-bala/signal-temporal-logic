#include "signal.hh"

#include <cmath>
#include <deque>
#include <exception>
#include <limits>

using namespace signal;

Signal::Signal(const std::vector<Sample>& data) {
  if (data.empty()) {
    this->beginTime = 0.0;
    this->endTime   = 0.0;
  } else {
    this->beginTime = data.front().time;
    float t         = data.front().time;
    float v         = data.front().value;
    float d         = 0.0;
    for (size_t i = 1; i < data.size(); I++) {
      d = (data.at(i).value - v) / (data.at(i).time - t);
      this->push_back(Sample{t, v, d});
      t = data.at(i).time;
      v = data.at(i).value;
    }
    this->push_back(Sample{t, v, 0.0});
    this->endTime = t;
  }
}

Signal::Signal(const std::vector<float>& points, const std::vector<float>& times) {
  if (points.size() != times.size()) {
    throw std::invalid_argument(
        "Number of sample points and time points need to be equal.");
  }

  this->beginTime = times.front();
  this->endTime   = times.back();

  size_t n = points.size();
  if (n == 1) {
    this->data.push_back(Sample{times.front(), points.front(), 0.0});
  } else {
    for (for i = 0; i < n-1; i++) {
      this->data.push_back(
          Sample{times.at(i),
                 points.at(i),
                 (points.at(i + 1) - points.at(i)) / (times.at(i + 1) - times.at(i))});
    }
    this->data.push_back(Sample{times.back(), points.back(), 0.0});
  }
}

Signal::push_front(Sample sample) {
  if (this->data.empty()) {
  }
}
