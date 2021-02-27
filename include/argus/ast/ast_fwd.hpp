/// @file     argus/ast/ast_fwd.hpp
/// @brief    Forward declarations of AST types

#pragma once
#ifndef ARGUS_AST_FORWARD_DECL
#define ARGUS_AST_FORWARD_DECL

#include <memory>
#include <string>
#include <variant>

#define ARGUS_AST_NS argus::ast::details

namespace argus {
struct Expr;
using ExprPtr = std::shared_ptr<Expr>;
} // namespace argus

namespace ARGUS_AST_NS {

using PrimitiveTypes =
    std::variant<std::string, double, long long int, unsigned long long int, bool>;
struct Constant;
struct Variable;
struct Parameter;

struct Function;

struct PredicateOp;
struct LogicalOp;
struct TemporalOp;

struct Interval;
struct Attribute;
} // namespace ARGUS_AST_NS

#endif /* end of include guard: ARGUS_AST_FORWARD_DECL */
