#ifndef __SIGNAL_TL_MINMAX_HH__
#define __SIGNAL_TL_MINMAX_HH__

#include "signal_tl/signal.hh"

namespace minmax {

/**
 * Compare segments from between two diffrent signals
 */
template <typename Compare>
void segment_minmax(
    std::vector<signal::Sample>& out,
    const signal::Sample& i,
    double t,
    std::vector<signal::Sample>::const_reverse_iterator& j,
    Compare op);

/**
 * Compute ordering for segments within a single signal
 */
template <typename Compare>
void partial_comp(
    std::vector<signal::Sample>& out,
    std::vector<signal::Sample>::const_reverse_iterator& i,
    double start_time,
    double end_time,
    Compare op);

} // namespace minmax

#endif /* end of include guard: __SIGNAL_TL_MINMAX_HH__ */
