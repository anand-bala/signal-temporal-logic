#pragma once

#ifndef ARGUS_PARSER_ACTIONS_HPP
#define ARGUS_PARSER_ACTIONS_HPP

#include "argus/ast/expression.hpp"

#include "grammar.hpp"
#include "utils/static_analysis_helpers.hpp"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <tao/pegtl.hpp>

#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

/// Here, we will define the custom actions for the PEG parser that will
/// convert each rule into a valid AST class.
namespace argus::parser::actions {
/* helpers */

/// Constant types
using ConstTypes =
    std::variant<std::string, double, long long int, unsigned long long int, bool>;

/// Move the contents of an optional, and invalidate the optional.
///
/// Doesn't check if the optional has contents.
template <typename T>
T remove_opt_quick(std::optional<T>& opt) {
  T content = std::move(*opt);
  opt       = std::nullopt;
  return content;
}

namespace peg = tao::pegtl;
namespace gm  = argus::grammar;

/// This encodes local state of the push-down parser for Terms.
struct TermContext {
  /// Purely here for debugging purposes.
  unsigned long long int level;

  /// List of parsed Constants
  ///
  /// Used immediately by either Term or AttributeValue
  std::vector<ConstTypes> constants;

  /// A Symbol. Needs to be immediately consumed by a parent rule.
  std::optional<std::string> symbol;

  /// Keyword (`:<keyword>`). Consumed immediately in Attributes.
  std::optional<std::string> keyword;

  /// List of parsed attributes
  std::vector<ast::Attribute> attributes;

  /// An identifier as parsed by the rule.
  ///
  /// This is immediately used by the parent rule (either in Term or in some Command) to
  /// either create new formulas/assertions or to map existing formulas/assertions to a
  /// valid `ast::Expr`.
  std::optional<std::string> identifier;

  /// Holds the variable name. Used immediately in VarDecl.
  std::optional<std::string> var_name;
  /// Holds the variable type. This is used immediately in a VarDecl rule.
  std::optional<std::string> var_type;
  /// A list of Variables (Terms) declared in the local context.
  ///
  /// Created in VarName with type `VarType::Unknown`. The parent rule will determine
  /// the type and the scope of the variable.
  /// Used by Pinning and Quantifier, and will be consumed immediately by the parent
  /// rule.
  std::vector<ExprPtr> variables;

  /// An interval command. Error if there are more than 1 in an Term.
  std::shared_ptr<ast::details::Interval> interval;

  /// An operation string. Used immediately by the Expression term.
  std::optional<std::string> operation;

  /// Whenever an Expression is completed, this field gets populared. Later,
  /// within the action for the Term rule, we move the result onto the vector
  /// `terms` to allow for the parent expression to easily combine it with
  /// their list of `terms`.
  std::unique_ptr<argus::Expr> result;
  /// A list of Terms parsed in the local context. For example, for an N-ary
  /// operation like And and Or, we expect the list to have at least 2 valid
  /// `ast::Expr`. This is populated when a local context is popped off the
  /// stack of a Term within the current rule.
  std::vector<ExprPtr> terms;
};

/// This maintains the global list of formulas and assertions that have been
/// parsed within the specification.
struct GlobalContext {
  /// List of defined constants
  std::map<std::string, ExprPtr> constants;
  /// List of defined Signals
  std::map<std::string, ExprPtr> signals;
  /// List of defined Parameters
  std::map<std::string, ExprPtr> parameters;

  /// List of defined formulas, keyed by their corresponding identifiers.
  std::map<std::string, ExprPtr> defined_formulas;
  /// List of settings for monitors, keyed by their corresponding identifiers.
  std::map<std::string, ExprPtr> monitors;

  /// List of global settings
  std::set<ast::Attribute, ast::Attribute::KeyCompare> settings;
};

template <typename Rule>
struct action : peg::nothing<Rule> {};

template <>
struct action<gm::KwTrue> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    ctx.constants.emplace_back(true);
  }
};

template <>
struct action<gm::KwFalse> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    ctx.constants.emplace_back(false);
  }
};

template <>
struct action<gm::BinInt> {
  static constexpr int base = 2;
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    auto val = std::stoll(in.string(), /*pos*/ 0, base);
    if (val >= 0) {
      ctx.constants.emplace_back(static_cast<unsigned long long>(val));
    } else {
      ctx.constants.emplace_back(val);
    }
  }
};

