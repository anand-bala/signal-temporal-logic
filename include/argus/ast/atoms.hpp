/// @file     Argus/ast/atoms.hpp
/// @brief    Atomic AST nodes.
///
/// In this file, we describe all possible leaf nodes of the AST.
#pragma once

#ifndef ARGUS_AST_ATOMS_HPP
#define ARGUS_AST_ATOMS_HPP

#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#include "argus/ast/ast_fwd.hpp"

namespace argus::ast::details {

/// @brief A constant in the AST.
///
/// An AST type that wraps around `string`, `double`, `int`, and `bool` to encode all
/// possible constants in the specification.
///
/// We use the string constant to represent special types for many specialized logics.
/// For example, in STQL, the string "CTIME" and "CFRAME" will refer to the
struct Constant : PrimitiveTypes {
  using PrimitiveTypes::variant;

  Constant()                = default;
  Constant(Constant&&)      = default;
  Constant(const Constant&) = default;

  Constant(PrimitiveTypes constant) : PrimitiveTypes{std::move(constant)} {}
  template <typename ConstantType>
  Constant(ConstantType constant) : PrimitiveTypes{std::move(constant)} {
    static_assert(
        std::disjunction_v<
            std::is_same_v<ConstantType, std::string>,
            std::is_same_v<ConstantType, bool>,
            std::is_same_v<ConstantType, double>,
            std::is_same_v<ConstantType, long long int>,
            std::is_same_v<ConstantType, unsigned long long int>>,
        "Attempting to initialize constant with unsupported type");
  }

  /// Convenience method to check if the constant is a `bool`.
  [[nodiscard]] constexpr bool is_bool() const {
    return std::holds_alternative<bool>(*this);
  }

  /// Convenience method to check if the constant is a `double`.
  [[nodiscard]] constexpr bool is_real() const {
    return std::holds_alternative<double>(*this);
  }

  /// Convenience method to check if the constant is a signed integer.
  [[nodiscard]] constexpr bool is_integer() const {
    return std::holds_alternative<long long int>(*this);
  }

  /// Convenience method to check if the constant is an unsigned integer.
  [[nodiscard]] constexpr bool is_unsigned() const {
    return std::holds_alternative<unsigned long long int>(*this);
  }

  /// Convenience method to check if the constant is a `string`.
  [[nodiscard]] constexpr bool is_string() const {
    return std::holds_alternative<std::string>(*this);
  }

  [[nodiscard]] constexpr bool is_nonnegative() const {
    if (is_unsigned()) {
      return true;
    } else if (is_integer()) {
      return std::get<long long int>(*this) >= 0;
    } else if (is_real()) {
      return std::get<double>(*this) >= 0.0;
    } else {
      return false;
    }
  }

  [[nodiscard]] std::string to_string() const;
};

/// @brief A typed variable.
///
/// Used as a placeholder until evaluated by the chosen semantics.
struct Variable {
  /// The name of the variable (some qualified identifier).
  std::string name;

  /// The type of the variable must be a primitive type.
  enum struct Type { Real, Int, UInt, Bool };
  /// The type of the variable.
  Type type;

  /// The scope of a variable deterimines if it is either one of:
  /// 1. A global input signal;
  /// 2. A global output signal (used for interface-aware semantics); or
  enum struct Scope { Input, Output };
  Scope scope;

  Variable(std::string name_arg, Type type_arg, Scope scope_arg = Scope::Input) :
      name{std::move(name_arg)}, type{type_arg}, scope{scope_arg} {};

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

  /// Check if the variable is an unsigned.
  [[nodiscard]] constexpr bool is_unsigned() const {
    return type == Type::UInt;
  }

  /// Check if the variable is an input signal.
  [[nodiscard]] constexpr bool is_input() const {
    return scope == Scope::Input;
  }

  /// Check if the variable is an output signal.
  [[nodiscard]] constexpr bool is_output() const {
    return scope == Scope::Output;
  }

  [[nodiscard]] std::string to_string() const;
};

/// @brief A typed parameter AST node
///
/// Used in Parametric STL to denote placeholders for variables that are not signals.
struct Parameter {
  /// The name of the parameter (some qualified identifier).
  std::string name;

  /// The type of the parameter must be a primitive type.
  enum struct Type { Real, Int, UInt, Bool };
  /// The type of the parameter.
  Type type;

  Parameter(std::string name_arg, Type type_arg) :
      name{std::move(name_arg)}, type{type_arg} {};

  /// Check if the parameter is a boolean.
  [[nodiscard]] constexpr bool is_bool() const {
    return type == Type::Bool;
  }

  /// Check if the parameter is real-valued (double).
  [[nodiscard]] constexpr bool is_real() const {
    return type == Type::Real;
  }

  /// Check if the parameter is an integer.
  [[nodiscard]] constexpr bool is_integer() const {
    return type == Type::Int;
  }

  [[nodiscard]] std::string to_string() const;
};

} // namespace argus::ast::details

#endif
