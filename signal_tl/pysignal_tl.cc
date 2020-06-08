#include "bindings.hh"

PYBIND11_MODULE(_cext, m) {
  m.doc() = "Signal Temporal Logic library.";
  init_ast_module(m);
  init_signal_module(m);
  init_robustness_module(m);
}