template <>
struct action<gm::OctInt> {
  static constexpr int base = 8;
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    auto val = std::stoll(in.string(), /*pos*/ 0, base);
    if (val >= 0) {
      ctx.constants.emplace_back(static_cast<unsigned long long>(val));
    } else {
      ctx.constants.emplace_back(val);
    }
  }
};

template <>
struct action<gm::HexInt> {
  static constexpr int base = 16;
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    auto val = std::stoll(in.string(), /*pos*/ 0, base);
    if (val >= 0) {
      ctx.constants.emplace_back(static_cast<unsigned long long>(val));
    } else {
      ctx.constants.emplace_back(val);
    }
  }
};

template <>
struct action<gm::DecInt> {
  static constexpr int base = 10;
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    auto val = std::stoll(in.string(), /*pos*/ 0, base);
    if (val >= 0) {
      ctx.constants.emplace_back(static_cast<unsigned long long>(val));
    } else {
      ctx.constants.emplace_back(val);
    }
  }
};

template <>
struct action<gm::DoubleLiteral> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    auto val = std::stod(in.string());
    ctx.constants.emplace_back(val);
  }
};

template <>
struct action<gm::StringLiteral> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    // Get the quoted content.
    std::string content = in.string();
    // Check if there is a " on either end and trim it.
    {
      size_t begin = 0, count = std::string::npos;
      if (content.front() == '"') {
        begin = 1;
      }
      if (content.back() == '"') {
        count = content.size() - begin - 1;
      }
      content = content.substr(begin, count);
    }
    ctx.constants.emplace_back(content);
  }
};

template <>
struct action<gm::SimpleSymbol> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    ctx.symbol = in.string();
  }
};

template <>
struct action<gm::QuotedSymbol> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    std::string content = in.string();
    // We need to trim the '|' on either ends.
    {
      size_t begin = 0, count = std::string::npos;
      if (content.front() == '|') {
        begin = 1;
      }
      if (content.back() == '|') {
        count = content.size() - begin - 1;
      }
      content = content.substr(begin, count);
    }
    ctx.symbol = content;
  }
};

template <>
struct action<gm::Keyword> {
  static void apply0(GlobalContext&, TermContext& state) {
    utils::assert_(
        state.symbol.has_value(), "Expected at least 1 symbol to parse as a keyword");
    utils::assert_(!state.keyword.has_value(), "Keyword seems to already have a value");
    state.keyword = remove_opt_quick(state.symbol);
  }
};

template <>
struct action<gm::Attribute> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.keyword.has_value(), "Expected a keyword to be parsed for attributes");
    std::string key = remove_opt_quick(ctx.keyword);
    auto vals       = std::move(ctx.constants);
    ctx.attributes.push_back(ast::Attribute{key, std::move(vals)});
  }
};

template <>
struct action<gm::QualifiedIdentifier> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.symbol.has_value(), "Expected a symbol to parse as a qualified identifier");
    utils::assert_(!ctx.identifier.has_value(), "Identifier already populated.");
    ctx.identifier = remove_opt_quick(ctx.symbol);
  }
};

template <>
struct action<gm::VarName> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.symbol.has_value(), "Expected a symbol to parse as a Variable name");
    ctx.var_name = remove_opt_quick(ctx.symbol);
  }
};

template <>
struct action<gm::VarType> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.symbol.has_value(), "Expected a symbol to parse as a Variable type");
    ctx.var_type = remove_opt_quick(ctx.symbol);
  }
};

template <>
struct action<gm::VarDecl> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.var_name.has_value(), "Expected a variable name to have been parsed");
    std::string var_name     = remove_opt_quick(ctx.var_name);
    std::string var_type_str = remove_opt_quick(ctx.var_type);
    auto var_type            = magic_enum::enum_cast<ast::VarType>(var_type_str);
    if (!var_type.has_value()) {
      throw peg::parse_error(
          fmt::format("Unknown Variable type: `{}`", var_type_str), in);
    }
    auto variable = Expr::Variable(var_name, *var_type);
    ctx.variables.push_back(std::move(variable));
  }
};

