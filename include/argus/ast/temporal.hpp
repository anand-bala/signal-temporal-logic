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
  std::array<ExprPtr, 2> args; // Has max 2 arguments.
  ExprPtr interval;

  TemporalOp(Type operation, std::array<ExprPtr, 2> arguments) :
      op{operation}, args{std::move(arguments)}, interval{nullptr} {}
  TemporalOp(Type operation, std::array<ExprPtr, 2> arguments, ExprPtr interval_arg) :
      op{operation}, args{std::move(arguments)}, interval{std::move(interval_arg)} {}
};
} // namespace PERCEMON_AST_NS

#endif /* end of include guard: PERCEMON_AST_DETAILS_TEMPORAL */

