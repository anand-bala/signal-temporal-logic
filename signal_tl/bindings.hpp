#pragma once

#ifndef SIGNAL_TL_PYTHON_BINDINGS_HPP
#define SIGNAL_TL_PYTHON_BINDINGS_HPP

#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <pybind11/operators.h>

using namespace pybind11::literals;
namespace py = pybind11;

void init_ast_module(py::module&);
void init_signal_module(py::module&);
void init_robustness_module(py::module&);

#endif
