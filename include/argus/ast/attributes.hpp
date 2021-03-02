/// @file     argus/ast/attributes.hpp
/// @brief    AST nodes for storing attributes/options passed to Commands and Functions.

#pragma once
#ifndef ARGUS_AST_ATTRIBUTES
#define ARGUS_AST_ATTRIBUTES

#include "argus/ast/ast_fwd.hpp" // for PrimitiveTypes

#include <string> // for operator<, string
#include <vector> // for vector

namespace argus::ast::details {

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

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_ATTRIBUTES */
