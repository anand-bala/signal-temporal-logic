#pragma once

#ifndef SIGNALTL_PARSER_ACTIONS_HPP
#define SIGNALTL_PARSER_ACTIONS_HPP

#include "grammar.hpp"
#include "signal_tl/ast.hpp"
#include <tao/pegtl.hpp> // IWYU pragma: keep

#include <fmt/core.h>

// #define NDEBUG
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace signal_tl::parser {

namespace actions {
/// Here, we will define the custom actions for the PEG parser that will
/// convert each rule into a valid AST class.
namespace peg = tao::pegtl;
using namespace signal_tl::grammar;

using PrimitiveState = std::variant<bool, long int, double>;

enum struct PredicateType { ONE, TWO };

/// This encodes local state of the push-down parser.
///
/// Essentially, this is a stack element, and a new one is pushed down
/// whenerver we encounter a Term, as that is the only recursive rule in our
/// grammar. This keeps track of whatever is required to make a Term, without
/// polluting the context of parent rules in the AST.
struct ParserState {
  /// Purely here for debugging purposes.
  unsigned long long int level;

  /// Whenever an Expression is completed, this field gets populared. Later,
  /// within the action for the Term rule, we move the result onto the vector
  /// `terms` to allow for the parent expression to easily combine it with
  /// their list of `terms`.
  std::optional<ast::Expr> result;

  /// When parsing a primitive (boolean or numeral), we add the result here so
  /// that the parent rule can immediately use it to construct either a
  /// BooleanLiteral or a Numeral (integer or double).
  std::optional<PrimitiveState> primitive_result;

  /// A list of identifiers as parsed by the rule.
  ///
  /// In most cases, this is immediately used by the parent rule (either in
  /// Term or in some Command) to either create new formulas/assertions or to
  /// map existing formulas/assertions to a valid `ast::Expr`.
  std::vector<std::string> identifiers;

  /// Informs the immediate parent rule (only useful for PredicateTerm) whether
  /// the Predicate is of the form `id ~ c` or `c ~ id`, essentially to change
  /// it to the canonical form `id ~ c`.
  std::optional<PredicateType> predicate_type;
  /// Informs the parent PredicateTerm what comparison operation is used in the
  /// predicate. It is used in conjunction with the `predicate_type` field to
  /// construct a valid `ast::Predicate`.
  std::optional<ast::ComparisonOp> comparison_type;

  /// A list of Terms parsed in the local context. For example, for an N-ary
  /// operation like And and Or, we expect the list to have at least 2 valid
  /// `ast::Expr`. This is populated when a local context is popped off the
  /// stack of a Term within the current rule.
  std::vector<ast::Expr> terms;
};

/// This maintains the global list of formulas and assertions that have been
/// parsed within the specification.
struct GlobalParserState {
  std::map<std::string, ast::Expr> formulas;
  std::map<std::string, ast::Expr> assertions;
};

template <typename Rule>
struct action : peg::nothing<Rule> {};

/// For each rule, we will assume that the top level function that calls this
/// action passes a reference to a `Specification` that we can populate, along
/// with other internal states.

template <>
struct action<peg::identifier> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalParserState&, ParserState& state) {
    auto id = std::string(in.string());
    state.identifiers.push_back(id);
  }
};

template <>
struct action<KwTrue> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.primitive_result = PrimitiveState(true);
  }
};

template <>
struct action<KwFalse> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.primitive_result = PrimitiveState(false);
  }
};

template <>
struct action<BooleanLiteral> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // This function is called only if the BooleanLiteral rule passes,
    // otherwise it is a parsing failure.
    if (state.primitive_result.has_value()) {
      bool primitive_val     = std::get<bool>(state.primitive_result.value());
      state.result           = Const(primitive_val);
      state.primitive_result = std::nullopt;
    }
  }
};

template <>
struct action<IntegerLiteral> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalParserState&, ParserState& state) {
    long int val           = std::stol(in.string());
    state.primitive_result = PrimitiveState(val);
  }
};

template <>
struct action<DoubleLiteral> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalParserState&, ParserState& state) {
    double val             = std::stod(in.string());
    state.primitive_result = PrimitiveState(val);
  }
};

template <>
struct action<LtSymbol> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.comparison_type = ast::ComparisonOp::LT;
  }
};

template <>
struct action<LeSymbol> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.comparison_type = ast::ComparisonOp::LE;
  }
};

template <>
struct action<GtSymbol> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.comparison_type = ast::ComparisonOp::GT;
  }
};

template <>
struct action<GeSymbol> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.comparison_type = ast::ComparisonOp::GE;
  }
};

template <>
struct action<PredicateForm1> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.predicate_type = PredicateType::ONE;
  }
};

