#include "argus/ast/expression.hpp"
#include "utils/visit.hpp"

#include <fmt/format.h>

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

template <typename CType>
std::unique_ptr<Expr> Expr::Constant(CType constant) {
  return make_expr(nodes::Constant{constant});
}

std::unique_ptr<Expr> Expr::Variable(std::string name, ast::VarType type) {
  return make_expr(nodes::Variable{std::move(name), type});
}

std::unique_ptr<Expr> Expr::Parameter(std::string name, ast::ParamType type) {
  return make_expr(nodes::Parameter{std::move(name), type});
}

} // namespace argus
