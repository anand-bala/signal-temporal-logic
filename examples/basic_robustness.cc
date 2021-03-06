#include "signal_tl/signal_tl.hpp" // for Predicate, Signal, compute_robust...

#include "signal_tl/fmt.hpp" // IWYU pragma: keep
#include <fmt/format.h>      // for print

#include <memory>  // for make_shared, __shared_ptr_access
#include <variant> // for visit
#include <vector>  // for vector

namespace stl = signal_tl;
using namespace signal_tl::signal;

int main() {
  const auto phi = stl::Predicate("a") | stl::Predicate("b");

  fmt::print("phi := {}\n", phi);

  auto xs = std::make_shared<Signal>(
      std::vector<double>{0, 2, 1, -2, -1}, std::vector<double>{0, 2.5, 4.5, 6.5, 9});
  auto ys = std::make_shared<Signal>(
      std::vector<double>{0, -2, 2, 1, -1.5}, std::vector<double>{0, 2, 6, 8.5, 11});
  fmt::print("xs:\t{}\n", *xs);
  fmt::print("ys:\t{}\n", *ys);

  {
    const auto trace1 = Trace{{"a", xs}, {"b", ys}};
    const auto rob1   = stl::compute_robustness(phi, trace1);
    fmt::print("unsynched robustness:\t{}\n", *rob1);
  }
  {
    const auto [xs_, ys_] = synchronize(xs, ys);
    const auto trace1     = Trace{{"a", xs_}, {"b", ys_}};
    const auto rob1       = stl::compute_robustness(phi, trace1, true);

    fmt::print("synched robustness:\t{}\n", *rob1);
  }

  return 0;
}