template <>
struct action<PredicateForm2> {
  static void apply0(GlobalParserState&, ParserState& state) {
    state.predicate_type = PredicateType::TWO;
  }
};

template <>
struct action<PredicateTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // Here, we assume that the parser succeeded in parsing 1 identifier and 1
    // numeral in the subtree, and now we can get their results from state.
    // Moreover, we assume that the sub-expressions set the predicate type and
    // the comparison type.

    if (state.identifiers.empty()) {
      throw std::logic_error("Predicate sub-parser did not parse any identifiers");
    }
    auto id = state.identifiers.back(); // Get the last identifier.
    state.identifiers.pop_back();       // Remove it from the list.
    auto comparison_type  = state.comparison_type.value();
    state.comparison_type = std::nullopt;

    double const_val = 0.0;
    if (auto pval = std::get_if<long int>(&state.primitive_result.value())) {
      const_val = static_cast<double>(*pval);
    } else {
      const_val = std::get<double>(state.primitive_result.value());
    }
    state.primitive_result = std::nullopt;

    if (state.predicate_type == PredicateType::TWO) {
      // For Type TWO, we need to flip the direction of the comparison
      switch (state.comparison_type.value()) {
        case ast::ComparisonOp::LT:
          comparison_type = ast::ComparisonOp::GT;
          break;
        case ast::ComparisonOp::LE:
          comparison_type = ast::ComparisonOp::GE;
          break;
        case ast::ComparisonOp::GT:
          comparison_type = ast::ComparisonOp::LT;
          break;
        case ast::ComparisonOp::GE:
          comparison_type = ast::ComparisonOp::LE;
          break;
      }
    }
    state.predicate_type = std::nullopt;

    state.result = ast::Predicate{id, comparison_type, const_val};
  }
};

