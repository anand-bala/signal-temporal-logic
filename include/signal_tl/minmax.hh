#ifndef __SIGNAL_TEMPORAL_LOGIC_MINMAX_HH__
#define __SIGNAL_TEMPORAL_LOGIC_MINMAX_HH__

#include "signal_tl/signal.hh"

#include <functional>
#include <numeric>

namespace signal_tl {
namespace minmax {

/**
 * Compute the element-wise minimum/maximum (depending on value of Compare) between
 * two signals.
 */
template <typename Compare>
signal::SignalPtr compute_minmax_pair(
    const signal::SignalPtr input_x,
    const signal::SignalPtr input_y,
    Compare comp,
    bool synchronized);

signal::SignalPtr compute_elementwise_min(
    const signal::SignalPtr x,
    const signal::SignalPtr y,
    bool synchronized);

signal::SignalPtr compute_elementwise_max(
    const signal::SignalPtr x,
    const signal::SignalPtr y,
    bool synchronized);

/**
 * Compute the element-wise minimum/maximum (depending on value of Compare) between
 * multiple signals.
 */
template <typename Compare>
signal::SignalPtr compute_minmax_pair(
    const std::vector<signal::SignalPtr>& xs,
    Compare comp,
    bool synchronized);

signal::SignalPtr
compute_elementwise_min(const std::vector<signal::SignalPtr>& xs, bool synchronized);

signal::SignalPtr
compute_elementwise_max(const std::vector<signal::SignalPtr>& xs, bool synchronized);

/**
 * Compute the rolling min/max of a signal, i.e., at time t, the min/max value is the
 * sample with min/max value in the window [t, t + inf).
 */
template <typename Compare>
signal::SignalPtr compute_minmax_seq(const signal::SignalPtr x, Compare comp);

signal::SignalPtr compute_max_seq(const signal::SignalPtr x);
signal::SignalPtr compute_min_seq(const signal::SignalPtr x);

/**
 * Compute the windowed min/max of a signal, i.e., at time t, the min/max value is the
 * sample with min/max value in the window [t + a, t + b].
 */
template <typename Compare>
signal::SignalPtr
compute_minmax_seq(const signal::SignalPtr x, double a, double b, Compare comp);

signal::SignalPtr compute_max_seq(const signal::SignalPtr x, double a, double b);
signal::SignalPtr compute_min_seq(const signal::SignalPtr x, double a, double b);

} // namespace minmax
} // namespace signal_tl
#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_MINMAX_HH__ */
