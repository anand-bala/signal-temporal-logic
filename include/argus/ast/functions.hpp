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

// Forward-declare Expr
namespace argus {
struct Expr;
} // namespace argus

namespace argus::ast::details {

/// @brief Functions on `Constant`s, `Variable`s, and other `Function`s.
///
/// This can refer to mathematical functions (excluding logical operations) or to STQL
/// specific functions like Euclidean distance, Class reference, Probability reference,
/// etc.
struct Function {
  enum struct Type { Add, Sub, Mul, Div, Custom };

  Type fn;
  std::optional<std::string> custom_fn;
  std::vector<std::shared_ptr<Expr>> args;

  std::set<std::string> attributes;
};

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_DETAILS_FUNCTIONS */
