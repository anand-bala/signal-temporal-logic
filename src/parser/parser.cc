#include "signal_tl/parser.hpp"
#include "grammar.hpp"
#include "parser_errors.hpp"

#include <iostream>

#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/trace.hpp>

namespace signal_tl::parser {

std::unique_ptr<Specification> from_string(std::string_view input) {
  tao::pegtl::string_input in(input, "from_content");
  const auto r = tao::pegtl::parse<grammar::SpecificationFile>(in);
  std::cout << "Parsed string and returned = " << r << std::endl;
  return {};
}

std::unique_ptr<Specification> from_file(const stdfs::path& input) {
  tao::pegtl::file_input in(input);
  const auto r = tao::pegtl::parse<grammar::SpecificationFile>(in);
  std::cout << "Parsed file and returned = " << r << std::endl;
  return {};
}

} // namespace signal_tl::parser

// LCOV_EXCL_START
namespace signal_tl::grammar::internal {

size_t analyze(int verbose) {
  return tao::pegtl::analyze<SpecificationFile>(verbose);
}

bool trace_from_file(const stdfs::path& input_path) {
  tao::pegtl::file_input in(input_path);
  return tao::pegtl::
      standard_trace<SpecificationFile, peg::nothing, signal_tl::grammar::control>(in);
}
// LCOV_EXCL_STOP

} // namespace signal_tl::grammar::internal
