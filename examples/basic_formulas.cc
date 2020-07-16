#include "signal_tl/fmt.hh"
#include "signal_tl/signal_tl.hh"

#include <fmt/format.h>

namespace stl = signal_tl;

int main() {
  fmt::print("phi := {}\n", stl::Predicate("a"));

  fmt::print("----------------\n");

  fmt::print("phi := {}\n", stl::Predicate::as_expr("a"));

  return 0;
}
