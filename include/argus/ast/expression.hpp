/// @file     argus/ast/expression.hpp
/// @brief    Recursive definition of an AST node.

#pragma once
#ifndef ARGUS_AST_EXPRESSION
#define ARGUS_AST_EXPRESSION

// IWYU pragma: begin_exports
#include "argus/ast/ast_fwd.hpp"       // for ExprPtr, PrimitiveTypes
#include "argus/ast/atoms.hpp"         // for Constant (ptr only), Parameter
#include "argus/ast/attributes.hpp"    // for Attribute
#include "argus/ast/functions.hpp"     // for Function, Function::Type
#include "argus/ast/propositional.hpp" // for LogicalOp, LogicalOp::Type
#include "argus/ast/temporal.hpp"      // for Interval (ptr only), TemporalOp
// IWYU pragma: end_exports

#include <cstddef>   // for size_t
#include <memory>    // for shared_ptr
#include <set>       // for set
#include <stdexcept> // for invalid_argument
#include <string>    // for string
#include <utility>   // for move
#include <variant>   // for variant
#include <vector>    // for vector

namespace argus {
namespace ast {

using VarType     = details::Variable::Type;
using ParamType   = details::Parameter::Type;
using FnType      = details::Function::Type;
using CmpOp       = details::PredicateOp::Type;
using LogicOpType = details::LogicalOp::Type;
using ModalOpType = details::TemporalOp::Type;

using details::Attribute;
using details::Interval;

using ExprTypes = std::variant<
    details::Constant,
    details::Variable,
    details::Parameter,
    details::Function,
    details::PredicateOp,
    details::LogicalOp,
    details::TemporalOp>;
} // namespace ast

/// @brief The overarching expression type.
///
/// An expression must be created using the static factory methods given in this struct.
/// This allows for the library to assign the Expr with IDs that allow for efficient
/// look-up table implementations.
///
/// @note
/// We chose to not use a manager class that takes care of this as we can implement this
/// functionality using static thread-safe atomics. But we may change this in future
/// implementations of the library if users run into any issues.
struct Expr : ast::ExprTypes {
  using ast::ExprTypes::variant;

  /// @brief Get the ID of the Expression.
  [[nodiscard]] constexpr size_t id() const {
    return m_id;
  }

  /// @brief Check if the expression is a valid formula
  [[nodiscard]] bool is_valid() const;

  /// @brief Format the given formula as an S-Expression string.
  ///
  /// @note The output can be parsed by the parser to yield an identically structured
  /// `Expr`.
  [[nodiscard]] std::string to_string() const;

  /// @brief Create an expression with a Constant value.
  static ExprPtr Constant(ast::details::PrimitiveTypes constant);

  /// @brief Create a variable with a known type.
  static ExprPtr Variable(std::string name, ast::VarType type);

