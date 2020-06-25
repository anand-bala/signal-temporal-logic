#include "signal_tl/signal.hh"

#include "signal_tl/minmax.hh"

#include <limits>
#include <vector>

using namespace signal;

namespace minmax {

constexpr double TOP    = std::numeric_limits<double>::infinity();
constexpr double BOTTOM = -TOP;

template <typename Compare>
void segment_minmax(
    std::vector<Sample>& out,
    const Sample& i,
    double t,
    std::vector<Sample>::const_reverse_iterator& j,
    Compare op) {
  // PRECONDITIONS: j->time < t, i.time < t
  // POSTCONDITIONS: j->time <= i.time
  bool continued = false;
  double s       = j->time;

  // for every sample *j in (i.time, t)
  for (; s > i.time; (t = s), j++, (s = j->time)) {
    if (op(i.interpolate(t), j->interpolate(t))) {
      if (op(j->value, i.interpolate(s))) {
        t = i.time_intersect(*j);
        out.push_back({t, i.interpolate(t), i.derivative});
        out.push_back({s, j->value, j->derivative});
        continued = false;
      } else
        continued = true;
    } else if (i.interpolate(t) == j->interpolate(t)) {
      if (op(j->value, i.interpolate(s))) {
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
    if (op(j->interpolate(s), i.value)) {
      t = i.time_intersect(*j);
      out.push_back({t, i.interpolate(t), i.derivative});
      out.push_back({s, j->interpolate(s), j->derivative});
    } else {
      out.push_back(i);
    }
  } else if (i.interpolate(t) == j->interpolate(t)) {
    if (op(j->interpolate(s), i.value)) {
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

template <typename Compare>
void partial_comp(
    std::vector<signal::Sample>& out,
    std::vector<signal::Sample>::const_reverse_iterator& i,
    double start_time,
    double end_time,
    Compare op) {
  bool continued = false;
  double z_max   = BOTTOM;
  double s = start_time, t = end_time;
  while (i->time > s) {
    if (i->derivative >= 0) {
      if (z_max < i->interpolate(t)) {
        if (continued) {
          out.push_back(Sample{t, z_max, 0});
        }
        z_max = i->interpolate(t);
      }
      continued = true;
      // out.push_back(Sample(i->time, z_max, 0));
    } else if (i->interpolate(t) >= z_max) {
      if (continued) {
        out.push_back(Sample{t, z_max, 0});
        continued = false;
      }
      z_max = i->value;
      out.push_back(*i);
    } else if (z_max >= i->value) {
      continued = true;
      // out.push_back(Sample(i->time, z_max, 0));
    } else {
      out.push_back(Sample{i->time + (z_max - i->value) / i->derivative,
                           z_max,
                           0}); // time at which y reaches value next_z
      out.push_back(*i);
      z_max     = i->value;
      continued = false;
    }

    t = i->time;
    i++;
  }

  // leftmost sample *i may not be on s
  //"z_max" values of z are not longer "continued".
  if (i->derivative >= 0) {
    if (z_max < i->interpolate(t)) {
      if (continued) {
        out.push_back(Sample{t, z_max, 0});
      }
      z_max = i->interpolate(t);
    }
    out.push_back(Sample{s, z_max, 0});
  } else if (i->interpolate(t) >= z_max) {
    if (continued) {
      out.push_back(Sample{t, z_max, 0});
    }
    out.push_back(Sample{s, i->interpolate(s), i->derivative});
  } else if (z_max >= i->interpolate(s)) {
    out.push_back(Sample{s, z_max, 0});
  } else {
    out.push_back(Sample{s + (z_max - i->value) / i->derivative,
                         z_max,
                         0}); // time at which y reaches value next_z
    out.push_back(Sample{s, i->interpolate(s), i->derivative});
  }
}

} // namespace minmax