template <>
struct action<gm::Term> {
  template <
      typename Rule,
      peg::apply_mode A,
      peg::rewind_mode M,
      template <typename...>
      class Action,
      template <typename...>
      class Control,
      typename ParseInput>
  static bool match(ParseInput& in, GlobalContext& global_ctx, TermContext& ctx) {
    // Here, we implement a push-down parser. Essentially, the new_ctx was
    // pushed onto the stack before the parser entered the rule for Term, and
    // now we have to pop the top of the stack (new_ctx) and merge the top
    // smartly with the old_ctx.

    // Create a new layer on the stack
    auto new_local_ctx  = TermContext{};
    new_local_ctx.level = ctx.level + 1;

    // Parse the input with the new ctx.
    bool ret =
        tao::pegtl::match<Rule, A, M, Action, Control>(in, global_ctx, new_local_ctx);
    // Once we are done parsing, we need to reset the ctxs.
    // After the apply0 for Term was completed, new_ctx should have 1 Term in
    // the vector. This term needs to be moved onto the Terms vector in the
    // old_ctx.
    ctx.terms.insert(
        std::end(ctx.terms),
        std::begin(new_local_ctx.terms),
        std::end(new_local_ctx.terms));
    // We also need to move the interval expression to the current context
    if (ctx.interval != nullptr && new_local_ctx.interval != nullptr) {
      throw peg::parse_error(
          "Multiple interval expressions defined for the same term", in);
    } else if (new_local_ctx.interval) {
      ctx.interval = std::move(new_local_ctx.interval);
    }
    return ret;
  }

  template <typename ActionInput>
  static void
  apply(const ActionInput& in, GlobalContext& global_ctx, TermContext& ctx) {
    utils::assert_(
        ctx.terms.empty(),
        fmt::format("Expected 0 intermediary Terms, got {}", ctx.terms.size()));
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

    // If we have an Expression as the sub-result.
    if (ctx.result) {
      // We move the result onto the vector of terms.
      ctx.terms.push_back(std::move(ctx.result));
    } else if (ctx.identifier.has_value()) { // And if we have an id
      // Copy the pointer to the formula with the corresponding id
      const std::string id = remove_opt_quick(ctx.identifier);
      const auto it        = global_ctx.defined_formulas.find(id);
      if (it == global_ctx.defined_formulas.end()) {
        throw peg::parse_error(
            fmt::format("Reference to unknown identifier: `{}`", id), in);
      } else {
        ctx.terms.push_back(it->second);
      }
    } else if (!ctx.constants.empty()) { // We have a constant
      utils::assert_(
          ctx.constants.size() == 1,
          fmt::format("Expected exactly 1 constant, got {}", ctx.constants.size()));

      ConstTypes val = std::move(ctx.constants.back());
      ctx.constants.pop_back();
      ctx.terms.push_back(Expr::Constant(val));
    } else if (ctx.interval != nullptr) {
      // If we parsed an interval expression
    } else {
      // This is reachable if there is an empty "()" in the list of expressions. This
      // means that we need to throw a parsing error, where we matched an empty list
      // where it never makes sense to have one.
      throw peg::parse_error(
          "Looks like a pair '(' ')' was matched with nothing in between", in);
      // utils::unreachable("Looks like a Term has no sub expression or identifier.");
    }
  }
};

template <>
struct action<gm::IntervalExpression> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.terms.size() == 1, "Expected exactly 2 terms to parse as Interval bounds");
    // If an interval already exists in the current context. Not technically possible
    // here, but it needs to be checked properly in Terms.
    utils::assert_(
        ctx.interval != nullptr,
        "Attempting to specify multiple intervals to the same operation");
    // Move the terms here.
    std::vector<ExprPtr> terms = std::move(ctx.terms);
    // Create a Interval
    try {
      ctx.interval = std::make_shared<ast::details::Interval>(terms.at(0), terms.at(1));
    } catch (const std::invalid_argument& e) {
      throw peg::parse_error(fmt::format("Invalid interval: {}", e.what()), in);
    }
  }
};

template <>
struct action<gm::Operation> {
  static void apply0(GlobalContext&, TermContext& ctx) {
    utils::assert_(
        ctx.symbol.has_value(), "Expected a symbol to parse as an operation");
    utils::assert_(!ctx.operation.has_value(), "Operation already populated.");
    ctx.operation = remove_opt_quick(ctx.symbol);
  }
};

enum struct KnownOp {
  LT,
  LE,
  GT,
  GE,
  EQ,
  NEQ,
  NOT,
  AND,
  OR,
  IMPLIES,
  IFF,
  XOR,
  NEXT,
  PREV,
  EVENTUALLY,
  ONCE,
  ALWAYS,
  HISTORICALLY,
  UNTIL,
  SINCE,
  ADD,
  SUB,
  MUL,
  DIV,
};

