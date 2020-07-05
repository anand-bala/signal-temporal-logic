#include "bindings.hh"

PYBIND11_MODULE(_cext, m) {
  py::register_exception_translator([](std::exception_ptr p) {
    try {
      if (p)
        std::rethrow_exception(p);
    } catch (const not_implemented_error& e) {
      PyErr_SetString(PyExc_NotImplementedError, e.what());
    }
  });

  m.doc() = "Signal Temporal Logic library.";
  init_ast_module(m);
  init_signal_module(m);
  init_robustness_module(m);
}
