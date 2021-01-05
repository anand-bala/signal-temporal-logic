#include "signal_tl/parser.hpp"

#include <catch2/catch.hpp>

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  REQUIRE(signal_tl::parser::analyze_specification_grammar(1) != 0);
}