template <>
struct action<NotTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    assert(!state.terms.empty());
    // Here we assume that the NotTerm rule matched a single Term as a
    // subexpression. This implies that there should be exactly 1 Expr in
    // states.terms
    state.result = ::signal_tl::Not(state.terms.back());
    // We will remove that term after using it, and set Not(term) as the resultant
    // state.
    state.terms.pop_back();
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<AndTerm> : peg::require_apply {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalParserState&, ParserState& state) {
    if (state.terms.size() < 2) {
      throw peg::parse_error(
          std::string("expected at least 2 terms, got ") +
              std::to_string(state.terms.size()),
          in);
    }
    // We expect the state to contain at least 2 terms due to pegtl::rep_min
    assert(state.terms.size() >= 2);
    // Then, we just std::move the vector of terms into the And expression.
    state.result = ::signal_tl::And(std::move(state.terms));
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<OrTerm> : peg::require_apply {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalParserState&, ParserState& state) {
    if (state.terms.size() < 2) {
      throw peg::parse_error(
          std::string("expected at least 2 terms, got ") +
              std::to_string(state.terms.size()),
          in);
    }
    // We expect the state to contain at least 2 terms due to pegtl::rep_min
    assert(state.terms.size() >= 2);
    // Then, we just std::move the vector of terms into the Or expression.
    state.result = ::signal_tl::Or(std::move(state.terms));
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<ImpliesTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // We expect the state to contain exactly 2 terms
    assert(state.terms.size() == 2);
    // TODO(anand): Verify the order of the Terms.
    auto rhs = state.terms.back();
    state.terms.pop_back();
    auto lhs = state.terms.back();
    state.terms.pop_back();
    state.result = ::signal_tl::Implies(lhs, rhs);
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<IffTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // We expect the state to contain exactly 2 terms
    assert(state.terms.size() == 2);
    auto rhs = state.terms.back();
    state.terms.pop_back();
    auto lhs = state.terms.back();
    state.terms.pop_back();
    state.result = ::signal_tl::Iff(lhs, rhs);
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<XorTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // We expect the state to contain exactly 2 terms
    assert(state.terms.size() == 2);
    auto rhs = state.terms.back();
    state.terms.pop_back();
    auto lhs = state.terms.back();
    state.terms.pop_back();
    state.result = ::signal_tl::Xor(lhs, rhs);
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<AlwaysTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    assert(state.terms.size() == 1);
    // Here we assume that the AlwaysTerm rule matched a single Term as a
    // subexpression. This implies that there should be exactly 1 Expr in
    // states.terms
    // TODO(anand): Add support for intervals.
    state.result = ::signal_tl::Always(state.terms.back());
    // We will remove that term after using it, and set Not(term) as the resultant
    // state.
    state.terms.pop_back();
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<EventuallyTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    assert(state.terms.size() == 1);
    // Here we assume that the EventuallyTerm rule matched a single Term as a
    // subexpression. This implies that there should be exactly 1 Expr in
    // states.terms
    // TODO(anand): Add support for intervals.
    state.result = ::signal_tl::Eventually(state.terms.back());
    // We will remove that term after using it, and set Not(term) as the resultant
    // state.
    state.terms.pop_back();
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<UntilTerm> {
  static void apply0(GlobalParserState&, ParserState& state) {
    // We expect the state to contain exactly 2 terms
    assert(state.terms.size() == 2);
    auto rhs = state.terms.back();
    state.terms.pop_back();
    auto lhs = state.terms.back();
    state.terms.pop_back();
    state.result = ::signal_tl::Until(lhs, rhs);
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<Term> {
  template <
      typename Rule,
      peg::apply_mode A,
      peg::rewind_mode M,
      template <typename...>
      class Action,
      template <typename...>
      class Control,
      typename ParseInput>
  static bool
  match(ParseInput& in, GlobalParserState& global_state, ParserState& state) {
    // Here, we implement a push-down parser. Essentially, the new_state was
    // pushed onto the stack before the parser entered the rule for Term, and
    // now we have to pop the top of the stack (new_state) and merge the top
    // smartly with the old_state.

    // Create a new layer on the stack
    ParserState new_local_state{};
    new_local_state.level = state.level + 1;

    // Parse the input with the new state.
    bool ret = tao::pegtl::match<Rule, A, M, Action, Control>(
        in, global_state, new_local_state);
    // Once we are done parsing, we need to reset the states.
    // After the apply0 for Term was completed, new_state should have 1 Term in
    // the vector. This term needs to be moved onto the Terms vector in the
    // old_state.
    state.terms.insert(
        std::end(state.terms),
        std::begin(new_local_state.terms),
        std::end(new_local_state.terms));
    return ret;
  }

  static void apply0(GlobalParserState& global_state, ParserState& state) {
    // Here, we have two possibilities:
    //
    // 1. The Term was a valid expression; or
    // 2. The Term was an identifier.
    //
    // In the first case, once we have a resultant Expression, we should move
    // it to a vector so that it can be used by parent nodes in the AST.
    //
    // In the second case, we have to copy the expression pointed by the
    // identifier onto the terms vector.

    // If we have a Expression as the sub-result.
    if (state.result.has_value()) {
      // We move the result onto the vector of terms.
      state.terms.push_back(*state.result);
      // And invalidate the sub-result
      state.result = std::nullopt;
    } else if (!state.identifiers.empty()) { // And if we have an id
      // Copy the pointer to the formula with the corresponding id
      state.terms.push_back(global_state.formulas.at(state.identifiers.back()));
      state.identifiers.pop_back();
    } else {
      // Otherwise, it doesn't make sense that there are no results, as this
      // means that the parser failed. This should be unreachable.
      // LCOV_EXCL_START
      throw std::logic_error(
          "Should be unreachable, but looks like a Term has no sub expression or identifier.");
      // LCOV_EXCL_STOP
    }
  }
};

template <>
struct action<Assertion> {
  template <typename ActionInput>
  static void
  apply(const ActionInput& in, GlobalParserState& global_state, ParserState& state) {
    // For an assertion statement, we essentially will have 1 identifier, and 1
    // term. Thus, the identifier will be in the  result, and the number of
    // terms must be 1.
    assert(state.terms.size() == 1);
    assert(!state.identifiers.empty());

    auto id = state.identifiers.back(); // Must be there
    state.identifiers.pop_back();
    auto expr = state.terms.back(); // Must be there
    state.terms.pop_back();

    const auto [it, check] = global_state.assertions.insert({id, expr});
    if (!check) { // Unsuccessful insert. Probably due to id already being there
      throw peg::parse_error(
          fmt::format("possible redefinition of Assertion with id: \"{}\"", it->first),
          in);
    }

    assert(state.terms.empty());
  }
};

template <>
struct action<DefineFormula> {
  template <typename ActionInput>
  static void
  apply(const ActionInput& in, GlobalParserState& global_state, ParserState& state) {
    // For an define-formula command, we essentially will have 1 identifier,
    // and 1 term. Thus, the identifier will be in the  result, and the number
    // of terms must be 1.
    assert(state.terms.size() == 1);
    assert(!state.identifiers.empty());

    auto id = state.identifiers.back(); // Must be there
    state.identifiers.pop_back();
    auto expr = state.terms.back(); // Must be there
    state.terms.pop_back();

    const auto [it, check] = global_state.formulas.insert({id, expr});
    if (!check) { // Unsuccessful insert. Probably due to id already being there
      throw peg::parse_error(
          fmt::format("possible redefinition of Formula with id: \"{}\"", it->first),
          in);
    }

    assert(state.terms.empty());
  }
};

} // namespace actions

using actions::action;
} // namespace signal_tl::parser

#endif /* end of include guard: SIGNALTL_PARSER_ACTIONS_HPP */
