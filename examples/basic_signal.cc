#include "signal_tl/signal.hh"

#include <iomanip>
#include <iostream>

using namespace signal_tl::signal;

int main() {
  auto xs = std::make_shared<Signal>(
      std::vector<double>{0, 2, 1, -2, -1}, std::vector<double>{0, 2.5, 4.5, 6.5, 9});
  auto ys = std::make_shared<Signal>(
      std::vector<double>{0, -2, 2, 1, -1.5}, std::vector<double>{0, 2, 6, 8.5, 11});
  const auto [xs_, ys_] = synchronize(xs, ys);
  std::cout << "xs:\t" << *xs << std::endl << "ys:\t" << *ys << std::endl;

  std::cout << std::endl;

  std::cout << "xs t:\t\t";
  for (const auto& [t, v, d] : *xs) { std::cout << std::setw(4) << t << " "; }
  std::cout << std::endl;
  std::cout << "ys t:\t\t";
  for (const auto& [t, v, d] : *ys) { std::cout << std::setw(4) << t << " "; }
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "sync xs t:\t";
  for (const auto& [t, v, d] : *xs_) { std::cout << std::setw(4) << t << " "; }
  std::cout << std::endl;
  std::cout << "sync ys t:\t";
  for (const auto& [t, v, d] : *ys_) { std::cout << std::setw(4) << t << " "; }
  std::cout << std::endl;

  std::cout << std::endl;

  std::cout << "xs_:\t" << *xs_ << std::endl << "ys_:\t" << *ys_ << std::endl;

  return 0;
}
