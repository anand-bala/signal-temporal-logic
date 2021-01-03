#include "bindings.hpp"
#include "signal_tl/exception.hpp"

#include <pybind11/detail/common.h> // for PYBIND11_MODULE
#include <pybind11/pybind11.h>      // for module_, register_exception
#include <pybind11/pytypes.h>       // for str_attr_accessor
#include <pyerrors.h>               // for PyErr_SetString, PyExc_NotImplem...

PYBIND11_MODULE(_cext, m) { // NOLINT
  py::register_exception<signal_tl::not_implemented_error>(
      m, "FunctionNotImplemented", PyExc_NotImplementedError);

  m.doc() = "Signal Temporal Logic library.";
  init_ast_module(m);
  init_signal_module(m);
  init_robustness_module(m);
}
