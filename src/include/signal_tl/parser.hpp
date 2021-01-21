
#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_PARSER_HPP
#define SIGNAL_TEMPORAL_LOGIC_PARSER_HPP

#include "signal_tl/ast.hpp" // for Expr
#include "signal_tl/internal/filesystem.hpp"

#include <cstddef>     // for size_t
#include <map>         // for map
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <string_view> // for string_view

namespace signal_tl {

/// Holds the concrete `Specification` that is read from a file.
///
/// A specification file is a list of commands/declarations which will be used
/// to build monitors for signals. See the documentation for the specification
/// file for more details.
struct Specification {
  std::map<std::string, ast::Expr> formulas;
  std::map<std::string, ast::Expr> assertions;

  Specification() = default;
  Specification(
      std::map<std::string, ast::Expr> _formulas,
      std::map<std::string, ast::Expr> _assertions) :
      formulas{std::move(_formulas)}, assertions{std::move(_assertions)} {}

  void add_formula(const std::string&, ast::Expr);
  void add_assertion(const std::string&, ast::Expr);

  ast::Expr get_formula(std::string_view);
  ast::Expr get_assertion(std::string_view);
};

namespace parser {

/// Given a `string_view` of the actual specification (typically read from the
/// specification script file), this fuction will return the parsed contents.
///
/// Note that by returning a `std::unique_ptr`, the `signal_tl` library gives
/// up ownership of the `Specification`. The user of the library can manipulate
/// the `Specification` struct however they like, but the specification may
/// lose its meaning once you do.
std::unique_ptr<Specification> from_string(std::string_view);

/// Given a `std::filesystem::path` to the specification file, this function
/// reads the file and creates a concrete `Specification` from it.
///
/// Note that by returning a `std::unique_ptr`, the `signal_tl` library gives
/// up ownership of the `Specification`. The user of the library can manipulate
/// the `Specification` struct however they like, but the specification may
/// lose its meaning once you do.
std::unique_ptr<Specification> from_file(const stdfs::path&);

} // namespace parser

// LCOV_EXCL_START
namespace grammar::internal {

/// **INTERNAL USE ONLY**
///
/// This is used to call `tao::pagtl::contrib::analyze`, a function that
/// analyzes the parser grammar for construction errors like unresolved cycles,
/// etc. Used in the tests to check the grammar and is useful only for
/// developers of this library.
size_t analyze(int verbose = 1);

bool trace_from_file(const stdfs::path&);

} // namespace grammar::internal
// LCOV_EXCL_STOP

} // namespace signal_tl

#endif /* end of include guard: SIGNAL_TEMPORAL_LOGIC_PARSER_HPP */
