#include "argus/ast/attributes.hpp"
#include "utils/visit.hpp" // for overloaded, visit

#include <fmt/format.h> // for format, to_string, join

namespace argus::ast::details {

std::string Attribute::to_string() const {
  auto val_str = std::vector<std::string>{values.size()};
  for (const auto& v : values) {
    val_str.push_back(utils::visit(
        utils::overloaded{
            [](const std::string& str) { return fmt::format("\"{}\"", str); },
            [](const auto& c) { return fmt::to_string(c); },
        },
        v));
  }
  if (val_str.size() == 1) {
    return fmt::format("{} {}", key, val_str.back());
  }
  return fmt::format("{} ({})", key, fmt::join(val_str, " "));
}

} // namespace argus::ast::details
