#include "signal_tl/ast.hpp"
#include "signal_tl/internal/utils.hpp"

#include <cstddef>
#include <iterator>

namespace signal_tl {

namespace ast {

Predicate operator>(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::GT, bound};
}

Predicate operator>=(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::GE, bound};
}

Predicate operator<(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::LT, bound};
}

Predicate operator<=(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::LE, bound};
}

using utils::overloaded;

namespace {

Expr AndHelper(const AndPtr& lhs, const Expr& rhs) {
  std::vector<Expr> args{lhs->args};

  std::visit(
      overloaded{
          [&args](const auto e) { args.push_back(e); },
          [&args](const Const e) {
            if (!e.value) {
              args = {Expr{e}};
            }
          },
          [&args](const AndPtr& e) {
            args.reserve(
                args.size() +
                static_cast<size_t>(std::distance(e->args.begin(), e->args.end())));
            args.insert(args.end(), e->args.begin(), e->args.end());
          }},
      rhs);
  return std::make_shared<And>(args);
}

Expr OrHelper(const OrPtr& lhs, const Expr& rhs) {
  std::vector<Expr> args{lhs->args};

  std::visit(
      overloaded{
          [&args](const auto e) { args.push_back(e); },
          [&args](const Const e) {
            if (e.value) {
              args = {Expr{e}};
            }
          },
          [&args](const OrPtr& e) {
            args.reserve(
                args.size() +
                static_cast<size_t>(std::distance(e->args.begin(), e->args.end())));
            args.insert(args.end(), e->args.begin(), e->args.end());
          }},
      rhs);
  return std::make_shared<Or>(args);
}

} // namespace

Expr operator&(const Expr& lhs, const Expr& rhs) {
  if (const auto c_ptr = std::get_if<Const>(&lhs)) {
    return (c_ptr->value) ? rhs : *c_ptr;
  } else if (const AndPtr* e_ptr = std::get_if<AndPtr>(&lhs)) {
    return AndHelper(*e_ptr, rhs);
  }
  return std::make_shared<And>(std::vector{lhs, rhs});
}

Expr operator|(const Expr& lhs, const Expr& rhs) {
  if (const auto c_ptr = std::get_if<Const>(&lhs)) {
    return (!c_ptr->value) ? rhs : *c_ptr;
  } else if (const OrPtr* e_ptr = std::get_if<OrPtr>(&lhs)) {
    return OrHelper(*e_ptr, rhs);
  }
  return std::make_shared<Or>(std::vector{lhs, rhs});
}

Expr operator~(const Expr& expr) {
  if (const auto e = std::get_if<Const>(&expr)) {
    return Const{!(*e).value};
  }
  return std::make_shared<Not>(expr);
}

} // namespace ast

ast::Const Const(bool value) {
  return ast::Const{value};
}

ast::Predicate Predicate(std::string name) {
  return ast::Predicate{std::move(name)};
}

Expr Not(Expr arg) {
  if (const auto e = std::get_if<ast::Const>(&arg)) {
    return ast::Const{!(*e).value};
  }
  return std::make_shared<ast::Not>(std::move(arg));
}

Expr And(std::vector<Expr> args) {
  return std::make_shared<ast::And>(std::move(args));
}

Expr Or(std::vector<Expr> args) {
  return std::make_shared<ast::Or>(std::move(args));
}

Expr Implies(const Expr& x, const Expr& y) {
  return (~x) | (y);
}

Expr Xor(const Expr& x, const Expr& y) {
  return (x | y) & (~x | ~y);
}

Expr Iff(const Expr& x, const Expr& y) {
  return (x & y) | (~x & ~y);
}

Expr Always(Expr arg) {
  return std::make_shared<ast::Always>(std::move(arg));
}

Expr Always(Expr arg, ast::Interval interval) {
  return std::make_shared<ast::Always>(std::move(arg), interval);
}

Expr Eventually(Expr arg) {
  return std::make_shared<ast::Eventually>(std::move(arg));
}

Expr Eventually(Expr arg, ast::Interval interval) {
  return std::make_shared<ast::Eventually>(std::move(arg), interval);
}

Expr Until(Expr arg1, Expr arg2) {
  return std::make_shared<ast::Until>(std::move(arg1), std::move(arg2));
}

Expr Until(Expr arg1, Expr arg2, ast::Interval interval) {
  return std::make_shared<ast::Until>(std::move(arg1), std::move(arg2), interval);
}

} // namespace signal_tl
