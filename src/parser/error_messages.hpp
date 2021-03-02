#pragma once

#ifndef SIGNALTL_PARSER_ERRORS_HPP
#define SIGNALTL_PARSER_ERRORS_HPP

#include "grammar.hpp"

namespace argus::parser {
namespace peg = tao::pegtl;

/// Here we define the exceptions/error messages that must be thrown/shown when some
/// rule encounters a parsing error.
namespace control {

using namespace argus::grammar;

/// The default error message should be nothing. Then, we will override the
/// messages for specific rules.
template <typename Rule>
inline constexpr const char* error_message = nullptr;

template <>
inline constexpr auto error_message<Skip> = "expected whitespace or comment";

template <>
inline constexpr auto error_message<sym::rparen> = "expected a closing ')'";

template <>
inline constexpr auto
    error_message<peg::seq<peg::opt<peg::one<'+', '-'>>, peg::plus<peg::digit>>> =
        "expected some (signed) number after exponent";

template <>
inline constexpr auto error_message<peg::plus<num::binary_digit>> =
    "expected a list of binary digits";

template <>
inline constexpr auto error_message<peg::plus<num::octal_digit>> =
    "expected a list of octal digits";

template <>
inline constexpr auto error_message<peg::plus<num::hex_digit>> =
    "expected a list of hex digits";

template <>
inline constexpr auto error_message<peg::one<'b', 't', 'n', 'f', 'r', '"', '\\'>> =
    "unknown escape sequence";

template <>
inline constexpr auto error_message<
    peg::until<sym::double_quote, chars::raw_string_char>> =
    "invalid string literal (either used an invalid character or unclosed quotations)";

template <>
inline constexpr auto
    error_message<peg::until<sym::vert_bar, peg::plus<chars::quoted_symbol_char>>> =
        "invalid quoted symbol (either used an invalid character or unclosed `|`)";

template <>
inline constexpr auto error_message<Constant> = "expected a constant";

template <>
inline constexpr auto error_message<peg::list<Constant, Skip>> =
    "expected a list of constants";

template <>
inline constexpr auto error_message<SimpleSymbol> =
    "expected a simple (unquoted) symbol";

template <>
inline constexpr auto error_message<VarName> = "expected a variable name declaration";

template <>
inline constexpr auto error_message<VarDecl> =
    "expected a variable declaration `(name type)`";

template <>
inline constexpr auto error_message<Expression> =
    "expected a well-structured Expression";

template <>
inline constexpr auto error_message<Term> = "expected a well-structured Term";

template <>
inline constexpr auto error_message<valid_commands> = "invalid top-level command";

template <>
inline constexpr auto error_message<StatementList> = "invalid syntax";

struct error {
  template <typename Rule>
  static constexpr auto message = error_message<Rule>;

  /// This is used to prevent local failues in the Term rule from becoming
  /// global failures.
  template <typename Rule>
  static constexpr bool raise_on_failure = false;
};

template <typename Rule>
using control = peg::must_if<error, peg::normal>::control<Rule>;
} // namespace control

} // namespace argus::parser

#endif /* end of include guard: SIGNALTL_PARSER_ERRORS_HPP */
