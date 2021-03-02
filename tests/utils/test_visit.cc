/*
 * Copyright Björn Fahller 2018
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/utils/visit
 */

#include <catch2/catch.hpp>
#include <variant>

#include "utils/visit.hpp"

template <typename T>
struct dubious {
  dubious(T t) : t{std::move(t)} {}
  dubious(dubious&& r) noexcept(false) : t{std::move(r.t)} {}
  dubious(const dubious& r) noexcept(false) : t{r.t} {}
  dubious& operator=(const dubious&) = default;
  operator const T&() const {
    return t;
  }
  operator T&&() && {
    return std::move(t);
  }
  T t;
};

struct evil {
  template <typename T>
  operator dubious<T>() const {
    throw std::domain_error("evil");
  }
};

SCENARIO("single variant visit") {
  GIVEN("a variant and a visitor") {
    int int_visits    = 0;
    int string_visits = 0;
    int ptr_visits    = 0;
    auto visitor      = utils::overloaded{
        [&](int v) -> int {
          ++int_visits;
          return v;
        },
        [&](const std::string& s) -> int {
          ++string_visits;
          return s.length();
        },
        [&](void*) -> int {
          ++ptr_visits;
          return 0;
        }};
    std::variant<dubious<int>, dubious<std::string>, dubious<void*>> v{3};
    WHEN("visited with an int") {
      auto r = utils::visit(visitor, v);
      THEN("the return value is that of the int") {
        REQUIRE(r == 3);
      }
      AND_THEN("only the int was visited") {
        REQUIRE(int_visits == 1);
        REQUIRE(string_visits == 0);
        REQUIRE(ptr_visits == 0);
      }
    }
    AND_WHEN("visited with a string") {
      v      = std::string("foobar");
      auto r = utils::visit(visitor, v);
      THEN("return value is the length of the string") {
        REQUIRE(r == 6);
      }
      AND_THEN("only the string was visited") {
        REQUIRE(int_visits == 0);
        REQUIRE(string_visits == 1);
        REQUIRE(ptr_visits == 0);
      }
    }
    AND_WHEN("visited when empty with exception") {
      try {
        v.emplace<dubious<int>>(evil{});
      } catch (...) {}
      THEN("visit throws bad_variant_access") {
        REQUIRE_THROWS_AS(utils::visit(visitor, v), std::bad_variant_access);
      }
    }
  }
}

SCENARIO("multi variant visit") {
  GIVEN("two variants and a visitor") {
    using V = std::variant<dubious<int>, dubious<std::string>>;

    auto visitor = utils::overloaded{
        [](int i, int j) { return std::to_string(i) + std::to_string(j); },
        [](int i, const std::string& s) { return std::to_string(i) + s; },
        [](const std::string& s, int i) { return s + std::to_string(i); },
        [](const std::string& s1, const std::string& s2) {
          return s1 + s2;
        }};
    WHEN("visited with values") {
      V v1{3};
      V v2{std::string("foo")};
      auto r1 = utils::visit(visitor, v1, v2);
      auto r2 = utils::visit(visitor, v2, v1);
      v1      = std::string("bar");
      auto r3 = utils::visit(visitor, v1, v2);
      v1      = 3;
      v2      = 4;
      auto r4 = utils::visit(visitor, v1, v2);
      THEN("the return value comes from the visitor of the value pair") {
        REQUIRE(r1 == "3foo");
        REQUIRE(r2 == "foo3");
        REQUIRE(r3 == "barfoo");
        REQUIRE(r4 == "34");
      }
    }
    AND_WHEN("visited when empty with exception") {
      V v1{3};
      V v2{4};
      try {
        v1.emplace<dubious<int>>(evil{});
      } catch (...) {}
      THEN("visit throws bad_variant_access") {
        REQUIRE_THROWS_AS(utils::visit(visitor, v1, v2), std::bad_variant_access);
        REQUIRE_THROWS_AS(utils::visit(visitor, v2, v1), std::bad_variant_access);
      }
    }
  }
  AND_GIVEN("a type inherited from variant, a variant and a visitor") {
    struct S : public std::variant<int, std::string> {
      using std::variant<int, std::string>::variant;
    };
    using V = std::variant<int, std::string>;

    auto visitor = utils::overloaded{
        [](int i, int j) { return std::to_string(i) + std::to_string(j); },
        [](int i, const std::string& s) { return std::to_string(i) + s; },
        [](const std::string& s, int i) { return s + std::to_string(i); },
        [](const std::string& s1, const std::string& s2) {
          return s1 + s2;
        }};
    WHEN("visited with values") {
      V v{3};
      S s{std::string("foo")};
      auto r1 = utils::visit(visitor, v, s);
      auto r2 = utils::visit(visitor, s, v);
      v       = std::string("bar");
      auto r3 = utils::visit(visitor, v, s);
      v       = 3;
      s       = 4;
      auto r4 = utils::visit(visitor, v, s);
      THEN("the return value comes from the visitor of the value pair") {
        REQUIRE(r1 == "3foo");
        REQUIRE(r2 == "foo3");
        REQUIRE(r3 == "barfoo");
        REQUIRE(r4 == "34");
      }
    }
  }
}

SCENARIO("mixed types visit") {
  GIVEN("two variants, a unique_ptr and a visitor") {
    using up     = std::unique_ptr<int>;
    auto visitor = utils::overloaded{
        [](int&& i, up p, std::string&& s) {
          return std::to_string(i) + std::to_string(*p) + s;
        },
        [](std::string&& s, up p, int&& i) {
          return s + std::to_string(*p) + std::to_string(i);
        },
        [](std::string&& s, up p, std::string&& s2) {
          return s + std::to_string(*p) + s2;
        },
        [](int i1, up p, int i2) {
          return std::to_string(i1) + std::to_string(*p) + std::to_string(i2);
        }};
    using V = std::variant<dubious<int>, dubious<std::string>>;

    WHEN("visited with the variants and unique_ptr") {
      auto r1 = utils::visit(visitor, V{1}, std::make_unique<int>(3), V{2});
      std::string s("s");
      auto r2 = utils::visit(visitor, V{s}, std::make_unique<int>(3), V{0});
      THEN("they are perfect forwarded to the visitor") {
        REQUIRE(r1 == "132");
        REQUIRE(r2 == "s30");
      }
    }
    AND_WHEN("either variant is valueless by exception") {
      V v{0};
      try {
        v.emplace<dubious<int>>(evil{});
      } catch (...) {}
      THEN("visit throws bad_variant_access") {
        REQUIRE_THROWS_AS(
            utils::visit(visitor, V{3}, nullptr, std::move(v)),
            std::bad_variant_access);
        REQUIRE_THROWS_AS(
            utils::visit(visitor, std::move(v), nullptr, V{3}),
            std::bad_variant_access);
      }
    }
  }
}
