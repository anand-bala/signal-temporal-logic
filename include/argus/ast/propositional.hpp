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
/// either a Constant, a Variable, or a Function. This is checked by the semantics.
struct PredicateOp {
  enum struct Type { LE, LT, GE, GT, EQ, NE };

  Type op;
  ExprPtr lhs, rhs;

  PredicateOp(Type cmp, ExprPtr arg1, ExprPtr arg2) : op{cmp}, lhs{std::move(arg1)}, rhs{std::move(arg2)} {}

};

/// @brief Generic AST node for all propositional operations.
struct LogicalOp {
  enum struct Type { Not, And, Or };

  Type op;
  std::vector<ExprPtr> args;

  LogicalOp(Type operation, std::vector<ExprPtr> operands) : op{operation}, args{std::move(operands)} {}
};

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_DETAILS_PROPOSITIONAL */
