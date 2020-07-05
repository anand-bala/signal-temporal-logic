#include "signal_tl/signal_tl.hh"

#include <iomanip>
#include <iostream>

namespace stl = signal_tl;
using namespace signal_tl::signal;

int main() {
  const auto phi = stl::Predicate::as_expr("a") | stl::Predicate::as_expr("b");

  auto xs = std::make_shared<Signal>(
      std::vector<double>{0, 2, 1, -2, -1}, std::vector<double>{0, 2.5, 4.5, 6.5, 9});
  auto ys = std::make_shared<Signal>(
      std::vector<double>{0, -2, 2, 1, -1.5}, std::vector<double>{0, 2, 6, 8.5, 11});
  std::cout << "xs:\t" << *xs << std::endl << std::endl;
  std::cout << "ys:\t" << *ys << std::endl << std::endl;

  {
    const auto trace1 = Trace{{"a", xs}, {"b", ys}};
    const auto rob1   = stl::compute_robustness<stl::Semantics::CLASSIC>(phi, trace1);

    std::cout << "unsynched:\t" << *rob1 << std::endl << std::endl;
  }
  {
    const auto [xs_, ys_] = synchronize(xs, ys);
    const auto trace1     = Trace{{"a", xs_}, {"b", ys_}};
    const auto rob1 =
        stl::compute_robustness<stl::Semantics::CLASSIC>(phi, trace1, true);

    std::cout << "synched:\t" << *rob1 << std::endl << std::endl;
  }

  return 0;
}
