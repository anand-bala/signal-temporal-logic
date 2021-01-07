#include "signal_tl/grammar.hpp"
#include "signal_tl/parser.hpp"

#include <catch2/catch.hpp>

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  size_t issues = signal_tl::grammar::internal::analyze(1);
  INFO("Number of issues = " << issues);
  REQUIRE(issues != 0);
}
