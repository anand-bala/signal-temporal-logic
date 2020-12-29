#include <algorithm>
#include <vector>

#include "signal_tl/fmt.hpp"
#include "signal_tl/signal.hpp"

#include <fmt/format.h>

using namespace signal_tl::signal;

int main() {
  fmt::print("\n===== Basic Signals =====\n");

  auto xs = std::make_shared<Signal>(
      std::vector<double>{0, 2, 1, -2, -1}, std::vector<double>{0, 2.5, 4.5, 6.5, 9});
  auto ys = std::make_shared<Signal>(
      std::vector<double>{0, -2, 2, 1, -1.5}, std::vector<double>{0, 2, 6, 8.5, 11});
  fmt::print("xs:\t{}\n", *xs);
  fmt::print("ys:\t{}\n", *ys);

  {
    fmt::print("\n===== Unsynched timestamps =====\n");
    auto xs_t = std::vector<double>{};
    std::transform(
        xs->begin(), xs->end(), std::back_inserter(xs_t), [](const Sample& s) {
          return s.time;
        });
    auto ys_t = std::vector<double>{};
    std::transform(
        ys->begin(), ys->end(), std::back_inserter(ys_t), [](const Sample& s) {
          return s.time;
        });

    fmt::print("xs time:\t{}\n", fmt::join(xs_t, ", "));
    fmt::print("ys time:\t{}\n", fmt::join(ys_t, ", "));
  }

  {
    fmt::print("\n===== Synched Signals =====\n");
    const auto [xs_, ys_] = synchronize(xs, ys);
    fmt::print("xs sync:\t{}\n", *xs_);
    fmt::print("ys sync:\t{}\n", *ys_);

    fmt::print("\n===== Synched timestamps =====\n");
    auto xs_t = std::vector<double>{};
    std::transform(
        xs_->begin(), xs_->end(), std::back_inserter(xs_t), [](const Sample& s) {
          return s.time;
        });
    auto ys_t = std::vector<double>{};
    std::transform(
        ys_->begin(), ys_->end(), std::back_inserter(ys_t), [](const Sample& s) {
          return s.time;
        });

    fmt::print("xs sync time:\t{}\n", fmt::join(xs_t, ", "));
    fmt::print("ys sync time:\t{}\n", fmt::join(ys_t, ", "));
  }
  return 0;
}
