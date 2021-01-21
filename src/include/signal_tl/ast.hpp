#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_AST_HPP
#define SIGNAL_TEMPORAL_LOGIC_AST_HPP

#include <cmath>       // for isinf
#include <limits>      // for numeric_limits
#include <memory>      // for shared_ptr
#include <stdexcept>   // for invalid_argument
#include <string>      // for string, operator==, basic_string
#include <type_traits> // for remove_reference<>::type
#include <utility>     // for move, make_pair, pair
#include <variant>     // for get, get_if, visit, variant
#include <vector>      // for vector

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

/// Boolean constant AST nodes.
///
/// Used to represent `true` or `false` values efficiently.
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

/// Valid comparison operations within a predicate.
///
/// NOTE(anand): I personally think it never makes sense to check if some
/// signal value is _exactly_ equal to some value, as that is impossible with
/// real-valued signals.
enum class ComparisonOp { GT, GE, LT, LE };

/// A Predicate AST node.
///
/// It simply holds the expression `x ~ c`, where `x` is some signal identifier, `~` is
/// a valid comparison operator, and `c` is some constant (double).
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

/// A valid expression is one of the following:
///
/// - A Boolean constant;
/// - A Predicate expression;
/// - A unary Not, or n-ary And/Or; and
/// - The temporal operators, Always, Eventually, and Until.
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
using IntervalType = std::pair<double, double>;

/// A simple interval type for temporal operators.
///
/// Currently, it can only encode a pair of numbers (either `unsigned long long
/// int` or `double`), and enforced that any `double`s need to be positive at
/// runtime.
///
/// TODO
/// ----
///
/// - Support open and closed intervals, along with interval operations. Maybe
///   we will have to move this to a separate header file like `Signal`.
struct Interval {
  using Num = std::variant<unsigned long long int, double>;

  Num low;
  Num high;

  Interval() : low{0.0}, high{std::numeric_limits<double>::infinity()} {};
  Interval(unsigned long long int a, unsigned long long int b) : low{a}, high{b} {}
  Interval(double a, double b) : low{a}, high{b} {
    if (a < 0 || b < 0) {
      throw std::invalid_argument("Interval cannot have negative values");
    } else if (b <= a) {
      throw std::invalid_argument("Interval [a,b] cannot have b <= a");
    }
  }

  /// Return the (low, high) pair as `double`s.
  [[nodiscard]] std::pair<double, double> as_double() const {
    double a = 0.0, b = 0.0;
    if (auto a_pval = std::get_if<unsigned long long int>(&this->low)) {
      a = static_cast<double>(*a_pval);
    } else {
      a = std::get<double>(this->low);
    }

    if (auto b_pval = std::get_if<unsigned long long int>(&this->high)) {
      b = static_cast<double>(*b_pval);
    } else {
      b = std::get<double>(this->high);
    }
    return {a, b};
  }

  /// Check if the interval is [0, inf).
  [[nodiscard]] bool is_zero_to_inf() const {
    bool start_zero = std::visit([](auto&& start) { return start == 0; }, this->low);
    bool end_inf = std::visit([](auto&& end) { return std::isinf(end); }, this->high);
    return start_zero && end_inf;
  }

  /// Check if the interval is [0, inf).
  ///
  /// NOTE: Purely for backwards compatibility to a version of signal_tl that
  /// dealt with intervals as `std::pair<double, double>`, and the interval was
  /// stored inside an `std::optional` field in the Temporal operators.
  [[nodiscard]] bool has_value() const {
    return this->is_zero_to_inf();
  }
};

struct Always {
  Expr arg;
  Interval interval;

  // Always() = delete;
  Always(Expr operand) : arg{std::move(operand)}, interval{} {}
  Always(Expr operand, Interval time_interval) :
      arg(std::move(operand)), interval(time_interval) {}
};

struct Eventually {
  Expr arg;
  Interval interval;

  // Eventually() = delete;
  Eventually(Expr operand) : arg{std::move(operand)}, interval{} {}
  Eventually(Expr operand, Interval time_interval) :
      arg(std::move(operand)), interval(time_interval) {}
};

struct Until {
  std::pair<Expr, Expr> args;
  Interval interval;

  // Until() = delete;
  Until(Expr arg0, Expr arg1) :
      args{std::make_pair(std::move(arg0), std::move(arg1))}, interval{} {}
  Until(Expr arg0, Expr arg1, Interval time_interval) :
      args{std::make_pair(std::move(arg0), std::move(arg1))}, interval{time_interval} {}
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

// These are helper functions to automatically wrap the correct ast Node into a
// `std::shared_ptr` and then return that as an `Expr`.
ast::Const Const(bool value);
ast::Predicate Predicate(std::string name);
Expr Not(Expr arg);
Expr And(std::vector<Expr> args);
Expr Or(std::vector<Expr> args);
Expr Implies(Expr arg1, Expr arg2);
Expr Xor(const Expr& arg1, const Expr& arg2);
Expr Iff(const Expr& arg1, const Expr& arg2);
Expr Always(Expr arg);
Expr Always(Expr arg, ast::Interval interval);
Expr Eventually(Expr arg);
Expr Eventually(Expr arg, ast::Interval interval);
Expr Until(Expr arg1, Expr arg2);
Expr Until(Expr arg1, Expr arg2, ast::Interval interval);

} // namespace signal_tl

#endif
