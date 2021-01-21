#pragma once

#ifndef SIGNALTL_PARSER_ACTIONS_HPP
#define SIGNALTL_PARSER_ACTIONS_HPP

#include "grammar.hpp"
#include "signal_tl/ast.hpp"
#include <tao/pegtl.hpp> // IWYU pragma: keep

#include <fmt/core.h>

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

struct ParserState {
  std::optional<ast::Expr> result;

  std::optional<PrimitiveState> primitive_result;

  std::vector<std::string> identifiers;

  std::optional<PredicateType> predicate_type;
  std::optional<ast::ComparisonOp> comparison_type;

  std::vector<ast::Expr> terms;

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
  static void apply(const ActionInput& in, ParserState& state) {
    auto id = std::string(in.string());
    state.identifiers.push_back(id);
  }
};

template <>
struct action<KwTrue> {
  static void apply0(ParserState& state) {
    state.primitive_result = PrimitiveState(true);
  }
};

template <>
struct action<KwFalse> {
  static void apply0(ParserState& state) {
    state.primitive_result = PrimitiveState(false);
  }
};

template <>
struct action<BooleanLiteral> {
  static void apply0(ParserState& state) {
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
  static void apply(const ActionInput& in, ParserState& state) {
    long int val           = std::stol(in.string());
    state.primitive_result = PrimitiveState(val);
  }
};

template <>
struct action<DoubleLiteral> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, ParserState& state) {
    double val             = std::stod(in.string());
    state.primitive_result = PrimitiveState(val);
  }
};

template <>
struct action<LtSymbol> {
  static void apply0(ParserState& state) {
    state.comparison_type = ast::ComparisonOp::LT;
  }
};

template <>
struct action<LeSymbol> {
  static void apply0(ParserState& state) {
    state.comparison_type = ast::ComparisonOp::LE;
  }
};

template <>
struct action<GtSymbol> {
  static void apply0(ParserState& state) {
    state.comparison_type = ast::ComparisonOp::GT;
  }
};

template <>
struct action<GeSymbol> {
  static void apply0(ParserState& state) {
    state.comparison_type = ast::ComparisonOp::GE;
  }
};

template <>
struct action<PredicateForm1> {
  static void apply0(ParserState& state) {
    state.predicate_type = PredicateType::ONE;
  }
};

template <>
struct action<PredicateForm2> {
  static void apply0(ParserState& state) {
    state.predicate_type = PredicateType::TWO;
  }
};

template <>
struct action<PredicateTerm> {
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
    assert(state.terms.size() > 1);
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
struct action<AndTerm> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, ParserState& state) {
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
struct action<OrTerm> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
      state.terms.push_back(state.formulas.at(state.identifiers.back()));
      state.identifiers.pop_back();
    } else {
      // Otherwise, it doesn't make sense that there are no results, as this
      // means that the parser failed. This should be unreachable.
      throw std::logic_error(
          "Should be unreachable, but looks like a Term has no sub expression or identifier.");
    }
  }
};

template <>
struct action<Assertion> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, ParserState& state) {
    // For an assertion statement, we essentially will have 1 identifier, and 1
    // term. Thus, the identifier will be in the  result, and the number of
    // terms must be 1.
    assert(state.terms.size() == 1);
    assert(!state.identifiers.empty());

    auto id = state.identifiers.back(); // Must be there
    state.identifiers.pop_back();
    auto expr = state.terms.back(); // Must be there
    state.terms.pop_back();

    const auto [it, check] = state.assertions.insert({id, expr});
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
  static void apply(const ActionInput& in, ParserState& state) {
    // For an define-formula command, we essentially will have 1 identifier,
    // and 1 term. Thus, the identifier will be in the  result, and the number
    // of terms must be 1.
    assert(state.terms.size() == 1);
    assert(!state.identifiers.empty());

    auto id = state.identifiers.back(); // Must be there
    state.identifiers.pop_back();
    auto expr = state.terms.back(); // Must be there
    state.terms.pop_back();

    const auto [it, check] = state.formulas.insert({id, expr});
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
