#include "signal_tl/ast.hpp" // for Predicate, signal_tl
#include "signal_tl/fmt.hpp" // IWYU pragma: keep

#include <fmt/format.h> // for print
#include <memory>       // for allocator

namespace stl = signal_tl;

int main() {
  fmt::print("phi := {}\n", stl::ast::Predicate("a"));

  fmt::print("----------------\n");

  fmt::print("phi := {}\n", stl::Predicate("a") & stl::Predicate("b"));

  return 0;
}
