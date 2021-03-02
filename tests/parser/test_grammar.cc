#include <catch2/catch.hpp>

#include <tao/pegtl/contrib/analyze.hpp>

#include "parser/grammar.hpp"

TEST_CASE("PEG Grammar has no issues", "[parser][grammar]") {
  /// Call `tao::pagtl::contrib::analyze`, a function that analyzes the parser grammar
  /// for construction errors like unresolved cycles, etc.
  size_t issues = tao::pegtl::analyze<argus::grammar::Specification>(/*verbose=*/1);
  INFO("Number of issues = " << issues);
  REQUIRE(issues == 0);
}
