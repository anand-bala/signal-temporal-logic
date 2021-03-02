#include "argus/parser.hpp"
#include "actions.hpp"        // for ConstTypes, GlobalContext
#include "error_messages.hpp" // for control
#include "grammar.hpp"        // for grammar

#include <tao/pegtl.hpp> // for pegtl, parse, parse_error, position

#include <algorithm> // for max
#include <iomanip>   // for operator<<, setw
#include <iostream>  // for operator<<, endl, basic...
#include <stdexcept> // for invalid_argument
#include <utility>   // for move
#include <vector>    // for vector

/// Private namespace for the parser core.
namespace {

namespace grammar = argus::grammar;
namespace parser  = argus::parser;
namespace actions = argus::parser::actions;
namespace control = argus::parser::control;
namespace peg     = tao::pegtl;

/// This function takes in some PEGTL compliant input and outputs a concrete context.
///
/// To do this, it first generates a parse tree using PEGTL's in-built parse_tree
/// generator. Then, it transforms this parse_tree into the concrete context with the
/// abstract syntax tree.
template <typename ParseInput>
std::unique_ptr<argus::Context> _parse(ParseInput&& input) {
  auto global_state     = actions::GlobalContext{};
  auto top_local_state  = actions::TermContext{};
  top_local_state.level = 0;

  try {
    peg::parse<grammar::Specification, actions::action, control::control>(
        input, global_state, top_local_state);
  } catch (const peg::parse_error& e) {
    const auto p = e.positions().front();

    std::cerr << e.what() << std::endl
              << input.line_at(p) << '\n'
              << std::setw(p.column) << '^' << std::endl;
    return {};
  }

  auto ctx = std::make_unique<argus::Context>();

  ctx->defined_formulas = std::move(global_state.defined_formulas);
  ctx->monitors         = std::move(global_state.monitors);

  return ctx;
}

} // namespace

namespace argus {
std::unique_ptr<Context> Context::from_string(std::string_view input) {
  tao::pegtl::string_input in(input, "from_content");
  return _parse(in);
}

std::unique_ptr<Context> Context::from_file(const fs::path& input) {
  tao::pegtl::file_input in(input);
  return _parse(in);
}

} // namespace argus
