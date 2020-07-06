/**
 * This file will test the functionality of the Signal class
 */

#include "signaltl_tests.hh"

using namespace signal_tl::signal;

TEST_CASE("Signals are monotonically increasing", "[Signal]") {
  auto points   = std::vector<double>{25.0, 15.0, 22.0, -1.0};
  auto time_pts = std::vector<double>{0, 0.25, 5, 6.25};
  auto sig      = std::make_shared<Signal>(points, time_pts);

  REQUIRE(sig->size() == 4);
  REQUIRE(sig->begin_time() == Approx(0));
  REQUIRE(sig->end_time() == Approx(6.25));

  auto time_point = GENERATE(take(100, random(0.0, 6.25)));
  REQUIRE_THROWS(sig->push_back(time_point, 0.0));
}
