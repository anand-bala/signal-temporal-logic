#pragma once

#ifndef SIGNALTL_INTERNAL_GRAMMAR_HPP
#define SIGNALTL_INTERNAL_GRAMMAR_HPP

#include <tao/pegtl.hpp>

/// Here, we define the grammar for a Lisp-y specification format using
/// S-expressions. This is inspired, in part, by the SMT-LIB.

namespace signal_tl::internal::grammar {
namespace peg = tao::pegtl;

/// Line comments that starts using the character ';'
struct Comment : peg::disable<peg::one<';'>, peg::until<peg::eolf>> {};

/// Any horizontal white space or line comment
/// NOTE(anand): I am also going to include EOL as a a skippable comment.
struct Sep : peg::sor<peg::ascii::space, peg::eol, Comment> {};
struct Skip : peg::star<Sep> {};

/// Keywords are one the following.
struct KwNot : TAO_PEGTL_KEYWORD("not") {};
struct KwAnd : TAO_PEGTL_KEYWORD("and") {};
struct KwOr : TAO_PEGTL_KEYWORD("or") {};
struct KwImplies : TAO_PEGTL_KEYWORD("implies") {};
struct KwIff : TAO_PEGTL_KEYWORD("iff") {};
struct KwXor : TAO_PEGTL_KEYWORD("xor") {};
struct KwAlways : TAO_PEGTL_KEYWORD("always") {};
struct KwEventually : TAO_PEGTL_KEYWORD("eventually") {};
struct KwUntil : TAO_PEGTL_KEYWORD("until") {};

struct KwDefineFormula : TAO_PEGTL_KEYWORD("define-formula") {};
struct KwAssert : TAO_PEGTL_KEYWORD("assert") {};

struct Term;

struct Constant;

/// An Expression must be a valid STL formula (without specific functions for
/// the predicates). So, we will hard code the syntax and we don't have to
/// worry about precedence as S-expressions FTW!
struct Expression;

struct Term : peg::sor<
                  Constant,
                  peg::identifier,
                  peg::if_must<peg::one<'('>, peg::until<peg::one<')'>, Expression>>> {
};

struct Assertion : peg::if_must<KwAssert, Skip, peg::identifier, Skip, Term> {};
struct DefineFormula
    : peg::if_must<KwDefineFormula, Skip, peg::identifier, Skip, Expression> {};

/// The list of commands includes
///
/// - Assertion: `(assert <identifier> <expression>)`
///
///   Here, we will use the `<identifier>` to refer to the assertion rule
///   (essentially the semantics) from the program. This is different from most
///   other languages that have an `assert` statement, where it is a runtime
///   thing.
///
///   NOTE: We should eventually have this file run as a script.
///
/// - Define Formula `(define-formula <identifier> <expression>)`
///
///   This is straightforward enough: we define a formula (named using
///   `<identifier>`) as some expression.
struct Command : peg::pad<peg::sor<Assertion, DefineFormula>, Skip> {};

/// Commands are top level S-expressions with the syntax:
///
///   <Command>   ::= '(' <keyword> <arguments>* ')'
///
/// We will hard code the top level commands, like `define-formula` and
/// `assert` as that will allow us to directly reason about them. A command is
/// essentially intrinsic to the specification language.
struct AnyCommand : peg::if_must<peg::one<'('>, peg::until<peg::one<')'>, Command>> {};

/// A specification essentially consists for a list of top level commands,
/// andwe are just gonna ignore all horizontal spaces
struct TopLevel : peg::sor<Skip, AnyCommand> {};
struct SpecificationFile : peg::until<peg::eof, peg::must<peg::star<TopLevel>>> {};

}; // namespace signal_tl::internal::grammar

#endif /* end of include guard: SIGNALTL_INTERNAL_GRAMMAR_HPP */