  /// @brief Create a variable with a known type.
  template <typename T>
  static ExprPtr Variable(std::string name) {
    auto type = ast::VarType::Real;
    if constexpr (std::is_same_v<T, bool>) {
      type = ast::VarType::Bool;
    } else if constexpr (std::is_integral_v<T>) {
      if constexpr (std::is_unsigned_v<T>) {
        type = ast::VarType::UInt;
      } else {
        type = ast::VarType::Int;
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      type = ast::VarType::Real;
    } else {
      throw std::invalid_argument("Cannot create variable of given type");
    }
    return Variable(std::move(name), type);
  }

  /// @brief Create a parameter with a known type.
  static ExprPtr Parameter(std::string name, ast::ParamType type);

  /// @brief Create a parameter with a known type.
  template <typename T>
  static ExprPtr Parameter(std::string name) {
    auto type = ast::ParamType::Real;
    if constexpr (std::is_same_v<T, bool>) {
      type = ast::ParamType::Bool;
    } else if constexpr (std::is_integral_v<T>) {
      if constexpr (std::is_unsigned_v<T>) {
        type = ast::ParamType::UInt;
      } else {
        type = ast::ParamType::Int;
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      type = ast::ParamType::Real;
    } else {
      throw std::invalid_argument("Cannot create parameter of given type");
    }
    return Parameter(std::move(name), type);
  }

  /// @brief Create a pre-defined function, with given arguments and attributes
  static ExprPtr Function(
      ast::FnType op,
      std::vector<ExprPtr> args,
      std::set<ast::Attribute, ast::Attribute::KeyCompare> attrs);

  /// @brief Create a custom function, with given arguments and attributes
  static ExprPtr Function(
      std::string op,
      std::vector<ExprPtr> args,
      std::set<ast::Attribute, ast::Attribute::KeyCompare> attrs);

  /// @brief Create an Addition AST
  static ExprPtr Add(std::vector<ExprPtr> args);

  /// @brief Create an Multiplication AST
  static ExprPtr Mul(std::vector<ExprPtr> args);

  /// @brief Create an Subtraction AST
  static ExprPtr Subtract(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create an Division AST
  static ExprPtr Div(ExprPtr numerator, ExprPtr denominator);

  /// @brief Create a Equality predicate
  static ExprPtr Eq(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Not-Equality predicate
  static ExprPtr Neq(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Less Than predicate
  static ExprPtr Lt(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Less Than or Equal predicate
  static ExprPtr Le(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Greater Than predicate
  static ExprPtr Gt(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Greater Than or Equal predicate
  static ExprPtr Ge(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Negation
  static ExprPtr Not(ExprPtr arg);

  /// @brief Create a Logical And operation
  static ExprPtr And(std::vector<ExprPtr> arg);

  /// @brief Create a Logical Or operation
  static ExprPtr Or(std::vector<ExprPtr> arg);

  /// @brief Create a Logical Implication
  static ExprPtr Implies(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Implication
  static ExprPtr Xor(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Implication
  static ExprPtr Iff(ExprPtr lhs, ExprPtr rhs);

  /// @brief Next temporal operator
  static ExprPtr Next(ExprPtr arg);

  /// @brief Previous temporal operator
  static ExprPtr Previous(ExprPtr arg);

  /// @brief Eventually temporal operator
  static ExprPtr Eventually(ExprPtr arg);
  /// @brief Eventually temporal oprator with interval
  static ExprPtr
  Eventually(ExprPtr arg, std::shared_ptr<ast::details::Interval> interval);

  /// @brief Once temporal operator
  static ExprPtr Once(ExprPtr arg);
  /// @brief Once temporal operator with interval
  static ExprPtr Once(ExprPtr arg, std::shared_ptr<ast::details::Interval> interval);

  /// @brief Always temporal operator
  static ExprPtr Always(ExprPtr arg);
  /// @brief Always temporal operator with interval
  static ExprPtr Always(ExprPtr arg, std::shared_ptr<ast::details::Interval> interval);

  /// @brief Historically temporal operator
  static ExprPtr Historically(ExprPtr arg);
  /// @brief Historically temporal oprator with interval
  static ExprPtr
  Historically(ExprPtr arg, std::shared_ptr<ast::details::Interval> interval);

  /// @brief Until temporal operator
  static ExprPtr Until(ExprPtr arg1, ExprPtr arg2);
  /// @brief Until temporal operator with interval
  static ExprPtr
  Until(ExprPtr arg1, ExprPtr arg2, std::shared_ptr<ast::details::Interval> interval);

  /// @brief Since temporal operator
  static ExprPtr Since(ExprPtr arg1, ExprPtr arg2);
  /// @brief Since temporal operator with interval
  static ExprPtr
  Since(ExprPtr arg1, ExprPtr arg2, std::shared_ptr<ast::details::Interval> interval);

 private:
  /// @brief The unique ID for an expression
  ///
  /// The ID of the Expr is used to create look-up tables within contexts, allowing for
  /// efficient use of the
  size_t m_id;
  /// Private factory function.
  static ExprPtr make_expr(Expr&&);
};
} // namespace argus

#endif /* end of include guard: ARGUS_AST_EXPRESSION */
