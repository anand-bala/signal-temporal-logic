#include "signal_tl/signal.hh"
#include "utils.hh"

#include <cmath>
#include <exception>
#include <functional>
#include <iterator>

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

namespace {

void compute_segment_minmax(
    std::vector<Sample>& out,
    const Sample& i,
    double t,
    std::vector<Sample>::const_reverse_iterator& j,
    bool minimize = true) {
  std::function<bool(double, double)> op, op_c;
  if (minimize) {
    op   = std::less<double>();
    op_c = std::greater<double>();
  } else {
    op   = std::greater<double>();
    op_c = std::less<double>();
  }
  // PRECONDITIONS: j->time < t, i.time < t
  // POSTCONDITIONS: j->time <= i.time
  bool continued = false;
  double s       = j->time;

  // for every sample *j in (i.time, t)
  for (; s > i.time; (t = s), j++, (s = j->time)) {
    if (op(i.interpolate(t), j->interpolate(t))) {
      if (op_c(i.interpolate(s), j->value)) {
        t = i.time_intersect(*j);
        out.push_back({t, i.interpolate(t), i.derivative});
        out.push_back({s, j->value, j->derivative});
        continued = false;
      } else
        continued = true;
    } else if (i.interpolate(t) == j->interpolate(t)) {
      if (op_c(i.interpolate(s), j->value)) {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
          continued = false;
        }
        out.push_back({s, j->value, j->derivative});
      } else
        continued = true;
    } else {
      if (op(i.interpolate(s), j->value)) {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
        }
        t = i.time_intersect(*j);

        out.push_back({t, j->interpolate(t), j->derivative});
        continued = true;
      } else {
        if (continued) {
          out.push_back({t, i.interpolate(t), i.derivative});
          continued = false;
        }
        out.push_back({s, j->value, j->derivative});
      }
    }
  }

  // here we may have j->time < i.time
  // "i" values of z are no longer "continued"
  s = i.time;
  if (op(i.interpolate(t), j->interpolate(t))) {
    if (op_c(i.value, j->interpolate(s))) {
      t = i.time_intersect(*j);
      out.push_back({t, i.interpolate(t), i.derivative});
      out.push_back({s, j->interpolate(s), j->derivative});
    } else {
      out.push_back(i);
    }
  } else if (i.interpolate(t) == j->interpolate(t)) {
    if (op_c(i.value, j->interpolate(s))) {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      out.push_back({s, j->interpolate(s), j->derivative});
    } else {
      out.push_back(i);
    }
  } else {
    if (op(i.value, j->interpolate(s))) {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      t = i.time_intersect(*j);
      out.push_back({t, j->interpolate(t), j->derivative});
      out.push_back(i);

    } else {
      if (continued) {
        out.push_back({t, i.interpolate(t), i.derivative});
      }
      out.push_back({s, j->interpolate(s), j->derivative});
    }
  }
}

} // namespace

SignalPtr min(const SignalPtr y1, const SignalPtr y2) {
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
    compute_segment_minmax(sig_stack, *i, t, j, true);
    if (j->time == i->time)
      j++;
  }
  if (i->time == begin_time) {
    compute_segment_minmax(sig_stack, *i, t, j, true);
  } else {
    compute_segment_minmax(
        sig_stack, {begin_time, i->interpolate(begin_time), i->derivative}, t, j, true);
  }

  auto out = std::make_shared<Signal>(std::rbegin(sig_stack), std::rend(sig_stack));
  return out->simplify();
}

SignalPtr min(const std::vector<SignalPtr>& ys) {
  auto out = std::make_shared<Signal>();
  if (ys.empty()) {
    out->push_back(0, -std::numeric_limits<double>::infinity());
    return out;
  } else if (ys.size() == 1) {
    return ys.at(0);
  }

  out = ys.at(0);
  for (size_t i = 1; i < ys.size(); i++) out = min(out, ys.at(1));
  return out;
}

SignalPtr max(const SignalPtr y1, const SignalPtr y2) {
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
    compute_segment_minmax(sig_stack, *i, t, j, false);
    if (j->time == i->time)
      j++;
  }
  if (i->time == begin_time) {
    compute_segment_minmax(sig_stack, *i, t, j, false);
  } else {
    compute_segment_minmax(
        sig_stack,
        {begin_time, i->interpolate(begin_time), i->derivative},
        t,
        j,
        false);
  }

  auto out = std::make_shared<Signal>(std::rbegin(sig_stack), std::rend(sig_stack));
  return out->simplify();
}

SignalPtr max(const std::vector<SignalPtr>& ys) {
  auto out = std::make_shared<Signal>();
  if (ys.empty()) {
    out->push_back(0, -std::numeric_limits<double>::infinity());
    return out;
  } else if (ys.size() == 1) {
    return ys.at(0);
  }

  out = ys.at(0);
  for (size_t i = 1; i < ys.size(); i++) out = min(out, ys.at(1));
  return out;
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
