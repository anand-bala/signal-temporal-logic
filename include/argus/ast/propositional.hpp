/// @file     ast/details/propositional.hpp
/// @brief    Specialized AST nodes for propositional logic.
///
/// Here, we will define the specialized AST nodes for propositional logic operations,
/// including universal and existential quantifiers. Here, we will also define
/// Predicates (using relational operations) as they are the first non-constant boolean
/// expressions.

#pragma once
#ifndef ARGUS_AST_DETAILS_PROPOSITIONAL
#define ARGUS_AST_DETAILS_PROPOSITIONAL

#include <memory>
#include <vector>

#include "argus/ast/ast_fwd.hpp"

namespace ARGUS_AST_NS {

/// @brief AST node for relational operations/predicates
///
/// @note
/// Predicates can have at most 2 arguments: the LHS and the RHS. Each of these must be
/// either a Constant, a Variable, or a Function.
struct PredicateOp {
  enum struct Type { LE, LT, GE, GT, EQ, NE };

  Type op;
  ExprPtr lhs, rhs;

  PredicateOp(Type cmp, ExprPtr arg1, ExprPtr arg2);

  [[nodiscard]] std::string to_string() const;
};

/// @brief Generic AST node for all propositional operations.
///
/// @note
/// The argument to a LogicalOp must be either a Predicate, a TemporalOp or another
/// LogicalOp. Specifically, we can't use Functions here.
struct LogicalOp {
  enum struct Type { Not, And, Or };

  Type op;
  std::vector<ExprPtr> args;

  LogicalOp(Type operation, std::vector<ExprPtr> operands);

  [[nodiscard]] std::string to_string() const;
};

} // namespace ARGUS_AST_NS

#endif /* end of include guard: ARGUS_AST_DETAILS_PROPOSITIONAL */
