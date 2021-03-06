#include "signal_tl/signal.hpp" // for Sample, Signal, signal

#include <catch2/catch.hpp> // for Approx, operator==, SourceLineInfo

#include <memory> // for __shared_ptr_access, shared_ptr, all...
#include <random> // for default_random_engine, random_device
#include <vector> // for vector

using namespace signal_tl::signal;

namespace {
class MonotonicIncreasingTimestampedSignal
    : public Catch::Generators::IGenerator<Sample> {
  std::default_random_engine rng;
  std::uniform_real_distribution<> dist;
  double current_end_time{0.0};
  Sample current_last_sample;

 public:
  MonotonicIncreasingTimestampedSignal(
      double interval_size = 10.0, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      double delta = 0.1) :
      rng(std::random_device{}()), dist(delta, interval_size) {
    current_last_sample =
        Sample{current_end_time, 10.0}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  }

  [[nodiscard]] Sample const& get() const override;

  bool next() override {
    current_end_time += dist(rng);
    current_last_sample.time = current_end_time;
    return true;
  }
};

Catch::Generators::GeneratorWrapper<Sample> mono_increase_sample(
    double interval_size = 10.0, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    double delta         = 0.1) {        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  return Catch::Generators::GeneratorWrapper<Sample>(
      std::unique_ptr<Catch::Generators::IGenerator<Sample>>(
          new MonotonicIncreasingTimestampedSignal(interval_size, delta)));
}

} // namespace

Sample const& MonotonicIncreasingTimestampedSignal::get() const {
  return current_last_sample;
}

TEST_CASE("Signals are monotonically increasing", "[signal]") {
  SECTION("Manually created signal") {
    auto points = std::vector<double>{
        25.0, 15.0, 22.0, -1.0}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    auto time_pts = std::vector<double>{
        0, 0.25, 5, 6.25}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    auto sig = std::make_shared<Signal>(points, time_pts);

    REQUIRE(sig->size() == 4);
    REQUIRE(sig->begin_time() == Approx(0));
    REQUIRE(
        sig->end_time() ==
        Approx(6.25)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    auto time_point = GENERATE(
        take(100, random(0.0, 6.25))); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    REQUIRE_THROWS(sig->push_back(time_point, 0.0));
  }

  SECTION("Automatically generated signals") {
    auto samples = GENERATE(take(1000, chunk(50, mono_increase_sample())));
    REQUIRE_NOTHROW(Signal{samples});
  }
}
