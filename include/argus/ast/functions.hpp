/// @file     ast/details/functions.hpp
/// @brief    Generic function AST definitions.
///
/// Here, we define a generic AST node for functions, which can have arbitrary number of
/// arguments and attributes that change the semantics of the function. In the context
/// of STL, involves functions on the following data types: Constants, Variables,  and
/// other Functions. Functions do not involve the usual Logical operators, as they have
/// special meaning in our semantics, nor do they include Predicates defined by
/// relational operations (as they are the smallest non-Constant boolean expressions),
/// but they do include arithmetic operations (`+, -, /, *`), and any other mathematical
/// operations supported by the semantics.

#pragma once
#ifndef ARGUS_AST_DETAILS_FUNCTIONS
#define ARGUS_AST_DETAILS_FUNCTIONS

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "argus/ast/ast_fwd.hpp"
#include "argus/ast/attributes.hpp"

namespace argus::ast::details {

/// @brief Functions on `Constant`s, `Variable`s, and other `Function`s.
///
/// This mainly refers to mathematical functions (excluding logical operations).
struct Function {
  enum struct Type { Add, Sub, Mul, Div, Custom };

  Type fn;
  std::optional<std::string> custom_fn;
  std::vector<ExprPtr> args;

  std::set<Attribute, Attribute::KeyCompare> attrs;

  Function(
      Type op,
      std::optional<std::string> op_str,
      std::vector<ExprPtr> operands,
      std::set<Attribute, Attribute::KeyCompare> attributes);

  Function(Type op, std::vector<ExprPtr> operands) :
      Function{op, std::nullopt, std::move(operands), {}} {}

  Function(
      Type op,
      std::vector<ExprPtr> operands,
      std::set<Attribute, Attribute::KeyCompare> attributes) :
      Function{op, std::nullopt, std::move(operands), std::move(attributes)} {}

  Function(
      std::string op,
      std::vector<ExprPtr> operands,
      std::set<Attribute, Attribute::KeyCompare> attributes) :
      Function{
          Type::Custom,
          std::move(op),
          std::move(operands),
          std::move(attributes)} {}

  [[nodiscard]] std::string to_string() const;
};

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_DETAILS_FUNCTIONS */
