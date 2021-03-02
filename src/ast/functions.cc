#include "argus/ast/expression.hpp"          // for ExprPtr, Attribute, Function
#include "utils/static_analysis_helpers.hpp" // for assert_
#include "utils/visit.hpp"                   // for overloaded, visit

#include <fmt/format.h>   // for format, join
#include <magic_enum.hpp> // for enum_name, enum_cast

#include <memory>      // for allocator, operator==
#include <optional>    // for optional
#include <set>         // for set
#include <stdexcept>   // for invalid_argument
#include <string>      // for string, basic_string
#include <string_view> // for string_view
#include <utility>     // for move
#include <vector>      // for vector

namespace argus {

namespace ast::details {

Function::Function(
    Type op,
    std::optional<std::string> op_str,
    std::vector<ExprPtr> operands,
    std::set<Attribute, Attribute::KeyCompare> attributes) :
    fn{op},
    custom_fn{std::move(op_str)},
    args{std::move(operands)},
    attrs{std::move(attributes)} {
  // Check if args have been initialized.
  utils::assert_(!args.empty(), "A function must have at least 1 argument");
  // After the fields have been initialized, we need to check a few things.
  // 1. If the Type is not custom, the custom_fn field must be empty:
  if (custom_fn.has_value() && fn != Type::Custom) {
    throw std::invalid_argument(fmt::format(
        "Function having known type `{}` was also supplied with a custom function name string \"{}\"",
        magic_enum::enum_name(fn),
        *custom_fn));
  }
  // 2. If type is custom, but the string is empty
  if (fn == Type::Custom) {
    if (!custom_fn.has_value() || *custom_fn == "") {
      /// This should never happen when using the parser.
      throw std::invalid_argument(
          "Function with custom operation has an empty operation name");
    }
  }
  // 3. Each of the operands are either Constant, Parameter, Variable, or Function.
  // This enforces that functions are used only within predicates.
  for (const auto& expr : this->args) {
    if (expr == nullptr) {
      auto fn_name = magic_enum::enum_name(fn);
      throw std::invalid_argument(
          fmt::format("Function `{}` has a nullptr argument", fn_name));
    }
    auto is_valid = utils::visit(
        utils::overloaded{
            [](const Constant&) { return true; },
            [](const Parameter&) { return true; },
            [](const Variable&) { return true; },
            [](const Function&) { return true; },
            [](const auto&) {
              return false;
            }},
        *expr);
    if (!is_valid) {
      throw std::invalid_argument(
          "Functions can only operate on Constants, Parameters, Variables, or other Functions.");
    }
  }
}

std::string Function::to_string() const {
  std::string_view op;
  switch (fn) {
    case Type::Custom:
      op = custom_fn.value();
      break;
    case Type::Add:
      op = "+";
      break;
    case Type::Sub:
      op = "-";
      break;
    case Type::Mul:
      op = "*";
      break;
    case Type::Div:
      op = "/";
      break;
  }

  auto args_str = std::vector<std::string>(args.size());
  for (const auto& sub_expr : args) { args_str.push_back(sub_expr->to_string()); }
  auto attr_str = std::vector<std::string>(attrs.size());
  for (const auto& attr : attrs) { attr_str.push_back(attr.to_string()); }

  return fmt::format(
      "({} {} {})", op, fmt::join(args_str, " "), fmt::join(attr_str, " "));
}

} // namespace ast::details

ExprPtr Expr::Function(
    ast::FnType op,
    std::vector<ExprPtr> args,
    std::set<ast::Attribute, ast::Attribute::KeyCompare> attrs) {
  return make_expr(
      argus::ast::details::Function{op, std::move(args), std::move(attrs)});
}

ExprPtr Expr::Function(
    std::string op,
    std::vector<ExprPtr> args,
    std::set<ast::Attribute, ast::Attribute::KeyCompare> attrs) {
  auto fntype = magic_enum::enum_cast<ast::FnType>(op);
  if (fntype.has_value()) {
    if (*fntype != ast::FnType::Custom) {
      return Function(*fntype, std::move(args), std::move(attrs));
    }
  }
  return make_expr(
      argus::ast::details::Function{std::move(op), std::move(args), std::move(attrs)});
}

ExprPtr Expr::Add(std::vector<ExprPtr> args) {
  return Function(ast::FnType::Add, std::move(args), {});
}

ExprPtr Expr::Mul(std::vector<ExprPtr> args) {
  return Function(ast::FnType::Mul, std::move(args), {});
}

ExprPtr Expr::Subtract(ExprPtr lhs, ExprPtr rhs) {
  return Function(ast::FnType::Sub, {std::move(lhs), std::move(rhs)}, {});
}

ExprPtr Expr::Div(ExprPtr num, ExprPtr den) {
  return Function(ast::FnType::Div, {std::move(num), std::move(den)}, {});
}

} // namespace argus