template <>
struct action<gm::OperationExpression> {
  static std::optional<KnownOp> lookup(std::string_view op) {
    // clang-format off
    if ( op == "lt" || op == "<"   )  { return KnownOp::LT;           }
    if ( op == "le" || op == "<="  )  { return KnownOp::LE;           }
    if ( op == "gt" || op == ">"   )  { return KnownOp::GT;           }
    if ( op == "ge" || op == ">="  )  { return KnownOp::GE;           }
    if ( op == "eq" || op == "=="  )  { return KnownOp::EQ;           }
    if ( op == "neq" || op == "!=" )  { return KnownOp::NEQ;          }
    if ( op == "not"               )  { return KnownOp::NOT;          }
    if ( op == "and"               )  { return KnownOp::AND;          }
    if ( op == "or"                )  { return KnownOp::OR;           }
    if ( op == "next"              )  { return KnownOp::NEXT;         }
    if ( op == "previous"          )  { return KnownOp::PREV;         }
    if ( op == "eventually"        )  { return KnownOp::EVENTUALLY;   }
    if ( op == "once"              )  { return KnownOp::ONCE;         }
    if ( op == "always"            )  { return KnownOp::ALWAYS;       }
    if ( op == "historically"      )  { return KnownOp::HISTORICALLY; }
    if ( op == "until"             )  { return KnownOp::UNTIL;        }
    if ( op == "since"             )  { return KnownOp::SINCE;        }
    if ( op == "add" || op == "+"  )  { return KnownOp::ADD;          }
    if ( op == "sub" || op == "-"  )  { return KnownOp::SUB;          }
    if ( op == "mul" || op == "*"  )  { return KnownOp::MUL;          }
    if ( op == "div" || op == "/"  )  { return KnownOp::DIV;          }
    // clang-format on
    return {};
  }

  template <typename ActionInput>
  static void handle_predicate(const ActionInput& in, TermContext& ctx, KnownOp op) {
    // 1. Check if we have 2 terms.
    if (ctx.terms.size() != 2) {
      throw peg::parse_error(
          fmt::format(
              "Predicate expects 2 arguments, an LHS and an RHS, got {}",
              ctx.terms.size()),
          in);
    }
    // 2. Load the args. Clear `terms`
    auto terms  = std::move(ctx.terms);
    ExprPtr lhs = std::move(terms[0]);
    ExprPtr rhs = std::move(terms[1]);
    // 3. Create result
    try {
      switch (op) {
        case KnownOp::LT:
          ctx.result = Expr::Lt(std::move(lhs), std::move(rhs));
          return;
        case KnownOp::LE:
          ctx.result = Expr::Le(std::move(lhs), std::move(rhs));
          return;
        case KnownOp::GT:
          ctx.result = Expr::Gt(std::move(lhs), std::move(rhs));
          return;
        case KnownOp::GE:
          ctx.result = Expr::Ge(std::move(lhs), std::move(rhs));
          return;
        case KnownOp::EQ:
          ctx.result = Expr::Eq(std::move(lhs), std::move(rhs));
          return;
        case KnownOp::NEQ:
          ctx.result = Expr::Neq(std::move(lhs), std::move(rhs));
          return;
        default:
          utils::unreachable();
      }
    } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
  }

  template <typename ActionInput>
  static void handle_unary(const ActionInput& in, TermContext& ctx, KnownOp op) {
    // 1. Check if we have 1 term.
    if (ctx.terms.size() != 1) {
      throw peg::parse_error(
          fmt::format("Unary operation expects 1 argument, got {}", ctx.terms.size()),
          in);
    }
    // 2. Load the arg. Clear `terms`.
    //    Load the options.
    auto terms  = std::move(ctx.terms);
    ExprPtr arg = std::move(terms[0]);
    auto attrs  = std::move(ctx.attributes);
    // 2.1 Can't have any intervals here
    if (ctx.interval != nullptr) {
      throw peg::parse_error(
          "Operation is not temporal an doesn't support Intervals", in);
    }
    // 3. Create result
    try {
      switch (op) {
        case KnownOp::NOT:
          ctx.result = Expr::Not(std::move(arg));
          return;
        case KnownOp::NEXT:
          ctx.result = Expr::Next(std::move(arg));
          return;
        case KnownOp::PREV:
          ctx.result = Expr::Previous(std::move(arg));
          return;
        default:
          utils::unreachable();
      }
    } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
  }

