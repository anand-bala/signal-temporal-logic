#pragma once

#if !defined(__SIGNAL_TEMPORAL_LOGIC_AST_HH__)
#define __SIGNAL_TEMPORAL_LOGIC_AST_HH__

#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace signal_tl {
namespace ast {

/* Define Syntax Tree */

struct Const;
using ConstPtr = std::shared_ptr<Const>;
struct Predicate;
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
    ConstPtr,
    PredicatePtr,
    NotPtr,
    AndPtr,
    OrPtr,
    AlwaysPtr,
    EventuallyPtr,
    UntilPtr>;
using ExprPtr = std::shared_ptr<Expr>;
std::ostream& operator<<(std::ostream& out, const Expr& expr);

/* Atomic Predicates and Constants */

struct Const {
  const bool value;

  Const() = delete;
  Const(bool value) : value(value) {}

  friend std::ostream& operator<<(std::ostream& out, const Const& expr) {
    return out << std::boolalpha << expr.value;
  }

  static Expr as_expr(bool value) {
    return Expr(std::make_shared<Const>(value));
  };
};

struct Predicate {
  const std::string name;

  Predicate() = delete;
  Predicate(const std::string& name) : name(name) {}

  friend std::ostream& operator<<(std::ostream& out, const Predicate& expr) {
    return out << expr.name;
  }

  static Expr as_expr(const std::string& name) {
    return Expr(std::make_shared<Predicate>(name));
  };
};

/* Propositional Logic */

struct Not {
  const Expr arg;

  Not() = delete;
  Not(const Expr& arg) : arg(arg) {}

  friend std::ostream& operator<<(std::ostream& out, const Not& expr) {
    return out << "~" << expr.arg;
  }

  static Expr as_expr(const Expr& arg) {
    if (auto cval = std::get_if<ConstPtr>(&arg)) {
      return Expr(std::make_shared<Const>(!(*cval)->value));
    } else if (auto not_ = std::get_if<NotPtr>(&arg)) {
      return Expr((*not_)->arg);
    }
    return Expr(std::make_shared<Not>(arg));
  };
};

struct And {
  const std::vector<Expr> args;

  And() = delete;
  And(const std::vector<Expr>& args) : args(args) {
    if (args.size() < 2) {
      std::invalid_argument(
          "It doesn't make sense to have an And operator with < 2 operands");
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const And& expr) {
    out << "(";
    for (size_t i = 0; i < expr.args.size(); i++) {
      if (i != 0)
        out << " & ";
      out << expr.args[i];
    }
    out << ")";
    return out;
  }

  static Expr as_expr(const std::vector<Expr>& args) {
    if (args.size() == 1) {
      return args[0];
    }
    return Expr(std::make_shared<And>(args));
  };
};

struct Or {
  const std::vector<Expr> args;

  Or() = delete;
  Or(const std::vector<Expr>& args) : args(args) {
    if (args.size() < 2) {
      std::invalid_argument(
          "It doesn't make sense to have an Or operator with < 2 operands");
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const Or& expr) {
    out << "(";
    for (size_t i = 0; i < expr.args.size(); i++) {
      if (i != 0)
        out << " | ";
      out << expr.args[i];
    }
    out << ")";
    return out;
  }

  static Expr as_expr(const std::vector<Expr>& args) {
    if (args.size() == 1) {
      return args[0];
    }
    return Expr(std::make_shared<Or>(args));
  };
};

/* Modal Logic */
using Interval = std::pair<double, double>;

struct Always {
  const Expr arg;
  const std::optional<Interval> interval;

  Always() = delete;
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

  friend std::ostream& operator<<(std::ostream& out, const Always& expr) {
    if (expr.interval.has_value()) {
      const auto [a, b] = expr.interval.value();
      if (std::isinf(b)) {
        return out << "G " << expr.arg;
      }
      return out << "G[" << a << "," << b << "] " << expr.arg;
    }
    return out << "G " << expr.arg;
  }

  static Expr
  as_expr(const Expr& arg, const std::optional<Interval> interval = std::nullopt) {
    return Expr(std::make_shared<Always>(arg, interval));
  };
};

struct Eventually {
  const Expr arg;
  const std::optional<Interval> interval;

  Eventually() = delete;
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

  friend std::ostream& operator<<(std::ostream& out, const Eventually& expr) {
    if (expr.interval.has_value()) {
      const auto [a, b] = expr.interval.value();
      if (std::isinf(b)) {
        return out << "F " << expr.arg;
      }
      return out << "F[" << a << "," << b << "] " << expr.arg;
    }
    return out << "F " << expr.arg;
  }

  static Expr
  as_expr(const Expr& arg, const std::optional<Interval> interval = std::nullopt) {
    return Expr(std::make_shared<Eventually>(arg, interval));
  };
};

struct Until {
  const std::pair<Expr, Expr> args;
  const std::optional<Interval> interval;

  Until() = delete;
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

  friend std::ostream& operator<<(std::ostream& out, const Until& expr) {
    if (expr.interval.has_value()) {
      const auto [a, b] = expr.interval.value();
      if (std::isinf(b)) {
        return out << std::get<0>(expr.args) << " U " << std::get<1>(expr.args);
      }
      return out << std::get<0>(expr.args) << " U[" << a << "," << b << "] "
                 << std::get<1>(expr.args);
    }
    return out << std::get<0>(expr.args) << " U " << std::get<1>(expr.args);
  }

  static Expr as_expr(
      const Expr& arg0,
      const Expr& arg1,
      const std::optional<Interval> interval = std::nullopt) {
    return Expr(std::make_shared<Until>(arg0, arg1, interval));
  };
};

std::ostream& operator<<(std::ostream& out, const Expr& expr);
Expr operator&(const Expr& lhs, const Expr& rhs);
Expr operator|(const Expr& lhs, const Expr& rhs);
Expr operator~(const Expr& expr);

} // namespace ast
} // namespace signal_tl
#endif
