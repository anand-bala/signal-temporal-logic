/// @file     argus/ast/attributes.hpp
/// @brief    AST nodes for storing attributes/options passed to Commands and Functions.

#pragma once
#ifndef ARGUS_AST_ATTRIBUTES
#define ARGUS_AST_ATTRIBUTES

#include <memory>
#include <string>
#include <variant>

namespace argus::ast::details {

struct Attribute;

struct AttributeValue : std::variant<
                            bool,
                            long long int,
                            unsigned long long int,
                            std::string,
                            std::unique_ptr<Attribute>> {};

struct Attribute {
  std::string key;
  AttributeValue value;

  struct KeyCompare {
    bool operator()(const Attribute& lhs, const Attribute& rhs) {
      return lhs.key < rhs.key;
    }
  };
};

} // namespace argus::ast::details

#endif /* end of include guard: ARGUS_AST_ATTRIBUTES */
