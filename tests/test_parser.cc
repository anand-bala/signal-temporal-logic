#include "signal_tl/parser.hpp"

#include <catch2/catch.hpp>

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  size_t issues = signal_tl::parser::internal::analyze_grammar(1);
  INFO("Number of issues = " << issues);
  REQUIRE(issues != 0);
}
