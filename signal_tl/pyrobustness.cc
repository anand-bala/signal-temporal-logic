#include "bindings.hpp"
#include "signal_tl/fmt.hpp"
#include "signal_tl/robustness.hpp"

using namespace signal_tl;
using namespace semantics;
using namespace signal;

void init_robustness_module(py::module& parent) {
  auto m = parent.def_submodule("semantics", "Robustness semantics for STL");

  m.def(
      "compute_robustness",
      [](const ast::Expr& phi, const Trace& trace, bool synchronized) {
        return compute_robustness(phi, trace, synchronized);
      },
      "phi"_a,
      "trace"_a,
      "synchronized"_a = false);
}
