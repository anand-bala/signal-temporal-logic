/// @file   Argus/parser.hpp
/// @brief  Interface to the parser for the Argus specification language.
///
/// Here, we define the general interface for parsing the Argus specification language
/// from a string and from a file into a context.

#pragma once

#ifndef ARGUS_PARSER_HPP
#define ARGUS_PARSER_HPP

#include "argus/ast/ast_fwd.hpp"    // for ExprPtr
#include "argus/ast/attributes.hpp" // for Attribute::KeyCompare, Attribute
#include "argus/internal/filesystem.hpp"

#include <map>         // for map
#include <memory>      // for unique_ptr
#include <set>         // for set
#include <string>      // for string
#include <string_view> // for string_view

namespace argus {

/// The type of syntax that can be used in the specification.
///
/// - `FUTURE`: Implies that only future-time temporal operators (`Next`, `Always`,
///   `Eventually`, and `Until`) can be used in the specification.
/// - `PAST`: Implies that only _past-time_ temporal operators (`Prev`, `Historically`,
///   `Once`, and `Until`) can be used.
/// - `MIXED`: Implies that a mix of past-time and future-time operators can be used in
///   the specification language.
///
/// @note
/// Once the syntax has been set in the specification, we also need to check if the
/// chosen logic and the semantics support the given syntax.
enum struct SyntaxSettings { FUTURE, PAST, MIXED };

/// The type of logic used in the specification.
///
/// @note
/// The difference between `STL` and `MTL` is subtle: when the logic is set to `STL`,
/// the user must also specify the nature of the input signals, namely, if they are
/// discrete signals sampled at regular periods or if they are assumed to be linearly
/// interpolated signals. For `MTL` it is assumed to be just a discrete signal sampled
/// at regular periods.
enum struct Logic { MTL, STL, /* TQTL, STQL */ };

/// Holds the context of the parsed specification.
struct Context {
  Context() = default;

  /// Given a `string_view` of the actual specification (typically read from the
  /// specification script file), this fuction will return the parsed contents.
  ///
  /// Note that by returning a `std::unique_ptr`, the `signal_tl` library gives
  /// up ownership of the `Context`. The user of the library can manipulate
  /// the `Context` struct however they like, but the specification may
  /// lose its meaning once you do.
  static std::unique_ptr<Context> from_string(std::string_view);

  /// Given a `std::filesystem::path` to the specification file, this function
  /// reads the file and creates a concrete `Context` from it.
  ///
  /// Note that by returning a `std::unique_ptr`, the `signal_tl` library gives
  /// up ownership of the `Context`. The user of the library can manipulate
  /// the `Context` struct however they like, but the specification may
  /// lose its meaning once you do.
  static std::unique_ptr<Context> from_file(const fs::path&);

  /// List of defined formulas, keyed by their corresponding identifiers.
  std::map<std::string, ExprPtr> defined_formulas;
  /// List of  monitors, keyed by their corresponding identifiers.
  std::map<std::string, ExprPtr> monitors;
  /// List of settings
  std::set<argus::ast::details::Attribute, argus::ast::details::Attribute::KeyCompare>
      settings;
};

} // namespace argus

#endif /* end of include guard: ARGUS_PARSER_HPP */
