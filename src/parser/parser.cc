#include "signal_tl/parser.hpp"

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

/// Some symbols
struct LtSymbol : peg::seq<peg::one<'<'>, peg::at<peg::not_one<'<', '='>>> {};
struct LeSymbol : TAO_PEGTL_STRING("<=") {};
struct GtSymbol : peg::seq<peg::one<'>'>, peg::at<peg::not_one<'>', '='>>> {};
struct GeSymbol : TAO_PEGTL_STRING(">=") {};

/// Keywords are one the following.
struct KwTrue : TAO_PEGTL_KEYWORD("true") {};
struct KwFalse : TAO_PEGTL_KEYWORD("false") {};

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

// First we define some helpers
struct decimal_seq : peg::plus<peg::digit> {};

template <typename E>
struct exponent : tao::pegtl::if_must<
                      E,
                      tao::pegtl::opt<tao::pegtl::one<'+', '-'>>,
                      tao::pegtl::plus<tao::pegtl::digit>> {};

struct DoubleLiteral : peg::seq<
                           peg::sor<
                               peg::seq<
                                   decimal_seq,
                                   peg::one<'.'>,
                                   decimal_seq,
                                   peg::opt<exponent<peg::one<'e', 'E'>>>>,
                               peg::seq<
                                   decimal_seq,
                                   peg::opt<peg::one<'.'>>,
                                   exponent<peg::one<'e', 'E'>>>>,
                           Skip> {};
struct IntegerLiteral : peg::seq<decimal_seq, Skip> {};
struct BooleanLiteral : peg::seq<peg::sor<KwTrue, KwFalse>, Skip> {};

/// Primitive types are one of `double`, `int`, or `bool`.
///
/// NOTE:
///
/// The rule for Double must come before Integer as the latter partially
/// matches the former.
struct Numeral : peg::sor<DoubleLiteral, IntegerLiteral> {};
struct Constant : peg::sor<Numeral, BooleanLiteral> {};

struct Term;

/// A predicate term is an expression of the form `(~ x c)` or `(~ c x) where,
/// `x` is a signal identifier, `~` is a relational operation, and `c` is a
/// numeric constant (double or integer).
///
/// TODO(anand): No support for arithmetic expressions of signals. Must be
/// implemented in userland.
struct PredicateTerm : peg::seq<
                           peg::sor<LtSymbol, GtSymbol, LeSymbol, GeSymbol>,
                           Skip,
                           peg::sor<
                               peg::seq<peg::identifier, Skip, Numeral>,
                               peg::seq<Numeral, Skip, peg::identifier>>> {};

struct NotTerm : peg::if_must<KwNot, peg::pad<Term, Sep>> {};
struct AndTerm : peg::if_must<KwAnd, peg::list<Term, Sep>> {};
struct OrTerm : peg::if_must<KwOr, peg::list<Term, Sep>> {};
struct ImpliesTerm : peg::if_must<KwImplies, Skip, Term, Skip, Term, Skip> {};
struct IffTerm : peg::if_must<KwIff, Skip, Term, Skip, Term, Skip> {};
struct XorTerm : peg::if_must<KwXor, Skip, Term, Skip, Term, Skip> {};

// TODO(anand): Temporal operations must allow an optional Interval argument.
struct AlwaysTerm : peg::if_must<KwAlways, peg::pad<Term, Sep>> {};
struct EventuallyTerm : peg::if_must<KwEventually, peg::pad<Term, Sep>> {};
struct UntilTerm : peg::if_must<KwUntil, Skip, Term, Skip, Term, Skip> {};

/// An Expression must be a valid STL formula (without specific functions for
/// the predicates). So, we will hard code the syntax and we don't have to
/// worry about precedence as S-expressions FTW!
struct Expression : peg::sor<
                        NotTerm,
                        AndTerm,
                        OrTerm,
                        ImpliesTerm,
                        IffTerm,
                        XorTerm,
                        AlwaysTerm,
                        EventuallyTerm,
                        UntilTerm,
                        Term> {};

struct Term : peg::sor<
                  Constant,
                  peg::identifier,
                  peg::if_must<
                      peg::one<'('>,
                      peg::until<peg::one<')'>, peg::pad<Expression, Skip>>>> {};

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
struct TopLevel : peg::pad<AnyCommand, Sep> {};
struct SpecificationFile : peg::until<peg::eof, peg::must<peg::star<TopLevel>>> {};

}; // namespace signal_tl::internal::grammar

#include <tao/pegtl/contrib/analyze.hpp>

size_t signal_tl::parser::internal::analyze_grammar(int verbose) {
  using namespace ::signal_tl::internal::grammar;
  return ::tao::pegtl::analyze<SpecificationFile>(verbose);
}
