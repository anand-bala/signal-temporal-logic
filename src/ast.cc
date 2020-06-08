#include "signal_tl/ast.hh"
#include "utils.hh"

#include <iterator>

namespace ast {
using utils::overloaded;

namespace {

Expr AndHelper(const AndPtr& lhs, const Expr& rhs) {
  std::vector args{lhs->args};

  std::visit(
      overloaded{[&args](auto&& e) { args.push_back(e); },
                 [&args](const AndPtr& e) {
                   args.reserve(
                       args.size() + std::distance(e->args.begin(), e->args.end()));
                   args.insert(args.end(), e->args.begin(), e->args.end());
                 }},
      rhs);

  return And::as_expr(args);
}

Expr OrHelper(const OrPtr& lhs, const Expr& rhs) {
  std::vector args{lhs->args};

  std::visit(
      overloaded{[&args](auto&& e) { args.push_back(e); },
                 [&args](const OrPtr& e) {
                   args.reserve(
                       args.size() + std::distance(e->args.begin(), e->args.end()));
                   args.insert(args.end(), e->args.begin(), e->args.end());
                 }},
      rhs);

  return Or::as_expr(args);
}

} // namespace

std::ostream& operator<<(std::ostream& out, const Expr& expr) {
  std::visit([&out](auto&& e) { out << *e; }, expr);
  return out;
}

Expr operator&(const Expr& lhs, const Expr& rhs) {
  return std::visit(
      overloaded{[&lhs, &rhs](auto&& e) {
                   return And::as_expr({lhs, rhs});
                 },
                 [&rhs](const AndPtr& e) {
                   return AndHelper(e, rhs);
                 }},
      lhs);
}

Expr operator|(const Expr& lhs, const Expr& rhs) {
  return std::visit(
      overloaded{[&lhs, &rhs](auto&& e) {
                   return Or::as_expr({lhs, rhs});
                 },
                 [&rhs](const OrPtr& e) {
                   return OrHelper(e, rhs);
                 }},
      lhs);
}

Expr operator~(const Expr& expr) {
  return Not::as_expr(expr);
}

} // namespace ast
