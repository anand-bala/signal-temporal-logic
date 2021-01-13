#include "signal_tl/internal/filesystem.hpp"
#include "signal_tl/parser.hpp"

#include <catch2/catch.hpp>
#include <iostream>

#include <string_view>
#include <utility>

#ifndef SIGNALTL_TESTS_DIR
#error "SIGNALTL_TESTS_DIR has not been defined in the preprocessor stage"
#endif

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  size_t issues = signal_tl::grammar::internal::analyze(1);
  INFO("Number of issues = " << issues);
  REQUIRE(issues == 0);
}

TEST_CASE("Parsing of string input specifications", "[parser][string-input]") {
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
}

TEST_CASE("Parsing of file input specifications", "[parser][file-input]") {
  const auto specification_dir = stdfs::path(SIGNALTL_TESTS_DIR) / "formulas";
  if (!stdfs::exists(specification_dir) || !stdfs::is_directory(specification_dir)) {
    FAIL("Directory with testing formulas doesn't exist: " << specification_dir);
  }

  for (auto& p : stdfs::directory_iterator(specification_dir)) {
    if (stdfs::is_regular_file(p)) {
      SECTION(std::string("Parsing file: ") + std::string(p.path())) {
        REQUIRE_NOTHROW(signal_tl::parser::from_file(p));
      }
    }
  }
}
