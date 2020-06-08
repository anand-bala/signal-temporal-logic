#include "signal_tl/robustness.hh"

namespace semantics {
using namespace signal;

namespace {}

Signal compute_robustness(
    const ast::Expr phi,
    const std::map<std::string, std::shared_ptr<Signal>>& trace) {
  return *trace.at("a");

  // return sig;
}

} // namespace semantics
