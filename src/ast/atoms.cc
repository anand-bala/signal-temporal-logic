#include "argus/ast/expression.hpp" // for Expr, ExprPtr, Parameter, Variable, Constant
#include "utils/visit.hpp"          // for overloaded, visit

#include <string>  // for string, basic_string
#include <utility> // for move

#include <fmt/format.h> // for format

namespace argus {

namespace ast::details {

std::string Constant::to_string() const {
  return utils::visit(
      utils::overloaded{
          [&](const std::string& s) { return fmt::format("\"{}\"", s); },
          [&](auto c) { return fmt::format("{}", c); },
      },
      *this);
}

std::string Variable::to_string() const {
  return name;
}

std::string Parameter::to_string() const {
  return name;
}

} // namespace ast::details

namespace nodes = ast::details;

ExprPtr Expr::Constant(nodes::PrimitiveTypes constant) {
  return make_expr(nodes::Constant{std::move(constant)});
}

ExprPtr Expr::Variable(std::string name, ast::VarType type) {
  return make_expr(nodes::Variable{std::move(name), type});
}

ExprPtr Expr::Parameter(std::string name, ast::ParamType type) {
  return make_expr(nodes::Parameter{std::move(name), type});
}

} // namespace argus