  template <typename ActionInput>
  static void handle_binary(const ActionInput& in, TermContext& ctx, KnownOp op) {
    // 1. Chck if we have exactly 2 terms
    if (ctx.terms.size() == 2) {
      throw peg::parse_error(
          fmt::format(
              "Binary operation expects exactly 2 arguments, got {}", ctx.terms.size()),
          in);
    }
    // 2. Load the args. (and the options)
    ExprPtr arg0 = std::move(ctx.terms[0]);
    ExprPtr arg1 = std::move(ctx.terms[1]);
    // 2.1 Can't have any intervals here
    if (ctx.interval != nullptr) {
      throw peg::parse_error(
          "Operation is not temporal an doesn't support Intervals", in);
    }
    // 3. Create the result
    try {
      switch (op) {
        case KnownOp::IMPLIES:
          ctx.result = Expr::Implies(arg0, arg1);
          return;
        case KnownOp::IFF:
          ctx.result = Expr::Iff(arg0, arg1);
          return;
        case KnownOp::XOR:
          ctx.result = Expr::Xor(arg0, arg1);
          return;
        case KnownOp::SUB:
          ctx.result = Expr::Subtract(arg0, arg1);
          return;
        case KnownOp::DIV:
          ctx.result = Expr::Div(arg0, arg1);
          return;
        default:
          utils::unreachable();
      }
    } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
  }

  template <typename ActionInput>
  static void handle_nary(const ActionInput& in, TermContext& ctx, KnownOp op) {
    // 1. Check if we have at least 2 terms.
    if (ctx.terms.size() < 2) {
      throw peg::parse_error(
          fmt::format(
              "N-ary operation expects at least 2 arguments, got {}", ctx.terms.size()),
          in);
    }
    // 2. Load the args
    auto args = std::move(ctx.terms);
    // 2.1 Can't have any intervals here
    if (ctx.interval != nullptr) {
      throw peg::parse_error(
          "Operation is not temporal an doesn't support Intervals", in);
    }
    // 3. Create result
    try {
      switch (op) {
        case KnownOp::AND:
          ctx.result = Expr::And(std::move(args));
          return;
        case KnownOp::OR:
          ctx.result = Expr::Or(std::move(args));
          return;
        case KnownOp::ADD:
          ctx.result = Expr::Add(std::move(args));
          return;
        case KnownOp::MUL:
          ctx.result = Expr::Mul(std::move(args));
          return;
        default:
          utils::unreachable();
      }
    } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
  }

  template <typename ActionInput>
  static void
  handle_temporal_unary(const ActionInput& in, TermContext& state, KnownOp op) {
    // 1. Check if we have at least 2 terms.
    if (state.terms.size() == 1) {
      throw peg::parse_error(
          fmt::format(
              "Temporal Unary operation expects exactly 1 argument, got {}",
              state.terms.size()),
          in);
    }
    // 2. Load the args
    auto terms    = std::move(state.terms);
    ExprPtr arg   = std::move(terms[0]);
    auto interval = std::move(state.interval);

    // 3. Create result
    try {
      switch (op) {
        case KnownOp::EVENTUALLY:
          state.result = Expr::Eventually(std::move(arg), std::move(interval));
          return;
        case KnownOp::ONCE:
          state.result = Expr::Once(std::move(arg), std::move(interval));
          return;
        case KnownOp::ALWAYS:
          state.result = Expr::Always(std::move(arg), std::move(interval));
          return;
        case KnownOp::HISTORICALLY:
          state.result = Expr::Historically(std::move(arg), std::move(interval));
          return;
        default:
          utils::unreachable();
      }
    } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
  }

  template <typename ActionInput>
  static void
  handle_temporal_binary(const ActionInput& in, TermContext& state, KnownOp op) {
    // 1. Check if we have 2 terms
    if (state.terms.size() == 2) {
      throw peg::parse_error(
          fmt::format(
              "Temporal Binary operation expects exactly 2 argument, got {}",
              state.terms.size()),
          in);
    }
    // 2. Load the args
    auto terms    = std::move(state.terms);
    ExprPtr arg0  = std::move(terms[0]);
    ExprPtr arg1  = std::move(terms[1]);
    auto interval = std::move(state.interval);

    // 3. Create result
    switch (op) {
      case KnownOp::UNTIL:
        state.result =
            Expr::Until(std::move(arg0), std::move(arg1), std::move(interval));
        return;
      case KnownOp::SINCE:
        state.result =
            Expr::Since(std::move(arg0), std::move(arg1), std::move(interval));
        return;
      default:
        utils::unreachable();
    }
  }

