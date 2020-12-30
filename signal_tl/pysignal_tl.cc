#include "bindings.hpp"
#include "signal_tl/exception.hpp"

PYBIND11_MODULE(_cext, m) { // NOLINT
  py::register_exception<signal_tl::not_implemented_error>(
      m, "FunctionNotImplemented", PyExc_NotImplementedError);

  m.doc() = "Signal Temporal Logic library.";
  init_ast_module(m);
  init_signal_module(m);
  init_robustness_module(m);
}
