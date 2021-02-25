/// @file     parser/grammar.hpp
/// @brief    Definition of specification language grammar.

/// Conventions
/// ===========
///
/// - Use `using` to declare a rule that is a helper to build an actual rule in the
///   grammar.
/// - Use `struct` to define rules that affect the AST.

#pragma once

#ifndef ARGUS_GRAMMAR_HPP
#define ARGUS_GRAMMAR_HPP

#include <tao/pegtl.hpp> // IWYU pragma: keep

/// @namsepace argus::grammar
/// @brief grammar definition for the argus specification language.
///
/// Here, we define the grammar for a Lisp-y specification format using
/// S-expressions. This is inspired, in part, by the SMT-LIB.
namespace argus::grammar {
namespace peg = tao::pegtl;

/// Line comments that starts using the character ';'
struct LineComment : peg::disable<peg::one<';'>, peg::until<peg::eolf>> {};

/// Any horizontal whitespace
struct Whitespace : peg::disable<peg::space> {};

/// Any horizontal white space or line comment
/// NOTE(anand): I am also going to include EOL as a a skippable comment.
struct Sep : peg::disable<peg::sor<Whitespace, LineComment, peg::eol>> {};
/// We use this as a placeholder for `Sep*`
struct Skip : peg::disable<peg::star<Sep>> {};
/// We use this to mark the end of a word/identifier (which is skippable whitespace)
struct EndOfWord : peg::seq<peg::not_at<peg::identifier_other>, Skip> {};

struct Identifier : peg::seq<peg::identifier, EndOfWord> {};

namespace sym {
using lparen    = peg::one<'('>;
using rparen    = peg::one<')'>;
using colon     = peg::one<':'>;
using semicolon = peg::one<';'>;

using double_quote = peg::one<'"'>;
using single_quote = peg::one<'\''>;
using vert_bar     = peg::one<'|'>;
using backslash    = peg::one<'\\'>;

using dot = peg::one<'.'>;

using plus          = peg::one<'+'>;
using minus         = peg::one<'-'>;
using plus_or_minus = peg::one<'+', '-'>;

using underscore = peg::one<'_'>;
using atsign     = peg::one<'@'>;
} // namespace sym

template <typename... R>
using rpad = peg::seq<R..., peg::at<Skip>>;

template <typename... R>
using paren_surround = peg::if_must<sym::lparen, Skip, R..., Skip, sym::rparen, Skip>;

/// Keywords are one the following.
template <typename Key>
using builtin = peg::seq<Key, EndOfWord>;

struct KwTrue : builtin<TAO_PEGTL_STRING("true")> {};
struct KwFalse : builtin<TAO_PEGTL_STRING("false")> {};

using bool_primitives = peg::sor<KwTrue, KwFalse>;

struct KwSetOption : builtin<TAO_PEGTL_STRING("set-option")> {};
struct KwDefineFormula : builtin<TAO_PEGTL_STRING("define-formula")> {};
struct KwDeclareSignal : builtin<TAO_PEGTL_STRING("declare-signal")> {};
struct KwDeclareParameter : builtin<TAO_PEGTL_STRING("declare-parameter")> {};
struct KwMonitor : builtin<TAO_PEGTL_STRING("monitor")> {};

struct IntervalOp : builtin<sym::underscore> {};

using builtin_cmd = peg::
    sor<KwSetOption, KwDefineFormula, KwDeclareSignal, KwDeclareParameter, KwMonitor>;

using reserved = peg::sor<builtin_cmd, bool_primitives>;

// Built-in types
struct TypeBool : builtin<TAO_PEGTL_STRING("Bool")> {};
struct TypeInt : builtin<TAO_PEGTL_STRING("Int")> {};
struct TypeUInt : builtin<TAO_PEGTL_STRING("UInt")> {};
struct TypeReal : builtin<TAO_PEGTL_STRING("Real")> {};

using builtin_type = peg::sor<TypeBool, TypeInt, TypeUInt, TypeReal>;

/// Numeric Tokens and helpers
namespace num {
using bin_pre = TAO_PEGTL_STRING("#b");
using oct_pre = TAO_PEGTL_STRING("#o");
using hex_pre = TAO_PEGTL_STRING("#x");

using binary_digit  = peg::one<'0', '1'>;
using octal_digit   = peg::range<'0', '7'>;
using decimal_digit = peg::range<'0', '9'>;
using hex_digit     = peg::ranges<'0', '9', 'a', 'f', 'A', 'F'>;

using decimal_seq = peg::plus<decimal_digit>;
template <typename E>
using exponent =
    peg::if_must<E, peg::seq<peg::opt<sym::plus_or_minus>, peg::plus<peg::digit>>>;
} // namespace num

struct BinInt : peg::if_must<num::bin_pre, peg::plus<num::binary_digit>> {};
struct OctInt : peg::if_must<num::oct_pre, peg::plus<num::octal_digit>> {};
struct HexInt : peg::if_must<num::hex_pre, peg::plus<num::hex_digit>> {};
struct DecInt : peg::plus<peg::digit> {};

struct IntegerLiteral : peg::seq<peg::sor<BinInt, OctInt, HexInt, DecInt>> {};

struct DoubleLiteral : peg::seq<peg::sor<
                           peg::seq<
                               num::decimal_seq,
                               sym::dot,
                               num::decimal_seq,
                               peg::opt<num::exponent<peg::one<'e', 'E'>>>>,
                           peg::seq<
                               num::decimal_seq,
                               peg::opt<sym::dot>,
                               num::exponent<peg::one<'e', 'E'>>>>> {};

struct BooleanLiteral : peg::sor<KwTrue, KwFalse> {};

/// Character tokens and helpers
///
/// Since we are deriving this grammar from SMT-LIBv2, we don't really care about
/// escaping characters within strings, etc. Rather, we only escape the sequence "" to
/// be the character "
namespace chars {

/// All printable characters
using printable_char = peg::utf8::ranges<U'\u0020', U'\u007e', U'\u0080', U'\uffff'>;

/// List of escape sequences include `\b \t \n \f \r \" \\`.
using escaped =
    peg::if_must<sym::backslash, peg::one<'b', 't', 'n', 'f', 'r', '"', '\\'>>;
/// All printable characters excluding double quotes, and the escape
/// sequences `\b \t \n \f \r \" \\`.
using raw_string_char = peg::sor<escaped, printable_char>;

using symbol_special_chars = peg::one<
    '~',
    '!',
    '@',
    '$',
    '%',
    '^',
    '&',
    '*',
    '_',
    '-',
    '+',
    '=',
    '<',
    '>',
    '.',
    '?',
    '/'>;
/// A simple symbol can start with any ASCII letter and the characters `~ ! @ $ % ^
/// & * _ - + = < > . ? /`.
using simple_symbol_start = peg::sor<peg::alpha, symbol_special_chars>;
/// The tail of a simple symbol can contain any ASCII letter, digits, and the characters
/// `~ ! @ $ % ^ & * _ - + = < > . ? /`.
using simple_symbol_tail = peg::sor<peg::digit, peg::alpha, symbol_special_chars>;

/// All printable characters excluding backslash \ and vert |
using quoted_symbol_char = peg::sor<
    Whitespace,
    peg::utf8::ranges<
        U'\u0020',
        U'\u005b',
        U'\u005d',
        U'\u007b',
        U'\u007d',
        U'\u007e',
        U'\u0080',
        U'\uffff'>>;

} // namespace chars

/// A string literal is any sequence of characters between two double quotes, with the
/// literal escape sequence `\b \t \n \f \r \" \\`.
///
/// @note
/// This eschews from the definition of a raw string literal in SMT-LIB (even though
/// this specification language is inspired by it) because of the different use cases.
/// In SMT-LIB, they assume that strings can be inputs to the solver, thus, the
/// significance of the backslash and other "traditionally" escaped characters in C-like
/// languages is important, as these can change the input to the program.
///
/// \par
/// In PerceMon, strings have no significance other than to document various things. In
/// reality, this is here only for familiarity and may never be used by the script user.
struct StringLiteral : peg::if_must<
                           sym::double_quote,
                           peg::until<sym::double_quote, chars::raw_string_char>> {};

/// A constant in the specification language can be a floating point constant, an
/// integer, a boolean literal, or a string.
struct Constant
    : peg::sor<DoubleLiteral, IntegerLiteral, BooleanLiteral, StringLiteral> {};

/// A simple symbol is a case sensitive sequence of characters that are either ASCII
/// letters, ASCII digits, or the characters `~ ! @ $ % ^ & * _ - + = < > . ? /`
struct SimpleSymbol : peg::seq<
                          peg::not_at<reserved>,
                          chars::simple_symbol_start,
                          peg::star<chars::simple_symbol_tail>> {};
/// A quoted symbol is any sequence of whitespace characters and printable characters
/// that starts and ends with `|` and does not contain `|`or `\`.
struct QuotedSymbol
    : peg::seq<peg::if_must<
          sym::vert_bar,
          peg::until<sym::vert_bar, peg::plus<chars::quoted_symbol_char>>>> {};

/// A valid symbol (mainly used as identifiers) is either a @ref QuotedSymbol or a @ref
/// SimpleSymbol.
struct Symbol : peg::sor<SimpleSymbol, QuotedSymbol> {};

/// A keyword is a token of the form `:<simple_symbol>`. They are used as attributes or
/// options for commands in PerceMon.
struct Keyword : peg::if_must<sym::colon, SimpleSymbol> {};

struct Attribute;

/// Option attributes.
///
/// Used to set options for operations and commands that accept them.
///
/// Action
/// ======
///
/// 1. Check if there is a keyword.
/// 2. Create an `attribute` with boolean `true` value.
struct OptionAttribute : Keyword {};

/// Attribute Values
///
/// A recursive rule that allows Key-value attributes the have nested attributes.
struct AttributeValue : peg::seq<paren_surround<Attribute>, Constant, Symbol> {};

/// Key-value attributes.
///
/// @todo
/// Currently, KeyValueAttributes can only have a single value. It may be useful to have
/// a list of mappings (but this can get weird for parsing).
struct KeyValueAttribute : peg::seq<Keyword, Skip, Constant> {};

/// An attribute is simply either an option attribute or a keyval attribute.
struct Attribute : peg::sor<KeyValueAttribute, OptionAttribute> {};

struct Term;

/// A operation is just a symbol. This can include one of the following
/// pre-defined operations/operators:
///
/// - `not`
/// - `and`
/// - `or`
/// - `implies`
/// - `iff`
/// - `xor`
/// - `always`
/// - `eventually`
/// - `until`
struct Operation : Symbol {};
/// A qualified identifier is used to reference some pre-defined symbol or variable.
struct QualifiedIdentifier : Symbol {};

/// Helper rule for Variable names
///
/// Action
/// ======
///
/// 1. Check if there is a `symbol`.
/// 2. Move `symbol` to `var_name`.
struct VarName : Symbol {};
/// Rule to infer variable type.
///
/// Action
/// ======
///
/// 1. Check if there is a `symbol`.
/// 2. Move the `{symbol}` to `type`.
struct VarType : Symbol {};
/// Typed or Untyped declarations for Variables.
///
/// Actions
/// =======
///
/// 1. Check if there is a var_name. (Must)
/// 2. Check if there is a type. (Optional)
/// 3. Create a `Variable` with given name and type.
/// 4. Move the `Variable` to `variables`
struct VarDecl : peg::sor<paren_surround<VarName, Skip, peg::opt<VarType>>, VarName> {};
/// A list of untyped/implicitely typed variables.
///
/// Action
/// ======
///
/// @todo Nothing, I think.
struct VarList : paren_surround<peg::list<VarDecl, Skip>> {};

/// Rule for pinning the frame and time variables.
///
/// Action
/// =====
///
/// 1. Assert the operation is "@" (redundant).
/// 2. Check if there is either 1 or 2 Variables in the `variables` list.
/// 3. Create a `result`.
struct PinningExpression : peg::seq<KwPin, Skip, VarList> {};

using quantifier_ops = peg::sor<KwExists, KwForall>;
/// Rule to encode quantifier expressions.
///
/// Action
/// ======
///
/// 1. Check if there is 1 `operation`.
/// 2. Check if there is at least 1 variable in the `variables` list.
/// 3. Check if there is 1 Term in the `terms` list.
/// 4. Create a `result` for the quantifier.
struct QuantifierExpression : peg::seq<quantifier_ops, Skip, VarList, Skip, Term> {};

/// Rule to encode Interval operations
///
/// Action
/// ======
///
/// 1. Check if the operation is '_'
/// 2. Check if there is exactly 1 Term in the `terms` list.
/// 3. Create an Interval.
struct IntervalExpression : peg::seq<KwInterval, Skip, Term> {};

/// Rule to encode arbitrary operations.
///
/// Used to match all operations, including Predicates, Arithmetic, Temporal, Spatial,
/// and Spatio-Temporal.
///
/// Action
/// ======
///
/// 1. Check if the `operation` exists. Get type of operation (Predicate, Arithmetic,
///    etc.)
/// 2. Get list of `terms` (make sure there is at least 1).
/// 3. Get list of `attributes`.
/// 4. Create `result`.
struct OperationExpression
    : peg::seq<Operation, Skip, peg::list<Term, Skip>, Skip, peg::star<Attribute>> {};

/// Helper rule for expressions that are within `( ... )`
///
/// Actions
/// =======
///
/// Nothing. Let sub-rules handle it.
struct Expression : peg::sor<
                        PinningExpression,
                        QuantifierExpression,
                        IntervalExpression,
                        OperationExpression> {};

/// A recursive Term expression.
///
/// Actions
/// =======
///
/// 1. Push in a new local context.
/// 2. Call default match.
/// 3. Check if there is a `result` at the top of the local context stack.
/// 4. Pop top of stack and move the result into the `terms` list in the new top.
struct Term : peg::sor<paren_surround<Expression>, Constant, QualifiedIdentifier> {};

/// Set global options for the script.
///
/// Actions
/// =======
///
/// For now, nothing.
struct CmdSetOption : peg::seq<KwSetOption, Skip, peg::plus<Attribute, Skip>> {};

/// Define a formula.
///
/// @note
/// The formula must be strongly typed/well-sorted, i.e., all signals referred to in a
/// formula must be of the same type or types that are coercable to each other (e.g.
/// `Int` can be coerced to `Real`).
///
/// Actions
/// =======
///
/// 1. Check if there is exactly 1 Identifier, and 1 Term.
/// 2. Add entry `{identifier, term}` to `formulas` map.
struct CmdDefineFormula
    : peg::seq<KwDefineFormula, Skip, QualifiedIdentifier, Skip, Term> {};

/// Define a monitor for a formula.
///
/// The monitor takes in a list of attributes that will set the semantics of the
/// monitor. The "name" of the identifier can be used to check the output of PerceMon.
///
/// Actions
/// =======
///
/// 1. In the parsed context, we will check if there is exactly 1 Identifier, 1 Term in
/// the
///    list of terms, and some arbitrary number of attributes.
/// 2. We then (for now) just add to the `monitors` map in the context the entry
///    `{identifier, term}`
struct CmdMonitor : peg::seq<
                        KwMonitor,
                        Skip,
                        QualifiedIdentifier,
                        Skip,
                        Term,
                        Skip,
                        peg::star<Attribute, Skip>> {};

using valid_commands = peg::sor<CmdSetOption, CmdDefineFormula, CmdMonitor>;

struct StatementList
    : peg::until<peg::eof, peg::pad<paren_surround<valid_commands>, Sep>> {};
/// A specification essentially consists for a list of top level commands,
/// andwe are just gonna ignore all horizontal spaces
struct Specification : peg::must<StatementList> {};

} // namespace argus::grammar

#endif /* end of include guard: ARGUS_GRAMMAR_HPP */
