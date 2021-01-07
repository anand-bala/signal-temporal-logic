#include "signal_tl/parser.hpp"

#include <catch2/catch.hpp>
#include <iostream>

#include <utility>

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  size_t issues = signal_tl::grammar::internal::analyze(1);
  INFO("Number of issues = " << issues);
  REQUIRE(issues == 0);
}

TEST_CASE("Parsing of input specifications", "[parser][acceptance]") {
  auto valid_spec = GENERATE(values({
      "(define-formula phi1 (< p 0))\n"
      "(assert monitor phi1)\n",
      "; Here, `phi1` is the name of the formula (which should be fetchable from some\n"
      "; hash table or something) and `x` is some signal value. Here we are defining\n"
      "; `always (x > 0)`.\n"
      "(define-formula phi1 (always (> x 0)))\n"
      "; Now we will define some regular formulas.\n"
      "(define-formula phi2 (< p 0))\n"
      "(define-formula phi3 (> q 0))\n"
      "(define-formula phi4 (and phi2 phi3))\n"
      "(define-formula phi5 (eventually phi4))\n"
      "(define-formula phi6 (always phi5))\n"
      "(assert monitor phi6)\n",
  }));
  SECTION("Valid specifications are parsed") {
    REQUIRE_NOTHROW(signal_tl::parser::from_string(valid_spec));
  }

  SECTION("Parsed specifications are correct") {
    REQUIRE_NOTHROW(signal_tl::parser::from_string(valid_spec));
  }
}
