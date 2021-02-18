/// @file     Argus/ast/atoms.hpp
/// @brief    Atomic AST nodes.
///
/// In this file, we describe all possible leaf nodes of the AST.
#pragma once

#ifndef ARGUS_AST_ATOMS_HPP
#define ARGUS_AST_ATOMS_HPP

#include <optional>
#include <string>
#include <variant>

namespace argus::ast {

/// @brief A constant in the AST.
///
/// An AST type that wraps around `string`, `double`, `int`, and `bool` to encode all
/// possible constants in the specification.
///
/// We use the string constant to represent special types for many specialized logics.
/// For example, in STQL, the string "CTIME" and "CFRAME" will refer to the
struct Constant : std::variant<std::string, double, int, bool> {
  using std::variant<std::string, double, int, bool>::variant;

  /// Convenience method to check if the constant is a `bool`.
  [[nodiscard]] constexpr bool is_bool() const {
    return std::holds_alternative<bool>(*this);
  }

  /// Convenience method to check if the constant is a `double`.
  [[nodiscard]] constexpr bool is_real() const {
    return std::holds_alternative<double>(*this);
  }

  /// Convenience method to check if the constant is a `int`.
  [[nodiscard]] constexpr bool is_integer() const {
    return std::holds_alternative<int>(*this);
  }

  /// Convenience method to check if the constant is a `string`.
  [[nodiscard]] constexpr bool is_string() const {
    return std::holds_alternative<std::string>(*this);
  }
};

/// @brief A typed variable.
///
/// Used as a placeholder until evaluated by the chosen semantics.
struct Variable {
  /// The name of the variable (some qualified identifier).
  std::string name;

  /// The type of the variable must be a primitive type.
  enum struct Type { Real, Int, Bool };
  /// The type of the variable.
  Type type;
  /// If the variable is a custom type, then this field holds the string representation
  /// of the type.
  std::optional<std::string> custom_type;

  /// The scope of a variable deterimines if it is either one of:
  /// 1. A global input signal;
  /// 2. A global output signal (used for interface-aware semantics); or
  enum struct Scope { Input, Output };
  Scope scope;

  /// Check if the variable is a boolean.
  [[nodiscard]] constexpr bool is_bool() const {
    return type == Type::Bool;
  }

  /// Check if the variable is real-valued (double).
  [[nodiscard]] constexpr bool is_real() const {
    return type == Type::Real;
  }

  /// Check if the variable is an integer.
  [[nodiscard]] constexpr bool is_integer() const {
    return type == Type::Int;
  }

  /// Check if the variable is an input signal.
  [[nodiscard]] constexpr bool is_input() const {
    return scope == Scope::Input;
  }

  /// Check if the variable is an output signal.
  [[nodiscard]] constexpr bool is_output() const {
    return scope == Scope::Output;
  }

  /// Check if the variable is an output signal.
};

/// @brief A typed parameter AST node
///
/// Used in Parametric STL to denote placeholders for variables that are not signals.
struct Parameter {
  /// The type of the parameter must be a primitive type.
};

}; // namespace argus::ast

#endif /* end of include guard: ARGUS_AST_ATOMS_HPP */
