#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__
#define __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

namespace semantics {
using namespace signal;

Signal compute_robustness(
    const ast::Expr phi,
    const std::map<std::string, std::shared_ptr<Signal>>& trace);

} // namespace semantics

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HH__ */
