#include "bindings.hh"
#include "signal_tl/robustness.hh"

#include <exception>

using namespace semantics;
using namespace signal;

void init_robustness_module(py::module& parent) {
  auto m = parent.def_submodule("semantics", "Robustness semantics for STL");

  py::enum_<Semantics>(m, "Semantics")
      .value("CLASSIC", Semantics::CLASSIC)
      .value("EFFICIENT", Semantics::EFFICIENT)
      .value("FILTERING", Semantics::FILTERING);

  m.def(
      "compute_robustness",
      [](const ast::Expr phi, Trace& trace, Semantics sem, bool synchronized) {
        switch (sem) {
          case Semantics::EFFICIENT:
            return compute_robustness<Semantics::EFFICIENT>(phi, trace, synchronized);
          case Semantics::CLASSIC:
            return compute_robustness<Semantics::CLASSIC>(phi, trace, synchronized);
          default:
            throw std::invalid_argument(
                "Robustness function not defined for given semantics");
        }
      },
      "phi"_a,
      "trace"_a,
      "semantics"_a    = Semantics::CLASSIC,
      "synchronized"_a = false);
}
