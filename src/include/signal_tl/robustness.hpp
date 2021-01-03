#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HPP
#define SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HPP

#include "signal_tl/ast.hpp"
#include "signal_tl/signal.hpp"

#include <map>
#include <memory>

namespace signal_tl::semantics {

signal::SignalPtr compute_robustness(
    const ast::Expr& phi,
    const signal::Trace& trace,
    bool synchronized = false);

} // namespace signal_tl::semantics

#endif
