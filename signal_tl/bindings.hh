#pragma once

#ifndef __SIGNAL_TL_PYTHON_BINDINGS_HH__
#define __SIGNAL_TL_PYTHON_BINDINGS_HH__

#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <pybind11/operators.h>

#include "signal_tl/fmt.hh"
#include "signal_tl/signal_tl.hh"

using namespace pybind11::literals;
namespace py = pybind11;
using namespace signal_tl;

// PYBIND11_MAKE_OPAQUE(std::vector<signal::Sample>);
// PYBIND11_MAKE_OPAQUE(std::map<std::string, signal::SignalPtr>);

void init_ast_module(py::module&);
void init_signal_module(py::module&);
void init_robustness_module(py::module&);

#endif /* end of include guard: __SIGNAL_TL_PYTHON_BINDINGS_HH__ */
