#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__
#define __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

#include <memory>

namespace semantics {

enum struct Semantics {
  CLASSIC,
  EFFICIENT,
  FILTERING,
  CUMULATIVE,
};

template <Semantics S>
signal::SignalPtr compute_robustness(
    const ast::Expr phi,
    const signal::Trace& trace,
    bool synchronized = false);

} // namespace semantics

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__ */
