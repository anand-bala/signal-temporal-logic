/// @file     argus/ast/attributes.hpp
/// @brief    AST nodes for storing attributes/options passed to Commands and Functions.

#pragma once
#ifndef ARGUS_AST_ATTRIBUTES
#define ARGUS_AST_ATTRIBUTES

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "argus/ast/ast_fwd.hpp"

namespace ARGUS_AST_NS {

struct Attribute {
  std::string key;
  std::vector<PrimitiveTypes> values;

  struct KeyCompare {
    bool operator()(const Attribute& lhs, const Attribute& rhs) const {
      return lhs.key < rhs.key;
    }

    bool operator()(const Attribute& lhs, const std::string& rhs) const {
      return lhs.key < rhs;
    }

    bool operator()(const std::string& lhs, const Attribute& rhs) const {
      return lhs < rhs.key;
    }
  };

  [[nodiscard]] std::string to_string() const;
};

} // namespace ARGUS_AST_NS

#endif /* end of include guard: ARGUS_AST_ATTRIBUTES */
