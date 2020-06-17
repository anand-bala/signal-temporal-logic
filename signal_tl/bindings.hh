#pragma once

#ifndef __SIGNAL_TL_PYTHON_BINDINGS_HH__
#define __SIGNAL_TL_PYTHON_BINDINGS_HH__

#include <iostream>
#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <pybind11/operators.h>

#include "signal_tl/ast.hh"
#include "signal_tl/robustness.hh"
#include "signal_tl/signal.hh"

using namespace pybind11::literals;
namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<signal::Sample>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, signal::SignalPtr>);

template <typename T>
std::string repr(const T& expr) {
  std::ostringstream os;
  os << expr;
  return os.str();
}

void init_ast_module(py::module&);
void init_signal_module(py::module&);
void init_robustness_module(py::module&);

#endif /* end of include guard: __SIGNAL_TL_PYTHON_BINDINGS_HH__ */
