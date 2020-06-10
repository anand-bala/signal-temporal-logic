#include "bindings.hh"
#include "signal_tl/robustness.hh"

using namespace semantics;
using namespace signal;

void init_robustness_module(py::module& parent) {
  auto m = parent.def_submodule("semantics", "Robustness semantics for STL");

  py::enum_<Semantics>(m, "Semantics")
      .value("EFFICIENT", Semantics::EFFICIENT)
      .value("FILTERING", Semantics::FILTERING);

  m.def("compute_robustness", &compute_robustness, "phi"_a, "trace"_a);
}
