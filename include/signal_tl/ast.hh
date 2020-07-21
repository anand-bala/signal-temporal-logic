#pragma once

#if !defined(__SIGNAL_TEMPORAL_LOGIC_AST_HH__)
#define __SIGNAL_TEMPORAL_LOGIC_AST_HH__

#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace signal_tl {
namespace ast {

/* Define Syntax Tree */

struct Const {
  bool value;

  // TODO: pybind11 doesn't like deleted default constructors
  // Const() = delete;
  Const(bool value) : value{value} {};

  inline bool operator==(const Const& other) const {
    return this->value == other.value;
  };

  inline bool operator!=(const Const& other) const {
    return !(*this == other);
  };
};
using ConstPtr = std::shared_ptr<Const>;

enum class ComparisonOp { GT, GE, LT, LE };

struct Predicate {
  std::string name;
  ComparisonOp op = ComparisonOp::GE;
  double lhs      = 0.0;

  // Predicate() = delete;
  Predicate(
      const std::string& name,
      const ComparisonOp op = ComparisonOp::GE,
      const double lhs      = 0.0) :
      name{name}, op{op}, lhs{lhs} {};

  inline bool operator==(const Predicate& other) const {
    return (name == other.name) && (op == other.op) && (lhs == other.lhs);
  };

  inline bool operator!=(const Predicate& other) const {
    return !(*this == other);
  };
};
using PredicatePtr = std::shared_ptr<Predicate>;

struct Not;
using NotPtr = std::shared_ptr<Not>;
struct And;
using AndPtr = std::shared_ptr<And>;
struct Or;
using OrPtr = std::shared_ptr<Or>;
struct Always;
using AlwaysPtr = std::shared_ptr<Always>;
struct Eventually;
using EventuallyPtr = std::shared_ptr<Eventually>;
struct Until;
using UntilPtr = std::shared_ptr<Until>;

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
  Not(const Expr& arg) : arg(arg) {}
};

struct And {
  std::vector<Expr> args;

  // And() = delete;
  And(const std::vector<Expr>& args) : args(args) {
    if (args.size() < 2) {
      throw std::invalid_argument(
          "It doesn't make sense to have an And operator with < 2 operands");
    }
  }
};

struct Or {
  std::vector<Expr> args;

  // Or() = delete;
  Or(const std::vector<Expr>& args) : args(args) {
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
  Always(const Expr& arg, const std::optional<Interval> interval = std::nullopt) :
      arg(arg), interval(interval) {
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
  Eventually(const Expr& arg, const std::optional<Interval> interval = std::nullopt) :
      arg(arg), interval(interval) {
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
  Until(
      const Expr& arg0,
      const Expr& arg1,
      std::optional<Interval> interval = std::nullopt) :
      args(std::make_pair(arg0, arg1)), interval(interval) {
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
ast::Predicate Predicate(const std::string& name);
ast::Expr Not(const ast::Expr& arg);
ast::Expr And(const std::vector<ast::Expr>& args);
ast::Expr Or(const std::vector<ast::Expr>& args);
ast::Expr Implies(const ast::Expr& arg1, const ast::Expr& arg2);
ast::Expr Always(
    const ast::Expr& arg,
    const std::optional<ast::Interval> interval = std::nullopt);
ast::Expr Eventually(
    const ast::Expr& arg,
    const std::optional<ast::Interval> interval = std::nullopt);
ast::Expr Until(
    const ast::Expr& arg1,
    const ast::Expr& arg2,
    const std::optional<ast::Interval> interval = std::nullopt);

} // namespace signal_tl

#endif
