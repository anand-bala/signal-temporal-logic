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

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Forward-declare Expr
namespace argus {
struct Expr;
} // namespace argus

namespace argus::ast::details {

/// @brief AST node for relational operations/predicates
///
/// @note
/// Predicates can have at most 2 arguments: the LHS and the RHS. Each of these must be
/// either a Constant, a Variable, or a Function. This is checked by the semantics.
struct PredicateOp {
  enum struct Type { LE, LT, GE, GT, EQ, NE };

  Type op;
  std::shared_ptr<Expr> lhs, rhs;
};

/// @brief Generic AST node for all propositional operations.
struct LogicalOp {
  enum struct Type { Not, And, Or };

  Type op;
  std::vector<std::shared_ptr<Expr>> args;
};

/// @brief Quantifier expressions
struct QuantifierOp {
  enum struct Type { Exists, Forall };

  Type op;
  std::vector<std::shared_ptr<Expr>> vars;
  std::shared_ptr<Expr> arg;
};

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_DETAILS_PROPOSITIONAL */
