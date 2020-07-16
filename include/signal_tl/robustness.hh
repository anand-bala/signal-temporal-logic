#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__
#define __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

#include <map>
#include <memory>

namespace signal_tl {
namespace semantics {

enum struct Semantics {
  CLASSIC,
  FILTERING,
  CUMULATIVE,
};

template <Semantics S = Semantics::CLASSIC>
signal::SignalPtr compute_robustness(
    const ast::Expr phi,
    const signal::Trace& trace,
    bool synchronized = false);

} // namespace semantics
} // namespace signal_tl

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__ */
