#include "signal_tl/parser.hpp"
#include "actions.hpp"
#include "error_messages.hpp" // for control
#include "grammar.hpp"        // for SpecificationFile

#include <tao/pegtl/contrib/analyze.hpp> // for analyze
#include <tao/pegtl/contrib/trace.hpp>   // for standard_trace
#include <tao/pegtl/nothing.hpp>         // for nothing
#include <tao/pegtl/parse.hpp>           // for parse

#include <stdexcept> // for logic_error

namespace {

namespace peg = tao::pegtl;
using namespace signal_tl;

template <typename ParseInput>
std::unique_ptr<Specification> _parse(ParseInput&& input) {
  bool success =
      peg::parse<grammar::SpecificationFile, parser::action, parser::control>(input);
  if (success) {
    return {};
  } else {
    throw std::logic_error(
        "Local error thrown during parsing of input. This is most likely a bug.");
  }
}
} // namespace

namespace signal_tl::parser {

std::unique_ptr<Specification> from_string(std::string_view input) {
  tao::pegtl::string_input in(input, "from_content");
  return _parse(in);
}

std::unique_ptr<Specification> from_file(const stdfs::path& input) {
  tao::pegtl::file_input in(input);
  return _parse(in);
}

} // namespace signal_tl::parser

// LCOV_EXCL_START
namespace signal_tl::grammar::internal {

size_t analyze(int verbose) {
  return peg::analyze<SpecificationFile>(verbose);
}

bool trace_from_file(const stdfs::path& input_path) {
  peg::file_input in(input_path);
  return peg::standard_trace<SpecificationFile, parser::action, parser::control>(in);
}
// LCOV_EXCL_STOP

} // namespace signal_tl::grammar::internal
