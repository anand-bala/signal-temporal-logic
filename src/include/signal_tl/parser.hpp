#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_PARSER_HPP
#define SIGNAL_TEMPORAL_LOGIC_PARSER_HPP

#include "signal_tl/ast.hpp"
#include "signal_tl/internal/filesystem.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace signal_tl {

struct Specification {
 private:
  std::map<std::string, ast::Expr> _formulas;
  std::map<std::string, ast::Expr> _assertions;

 public:
  Specification() = default;

  void add_formula(const std::string&, ast::Expr);
  void add_assertion(const std::string&, ast::Expr);

  ast::Expr get_formula(std::string_view);
  ast::Expr get_assertion(std::string_view);
};

namespace parser {
std::unique_ptr<Specification> parse_string(std::string_view);

size_t analyze_specification_grammar(int verbose = 1);

} // namespace parser

}; // namespace signal_tl

#endif /* end of include guard: SIGNAL_TEMPORAL_LOGIC_PARSER_HPP */
