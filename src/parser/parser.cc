#include "signal_tl/parser.hpp"

#include <iostream>

#include <tao/pegtl.hpp> // IWYU pragma: keep
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/trace.hpp>

/// Here, we define the grammar for a Lisp-y specification format using
/// S-expressions. This is inspired, in part, by the SMT-LIB.
namespace signal_tl::grammar {

namespace peg = tao::pegtl;

/// Line comments that starts using the character ';'
struct LineComment : peg::disable<peg::one<';'>, peg::until<peg::eolf>> {};

/// Any horizontal white space or line comment
/// NOTE(anand): I am also going to include EOL as a a skippable comment.
struct Sep : peg::sor<peg::ascii::space, LineComment> {};
struct Skip : peg::star<Sep> {};
struct EndOfWord : peg::seq<peg::not_at<peg::identifier_other>, Skip> {};

/// Some symbols
struct LParen : peg::seq<peg::one<'('>, Skip> {};
struct RParen : peg::seq<peg::one<')'>, Skip> {};
struct LtSymbol : peg::seq<peg::one<'<'>, peg::not_at<peg::one<'<', '='>>, Skip> {};
struct LeSymbol : peg::seq<TAO_PEGTL_STRING("<="), Skip> {};
struct GtSymbol : peg::seq<peg::one<'>'>, peg::not_at<peg::one<'>', '='>>, Skip> {};
struct GeSymbol : peg::seq<TAO_PEGTL_STRING(">="), Skip> {};

/// Keywords are one the following.
template <typename Key>
struct Keyword : tao::pegtl::seq<Key, EndOfWord> {};

struct KwTrue : Keyword<TAO_PEGTL_STRING("true")> {};
struct KwFalse : Keyword<TAO_PEGTL_STRING("false")> {};

struct KwNot : Keyword<TAO_PEGTL_STRING("not")> {};
struct KwAnd : Keyword<TAO_PEGTL_STRING("and")> {};
struct KwOr : Keyword<TAO_PEGTL_STRING("or")> {};
struct KwImplies : Keyword<TAO_PEGTL_STRING("implies")> {};
struct KwIff : Keyword<TAO_PEGTL_STRING("iff")> {};
struct KwXor : Keyword<TAO_PEGTL_STRING("xor")> {};
struct KwAlways : Keyword<TAO_PEGTL_STRING("always")> {};
struct KwEventually : Keyword<TAO_PEGTL_STRING("eventually")> {};
struct KwUntil : Keyword<TAO_PEGTL_STRING("until")> {};

struct KwDefineFormula : Keyword<TAO_PEGTL_STRING("define-formula")> {};
struct KwAssert : Keyword<TAO_PEGTL_STRING("assert")> {};

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
struct BooleanLiteral : peg::sor<KwTrue, KwFalse> {};

/// Primitive types are one of `double`, `int`, or `bool`.
///
/// NOTE:
///
/// The rule for Double must come before Integer as the latter partially
/// matches the former.
struct Numeral : peg::sor<DoubleLiteral, IntegerLiteral> {};
struct Constant : peg::sor<Numeral, BooleanLiteral> {};

/// Forward Declaration of Term fore recursion in Expression.
struct Term;

/// A predicate term is an expression of the form `(~ x c)` or `(~ c x) where,
/// `x` is a signal identifier, `~` is a relational operation, and `c` is a
/// numeric constant (double or integer).
///
/// TODO(anand): No support for arithmetic expressions of signals. Must be
/// implemented in userland.
struct PredicateTerm : peg::if_must<
                           peg::sor<LtSymbol, GtSymbol, LeSymbol, GeSymbol>,
                           peg::sor<
                               peg::seq<peg::identifier, Skip, Numeral>,
                               peg::seq<Numeral, Skip, peg::identifier>>> {};

struct NotTerm : peg::if_must<KwNot, Skip, Term> {};
struct AndTerm : peg::if_must<KwAnd, Skip, peg::list<Term, Sep>> {};
struct OrTerm : peg::if_must<KwOr, Skip, peg::list<Term, Sep>> {};
struct ImpliesTerm : peg::if_must<KwImplies, Skip, Term, Skip, Term> {};
struct IffTerm : peg::if_must<KwIff, Skip, Term, Skip, Term> {};
struct XorTerm : peg::if_must<KwXor, Skip, Term, Skip, Term> {};

// TODO(anand): Temporal operations must allow an optional Interval argument.
struct AlwaysTerm : peg::if_must<KwAlways, Skip, Term> {};
struct EventuallyTerm : peg::if_must<KwEventually, Skip, Term> {};
struct UntilTerm : peg::if_must<KwUntil, Skip, Term, Skip, Term> {};

/// An Expression must be a valid STL formula (without specific functions for
/// the predicates). So, we will hard code the syntax and we don't have to
/// worry about precedence as S-expressions FTW!
struct Expression : peg::sor<
                        PredicateTerm,
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

struct Term
    : peg::sor<
          Constant,
          peg::identifier,
          peg::if_must<LParen, peg::until<RParen, peg::seq<Expression, Skip>>>> {};

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
struct Command : peg::seq<peg::sor<Assertion, DefineFormula>, Skip> {};

/// Commands are top level S-expressions with the syntax:
///
///   <Command>   ::= '(' <keyword> <arguments>* ')'
///
/// We will hard code the top level commands, like `define-formula` and
/// `assert` as that will allow us to directly reason about them. A command is
/// essentially intrinsic to the specification language.
struct AnyCommand : peg::if_must<LParen, peg::until<RParen, Command>> {};

template <typename E>
struct StatementList : peg::seq<Skip, peg::until<E, AnyCommand, Skip>> {};

/// A specification essentially consists for a list of top level commands,
/// andwe are just gonna ignore all horizontal spaces
/* struct TopLevel : peg::pad<AnyCommand, Sep> {}; */
struct SpecificationFile : peg::must<StatementList<peg::eof>> {};

} // namespace signal_tl::grammar

namespace signal_tl::parser {

std::unique_ptr<Specification> from_string(std::string_view input) {
  tao::pegtl::string_input in(input, "from_content");
  const auto r = tao::pegtl::parse<grammar::SpecificationFile>(in);
  std::cout << "Returned = " << r << std::endl;
  return {};
}

std::unique_ptr<Specification> from_file(const stdfs::path& input) {
  tao::pegtl::file_input in(input);
  const auto r = tao::pegtl::parse<grammar::SpecificationFile>(in);
  std::cout << "Returned = " << r << std::endl;
  return {};
}

} // namespace signal_tl::parser

namespace signal_tl::grammar::internal {

size_t analyze(int verbose) {
  return tao::pegtl::analyze<SpecificationFile>(verbose);
}

bool trace_from_file(const stdfs::path& input_path) {
  tao::pegtl::file_input in(input_path);
  return tao::pegtl::standard_trace<SpecificationFile>(in);
}

} // namespace signal_tl::grammar::internal
