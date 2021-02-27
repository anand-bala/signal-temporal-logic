/// @file     argus/ast/temporal.hpp
/// @brief    AST nodes for temporal operators.
#pragma once
#ifndef PERCEMON_AST_DETAILS_TEMPORAL
#define PERCEMON_AST_DETAILS_TEMPORAL

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "argus/ast/ast_fwd.hpp"

namespace ARGUS_AST_NS {

/// @brief Interval type.
///
/// @note
/// An interval can only hold Constants or Parameters for the lower and upper bounds.
/// This is asserted at construction time.
struct Interval {
  ExprPtr low;
  ExprPtr high;

  Interval() = default;

  Interval(ExprPtr _low, ExprPtr _high);
  template <typename L, typename H>
  Interval(L _low, H _high);

  [[nodiscard]] std::string to_string() const;
};

/// @brief Generic AST node for temporal operators
struct TemporalOp {
  enum struct Type {
    Next,
    Previous,
    Eventually,
    Once,
    Always,
    Historically,
    Until,
    Since,
  };

  Type op;
  std::vector<ExprPtr> args; // Has max 2 arguments.
  std::shared_ptr<Interval> interval;

  TemporalOp(
      Type operation,
      std::vector<ExprPtr> arguments,
      std::shared_ptr<Interval> interval_arg);

  TemporalOp(Type operation, std::vector<ExprPtr> arguments) :
      TemporalOp{operation, std::move(arguments), {}} {}

  [[nodiscard]] std::string to_string() const;
};
} // namespace ARGUS_AST_NS

#endif /* end of include guard: PERCEMON_AST_DETAILS_TEMPORAL */
