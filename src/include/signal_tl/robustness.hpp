#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HPP
#define SIGNAL_TEMPORAL_LOGIC_ROBUSTNESS_HPP

#include "signal_tl/ast.hpp"
#include "signal_tl/signal.hpp"

#include <map>
#include <memory>

/// @namespace signal_tl::semantics
/// @brief API to access variout semantics for Signal Temporal Logic.
namespace signal_tl::semantics {

/// Compute the robustness of a given formula `phi` against a trace.
///
/// \note
/// Setting the parameter `synchronized` currently does nothing. Initially, it was
/// intended to be used by the user to say if the timestamps for each signal in the @ref
/// signal::Trace have been synchronized. For now, we do a redundant synchronization
/// step before computing the robustness.
signal::SignalPtr compute_robustness(
    const ast::Expr& phi,
    const signal::Trace& trace,
    bool synchronized = false);

} // namespace signal_tl::semantics

#endif
