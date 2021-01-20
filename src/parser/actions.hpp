#pragma once

#ifndef SIGNALTL_PARSER_ACTIONS_HPP
#define SIGNALTL_PARSER_ACTIONS_HPP

#include "grammar.hpp"
#include <tao/pegtl.hpp> // IWYU pragma: keep

#include "signal_tl/ast.hpp"

#include <cassert>
#include <map>
#include <memory>
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
  ast::Expr result;

  PrimitiveState primitive_result;
  std::string identifier_result;
  PredicateType predicate_type;
  ast::ComparisonOp comparison_type;

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
    state.identifier_result = in.string();
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
    bool primitive_val = std::get<bool>(state.primitive_result);
    state.result       = Const(primitive_val);
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

    double const_val = 0.0;
    if (auto pval = std::get_if<long int>(&state.primitive_result)) {
      const_val = static_cast<double>(*pval);
    } else {
      const_val = std::get<double>(state.primitive_result);
    }
    auto comparison_type = state.comparison_type;

    if (state.predicate_type == PredicateType::TWO) {
      // For Type TWO, we need to flip the direction of the comparison
      switch (state.comparison_type) {
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
    state.result = ast::Predicate(state.identifier_result, comparison_type, const_val);
  }
};

template <>
struct action<NotTerm> {
  static void apply0(ParserState& state) {
    assert(state.terms.size() == 1);
    // Here we assume that the NotTerm rule matched a single Term as a
    // subexpression. This implies that there should be exactly 1 Expr in
    // states.terms
    state.result = ::signal_tl::Not(std::move(state.terms.back()));
    // We will remove that term after using it, and set Not(term) as the resultant
    // state.
    state.terms.pop_back();
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<AndTerm> {
  static void apply0(ParserState& state) {
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
  static void apply0(ParserState& state) {
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
    auto rhs = std::move(state.terms.back());
    state.terms.pop_back();
    auto lhs = std::move(state.terms.back());
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
    auto rhs = std::move(state.terms.back());
    state.terms.pop_back();
    auto lhs = std::move(state.terms.back());
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
    auto rhs = std::move(state.terms.back());
    state.terms.pop_back();
    auto lhs = std::move(state.terms.back());
    state.terms.pop_back();
    state.result = ::signal_tl::Xor(lhs, rhs);
    // There should be exactly 0 terms left now.
    assert(state.terms.empty());
  }
};

template <>
struct action<Term> {
  static void apply0(ParserState& state) {
    // Once we have a resultant Term, we should move it to a vector so that it
    // can be used by parent nodes in the AST.
    state.terms.emplace_back(std::move(state.result));
  }
};

} // namespace actions

using actions::action;
} // namespace signal_tl::parser

#endif /* end of include guard: SIGNALTL_PARSER_ACTIONS_HPP */
