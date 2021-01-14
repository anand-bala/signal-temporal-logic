#pragma once

#ifndef SIGNALTL_PARSER_ERRORS_HPP
#define SIGNALTL_PARSER_ERRORS_HPP

#include "grammar.hpp"

namespace signal_tl::parser {
namespace peg = tao::pegtl;

namespace error_messages {

/// In this file we define the exceptions/error messages that must be
/// thrown/shown when some rule encounters a parsing error.
using namespace signal_tl::grammar;

/// The default error message should be nothing. Then, we will override the
/// messages for specific rules.
template <typename Rule>
inline constexpr const char* error_message = nullptr;

template <>
inline constexpr auto error_message<Skip> = "expected whitespace or comment";

template <>
inline constexpr auto error_message<peg::identifier> = "expected identifier";

template <>
inline constexpr auto error_message<
    peg::seq<peg::opt<tao::pegtl::one<'+', '-'>>, peg::plus<tao::pegtl::digit>>> =
    "expected some (signed) number after exponent";

template <>
inline constexpr auto error_message<PredicateForm> =
    "expected an identifier followed by a numeral or vice-versa";

template <>
inline constexpr auto error_message<peg::list<Term, Sep>> = "expected a list of Terms";

template <>
inline constexpr auto error_message<TermTail> =
    "expected an expression followed by a closing parenthesis ')'";

template <>
inline constexpr auto error_message<Term> = "expected a Term";

template <>
inline constexpr auto error_message<AllowedCommands> =
    "top-level command does not match list of known commands";

template <>
inline constexpr auto error_message<AnyCommandTail> =
    "expected (<command> ...). Maybe you have an unclosed S-expression command.";

template <>
inline constexpr auto error_message<StatementList> = "invalid top-level item";
} // namespace error_messages

struct error {
  template <typename Rule>
  static constexpr auto message = error_messages::error_message<Rule>;
};

template <typename Rule>
using control = peg::must_if<error, peg::normal>::control<Rule>;

} // namespace signal_tl::parser

#endif /* end of include guard: SIGNALTL_PARSER_ERRORS_HPP */
