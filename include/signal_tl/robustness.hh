#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__
#define __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

namespace semantics {
using namespace signal;

enum struct Semantics {
  EFFICIENT,
  FILTERING,
};

Signal compute_robustness(const ast::Expr phi, const Trace& trace);

} // namespace semantics

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__ */
