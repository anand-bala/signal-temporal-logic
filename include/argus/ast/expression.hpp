/// @file     argus/ast/expression.hpp
/// @brief    Recursive definition of an AST node.

#pragma once
#ifndef ARGUS_AST_EXPRESSION
#define ARGUS_AST_EXPRESSION

#include "argus/ast/ast_fwd.hpp"

// IWYU pragma: begin_exports
#include "argus/ast/atoms.hpp"
#include "argus/ast/attributes.hpp"
#include "argus/ast/functions.hpp"
#include "argus/ast/propositional.hpp"
#include "argus/ast/temporal.hpp"
// IWYU pragma: end_exports

#include <initializer_list>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace argus {
namespace ast {

using VarType     = details::Variable::Type;
using ParamType   = details::Parameter::Type;
using FnType      = details::Function::Type;
using CmpOp       = details::PredicateOp::Type;
using LogicOpType = details::LogicalOp::Type;
using ModalOpType = details::TemporalOp::Type;

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
  template <typename CType>
  static std::unique_ptr<Expr> Constant(CType constant);

  /// @brief Create a variable with a known type.
  static std::unique_ptr<Expr> Variable(std::string name, ast::VarType type);

  /// @brief Create a parameter with a known type.
  static std::unique_ptr<Expr> Parameter(std::string name, ast::ParamType type);

  /// @brief Create a pre-defined function, with given arguments and attributes
  static std::unique_ptr<Expr> Function(
      ast::FnType op,
      std::vector<ExprPtr> args,
      std::set<Attribute, Attribute::KeyCompare> attrs);

  /// @brief Create a custom function, with given arguments and attributes
  static std::unique_ptr<Expr> Function(
      std::string op,
      std::vector<ExprPtr> args,
      std::set<Attribute, Attribute::KeyCompare> attrs);

  /// @brief Create an Addition AST
  static std::unique_ptr<Expr> Add(std::vector<ExprPtr> args);

  /// @brief Create an Multiplication AST
  static std::unique_ptr<Expr> Mul(std::vector<ExprPtr> args);

  /// @brief Create an Subtraction AST
  static std::unique_ptr<Expr> Subtract(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create an Division AST
  static std::unique_ptr<Expr> Div(ExprPtr numerator, ExprPtr denominator);

  /// @brief Create a Equality predicate
  static std::unique_ptr<Expr> Eq(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Not-Equality predicate
  static std::unique_ptr<Expr> Neq(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Less Than predicate
  static std::unique_ptr<Expr> Lt(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Less Than or Equal predicate
  static std::unique_ptr<Expr> Le(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Greater Than predicate
  static std::unique_ptr<Expr> Gt(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Greater Than or Equal predicate
  static std::unique_ptr<Expr> Ge(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Negation
  static std::unique_ptr<Expr> Not(ExprPtr arg);

  /// @brief Create a Logical And operation
  static std::unique_ptr<Expr> And(std::vector<ExprPtr> arg);

  /// @brief Create a Logical Or operation
  static std::unique_ptr<Expr> Or(std::vector<ExprPtr> arg);

  /// @brief Create a Logical Implication
  static std::unique_ptr<Expr> Implies(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Implication
  static std::unique_ptr<Expr> Xor(ExprPtr lhs, ExprPtr rhs);

  /// @brief Create a Logical Implication
  static std::unique_ptr<Expr> Iff(ExprPtr lhs, ExprPtr rhs);

  /// @brief Next temporal operator
  static std::unique_ptr<Expr> Next(ExprPtr arg);

  /// @brief Previous temporal operator
  static std::unique_ptr<Expr> Previous(ExprPtr arg);

  /// @brief Eventually temporal operator
  static std::unique_ptr<Expr> Eventually(ExprPtr arg, ExprPtr interval = nullptr);

  /// @brief Once temporal operator
  static std::unique_ptr<Expr> Once(ExprPtr arg, ExprPtr interval = nullptr);

  /// @brief Always temporal operator
  static std::unique_ptr<Expr> Always(ExprPtr arg, ExprPtr interval = nullptr);

  /// @brief Historically temporal operator
  static std::unique_ptr<Expr> Historically(ExprPtr arg, ExprPtr interval = nullptr);

  /// @brief Until temporal operator
  static std::unique_ptr<Expr>
  Until(ExprPtr arg1, ExprPtr arg2, ExprPtr interval = nullptr);

  /// @brief Since temporal operator
  static std::unique_ptr<Expr>
  Since(ExprPtr arg1, ExprPtr arg2, ExprPtr interval = nullptr);

 private:
  /// @brief The unique ID for an expression
  ///
  /// The ID of the Expr is used to create look-up tables within contexts, allowing for
  /// efficient use of the
  size_t m_id;
  /// Private factory function.
  template <typename ExprType>
  static std::unique_ptr<Expr> make_expr(ExprType);
};
} // namespace argus

#endif /* end of include guard: ARGUS_AST_EXPRESSION */
