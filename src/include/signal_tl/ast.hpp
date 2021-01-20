#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_AST_HPP
#define SIGNAL_TEMPORAL_LOGIC_AST_HPP

#include <memory>
#include <optional>
#include <stdexcept> // for invalid_argument
#include <string>
#include <type_traits> // for remove_reference<>::type
#include <utility>
#include <variant>
#include <vector>

namespace signal_tl {
namespace ast {

struct Const;
struct Predicate;
struct Not;
struct And;
struct Or;
struct Always;
struct Eventually;
struct Until;

using ConstPtr      = std::shared_ptr<Const>;
using PredicatePtr  = std::shared_ptr<Predicate>;
using NotPtr        = std::shared_ptr<Not>;
using AndPtr        = std::shared_ptr<And>;
using OrPtr         = std::shared_ptr<Or>;
using AlwaysPtr     = std::shared_ptr<Always>;
using EventuallyPtr = std::shared_ptr<Eventually>;
using UntilPtr      = std::shared_ptr<Until>;

/* Define Syntax Tree */

struct Const {
  bool value = false;

  // TODO: pybind11 doesn't like deleted default constructors
  Const() = default;
  Const(bool boolean_value) : value{boolean_value} {};

  inline bool operator==(const Const& other) const {
    return this->value == other.value;
  };

  inline bool operator!=(const Const& other) const {
    return !(*this == other);
  };
};

enum class ComparisonOp { GT, GE, LT, LE };

struct Predicate {
  std::string name;
  ComparisonOp op = ComparisonOp::GE;
  double rhs      = 0.0;

  // Predicate() = delete;
  Predicate(
      std::string ap_name,
      ComparisonOp operation = ComparisonOp::GE,
      double constant_val    = 0.0) :
      name{std::move(ap_name)}, op{operation}, rhs{constant_val} {};

  inline bool operator==(const Predicate& other) const {
    return (name == other.name) && (op == other.op) && (rhs == other.rhs);
  };

  inline bool operator!=(const Predicate& other) const {
    return !(*this == other);
  };
};

using Expr = std::variant<
    Const,
    Predicate,
    NotPtr,
    AndPtr,
    OrPtr,
    AlwaysPtr,
    EventuallyPtr,
    UntilPtr>;
using ExprPtr = std::shared_ptr<Expr>;

struct Not {
  Expr arg;

  // Not() = delete;
  Not(Expr sub_expr) : arg(std::move(sub_expr)) {}
};

struct And {
  std::vector<Expr> args;

  // And() = delete;
  And(std::vector<Expr> operands) : args(std::move(operands)) {
    if (args.size() < 2) {
      throw std::invalid_argument(
          "It doesn't make sense to have an And operator with < 2 operands");
    }
  }
};

struct Or {
  std::vector<Expr> args;

  // Or() = delete;
  Or(std::vector<Expr> operands) : args(std::move(operands)) {
    if (args.size() < 2) {
      throw std::invalid_argument(
          "It doesn't make sense to have an Or operator with < 2 operands");
    }
  }
};

/* Modal Logic */
using Interval = std::pair<double, double>;

struct Always {
  Expr arg;
  std::optional<Interval> interval;

  // Always() = delete;
  Always(Expr operand, const std::optional<Interval> time_interval = std::nullopt) :
      arg(std::move(operand)), interval(time_interval) {
    if (interval.has_value()) {
      const auto [a, b] = interval.value();
      if (a < 0 || b < 0) {
        throw std::invalid_argument("Interval cannot have negative values");
      } else if (b <= a) {
        throw std::invalid_argument("Interval [a,b] cannot have b <= a");
      }
    }
  }
};

struct Eventually {
  Expr arg;
  std::optional<Interval> interval;

  // Eventually() = delete;
  Eventually(Expr operand, const std::optional<Interval> time_interval = std::nullopt) :
      arg(std::move(operand)), interval(time_interval) {
    if (interval.has_value()) {
      const auto [a, b] = interval.value();
      if (a < 0 || b < 0) {
        throw std::invalid_argument("Interval cannot have negative values");
      } else if (b <= a) {
        throw std::invalid_argument("Interval [a,b] cannot have b <= a");
      }
    }
  }
};

struct Until {
  std::pair<Expr, Expr> args;
  std::optional<Interval> interval;

  // Until() = delete;
  Until(Expr arg0, Expr arg1, std::optional<Interval> time_interval = std::nullopt) :
      args(std::make_pair(std::move(arg0), std::move(arg1))),
      interval(std::move(time_interval)) {
    if (interval.has_value()) {
      const auto [a, b] = interval.value();
      if (a < 0 || b < 0) {
        throw std::invalid_argument("Interval cannot have negative values");
      } else if (b <= a) {
        throw std::invalid_argument("Interval [a,b] cannot have b <= a");
      }
    }
  }
};

Predicate operator>(const Predicate& lhs, const double bound);
Predicate operator>=(const Predicate& lhs, const double bound);
Predicate operator<(const Predicate& lhs, const double bound);
Predicate operator<=(const Predicate& lhs, const double bound);

Expr operator~(const Expr& e);
Expr operator&(const Expr& lhs, const Expr& rhs);
Expr operator|(const Expr& lhs, const Expr& rhs);
Expr operator>>(const Expr& lhs, const Expr& rhs);

} // namespace ast

using ast::Expr;

ast::Const Const(bool value);
ast::Predicate Predicate(std::string name);
Expr Not(Expr arg);
Expr And(std::vector<Expr> args);
Expr Or(std::vector<Expr> args);
Expr Implies(Expr arg1, Expr arg2);
Expr Xor(const Expr& arg1, const Expr& arg2);
Expr Iff(const Expr& arg1, const Expr& arg2);
Expr Always(Expr arg, std::optional<ast::Interval> interval = std::nullopt);
Expr Eventually(Expr arg, std::optional<ast::Interval> interval = std::nullopt);
Expr Until(Expr arg1, Expr arg2, std::optional<ast::Interval> interval = std::nullopt);

} // namespace signal_tl

#endif
