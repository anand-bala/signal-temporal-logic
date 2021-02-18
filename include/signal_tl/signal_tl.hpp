/// @dir signal_tl
/// @brief Directory contains the public C++ API for the @ref signal_tl namespace.
///

#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_SIGNALTL_HPP
#define SIGNAL_TEMPORAL_LOGIC_SIGNALTL_HPP

// IWYU pragma: begin_exports
#include "signal_tl/ast.hpp"
#include "signal_tl/exception.hpp"
#include "signal_tl/robustness.hpp"
#include "signal_tl/signal.hpp"
// IWYU pragma: end_exports

/// @namespace signal_tl
/// @brief The Signal Temporal Logic Libarary
namespace signal_tl {
using namespace signal;
using namespace semantics;
} // namespace signal_tl

#endif