  template <typename ActionInput>
  static void apply(const ActionInput& in, GlobalContext&, TermContext& state) {
    utils::assert_(
        state.operation.has_value(), "Expected a symbol for the function operation");
    utils::assert_(state.terms.size() >= 1, "Expected at least 1 Term");

    std::string op_str        = remove_opt_quick(state.operation);
    std::optional<KnownOp> op = lookup(op_str);

    if (!op.has_value()) {
      auto options = std::set<ast::Attribute, ast::Attribute::KeyCompare>{
          state.attributes.begin(), state.attributes.end()};
      try {
        state.result = Expr::Function(
            ast::FnType::Custom, std::move(state.terms), std::move(options));
      } catch (const std::invalid_argument& e) { throw peg::parse_error(e.what(), in); }
      return;
    }

    switch (*op) {
      // Predicates
      case KnownOp::LT:
      case KnownOp::LE:
      case KnownOp::GT:
      case KnownOp::GE:
      case KnownOp::EQ:
      case KnownOp::NEQ:
        handle_predicate(in, state, *op);
        break;
      // Unary ops
      case KnownOp::NOT:
      case KnownOp::NEXT:
      case KnownOp::PREV:
        handle_unary(in, state, *op);
        break;
      // Binary operations
      case KnownOp::IMPLIES:
      case KnownOp::IFF:
      case KnownOp::XOR:
      case KnownOp::SUB:
      case KnownOp::DIV:
        handle_binary(in, state, *op);
        break;
      // Nary operations
      case KnownOp::AND:
      case KnownOp::OR:
      case KnownOp::ADD:
      case KnownOp::MUL:
        handle_nary(in, state, *op);
        break;
      // Temporal unary operations.
      case KnownOp::EVENTUALLY:
      case KnownOp::ONCE:
      case KnownOp::ALWAYS:
      case KnownOp::HISTORICALLY:
        handle_temporal_unary(in, state, *op);
        break;
      // Temporal binary operations.
      case KnownOp::UNTIL:
      case KnownOp::SINCE:
        handle_temporal_binary(in, state, *op);
    }

    // 4. Clear the attributes and interval.
    state.attributes.clear();
    state.interval.reset();
  }
};

template <>
struct action<gm::CmdSetOption> {
  static void apply0(GlobalContext& global_ctx, TermContext& ctx) {
    utils::assert_(!ctx.attributes.empty(), "Expected at least 1 option");
    auto attrs          = std::move(ctx.attributes);
    global_ctx.settings = std::set<ast::Attribute, ast::Attribute::KeyCompare>{
        attrs.begin(), attrs.end()};
  }
};

template <>
struct action<gm::CmdDefineFormula> {
  static void apply0(GlobalContext& global_ctx, TermContext& ctx) {
    utils::assert_(
        ctx.identifier.has_value(), "Expected exactly 1 formula name identifier");
    utils::assert_(ctx.terms.size() == 1, "Expected exactly 1 expression argument");

    auto terms     = std::move(ctx.terms);
    std::string id = remove_opt_quick(ctx.identifier);

    auto expr = std::move(terms[0]);

    global_ctx.defined_formulas[id] = std::move(expr);
  }
};

template <>
struct action<gm::valid_commands> {
  template <
      typename Rule,
      peg::apply_mode A,
      peg::rewind_mode M,
      template <typename...>
      class Action,
      template <typename...>
      class Control,
      typename ParseInput>
  static bool match(ParseInput& in, GlobalContext& global_state, TermContext&) {
    // We create a new local state for each top-level command. This way, we don't have
    // any pollution of scopes between commands (due to dev error).
    TermContext state{};
    state.level = 0;

    return tao::pegtl::match<Rule, A, M, Action, Control>(in, global_state, state);
  }

  static void apply0(GlobalContext&, TermContext&) {
    // Kept empty to satisfy PEGTL
  }
};

} // namespace argus::parser::actions

#endif /* end of include guard: ARGUS_PARSER_ACTIONS_HPP */
