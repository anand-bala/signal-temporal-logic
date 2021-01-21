#include "signal_tl/fmt.hpp"
#include "signal_tl/internal/filesystem.hpp"
#include "signal_tl/parser.hpp"
#include "signal_tl/signal_tl.hpp"

#include <fmt/format.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/trace.hpp>

#include <iostream>
#include <string>
#include <string_view>

namespace pegtl = tao::pegtl;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Invalid number of arguments. Need input file" << std::endl;
    return 1;
  }

  pegtl::file_input in(argv[1]);
  auto input_path = stdfs::path(argv[1]);
  try {
    // signal_tl::grammar::internal::trace_from_file(input_path);
    auto spec = signal_tl::parser::from_file(input_path);
    std::cout << "Parsed formulas are:" << std::endl;
    for (const auto& [expr_id, expr] : spec->formulas) {
      fmt::print(std::cout, "\t{} := {}\n", expr_id, expr);
    }
  } catch (const pegtl::parse_error& e) {
    const auto p = e.positions().front();
    std::cerr << e.what() << '\n'
              << in.line_at(p) << '\n'
              << std::setw(static_cast<int>(p.column)) << '^' << std::endl;
    return 1;
  }

  return 0;
}
