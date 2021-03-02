#include "argus/ast/expression.hpp"

#include "custom_generators.hpp"
#include <catch2/catch.hpp>
#include <magic_enum.hpp>

#include <stdexcept>
#include <string>

using argus::Expr;
using argus::ExprPtr;
using namespace argus::ast::details;

TEST_CASE("Formulas are formatted correctly", "[ast][fmt]") {
  SECTION("Constants") {
    constexpr auto bound = 20ll;
    auto i               = GENERATE(take(50, random(-bound, bound)));
    REQUIRE(Expr::Constant(i)->to_string() == std::to_string(i));
  }

  SECTION("Variables") {
    std::string str = GENERATE(take(50, random_str(5)));
    auto type = magic_enum::enum_cast<Variable::Type>(GENERATE(take(50, random(0, 4))));

    if (type.has_value()) {
      REQUIRE(Expr::Variable(str, *type)->to_string() == str);
    }
  }

  SECTION("Parameters") {
    std::string str = GENERATE(take(50, random_str(5)));
    auto type =
        magic_enum::enum_cast<Parameter::Type>(GENERATE(take(50, random(0, 4))));

    if (type.has_value()) {
      REQUIRE(Expr::Parameter(str, *type)->to_string() == str);
    }
  }
}

SCENARIO("Incorrectly structured expressions") {
  WHEN("variables are given to a logical operation") {
    ExprPtr var_x = Expr::Variable<double>("x");
    ExprPtr var_y = Expr::Variable<double>("y");
    REQUIRE(var_x != nullptr);
    REQUIRE(var_y != nullptr);
    THEN("the constructor must throw") {
      REQUIRE_THROWS_AS(Expr::And({var_x, var_y}), std::invalid_argument);
    }
  }
}
