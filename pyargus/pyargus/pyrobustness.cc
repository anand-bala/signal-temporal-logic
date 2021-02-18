#include "bindings.hpp"             // for init_robustness_module
#include "signal_tl/ast.hpp"        // for Expr, signal_tl
#include "signal_tl/robustness.hpp" // for compute_robustness, semantics
#include "signal_tl/signal.hpp"     // for Trace, signal

#include "signal_tl/fmt.hpp" // IWYU pragma: keep

#include <pybind11/cast.h>          // for operator""_a, arg
#include <pybind11/detail/common.h> // for constexpr_first, ignore_unused
#include <pybind11/detail/descr.h>  // for operator+
#include <pybind11/pybind11.h>      // for module, module_
#include <pybind11/pytypes.h>       // for dict

#include <memory> // for shared_ptr
#include <vector> // for vector

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
