#include "signal_tl/ast.hh"
#include "signal_tl/utils.hh"

#include <iterator>

namespace signal_tl {

namespace ast {

Predicate operator>(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::GT, bound};
};

Predicate operator>=(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::GE, bound};
};

Predicate operator<(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::LT, bound};
};

Predicate operator<=(const Predicate& lhs, const double bound) {
  return Predicate{lhs.name, ComparisonOp::LE, bound};
};

using utils::overloaded;

namespace {

Expr AndHelper(const AndPtr& lhs, const Expr& rhs) {
  std::vector<Expr> args{lhs->args};

  std::visit(
      overloaded{[&args](const auto e) { args.push_back(e); },
                 [&args](const Const e) {
                   if (!e.value) {
                     args = {Expr{e}};
                   }
                 },
                 [&args](const AndPtr e) {
                   args.reserve(
                       args.size() + std::distance(e->args.begin(), e->args.end()));
                   args.insert(args.end(), e->args.begin(), e->args.end());
                 }},
      rhs);
  return std::make_shared<And>(args);
}

Expr OrHelper(const OrPtr& lhs, const Expr& rhs) {
  std::vector<Expr> args{lhs->args};

  std::visit(
      overloaded{[&args](const auto e) { args.push_back(e); },
                 [&args](const Const e) {
                   if (e.value) {
                     args = {Expr{e}};
                   }
                 },
                 [&args](const OrPtr e) {
                   args.reserve(
                       args.size() + std::distance(e->args.begin(), e->args.end()));
                   args.insert(args.end(), e->args.begin(), e->args.end());
                 }},
      rhs);
  return std::make_shared<Or>(args);
}

} // namespace

Expr operator&(const Expr& lhs, const Expr& rhs) {
  if (const auto e_ptr = std::get_if<Const>(&lhs)) {
    return (e_ptr->value) ? rhs : *e_ptr;
  } else if (const AndPtr* e_ptr = std::get_if<AndPtr>(&lhs)) {
    return AndHelper(*e_ptr, rhs);
  }
  return std::make_shared<And>(std::vector{lhs, rhs});
}

Expr operator|(const Expr& lhs, const Expr& rhs) {
  if (const auto e_ptr = std::get_if<Const>(&lhs)) {
    return (e_ptr->value) ? rhs : *e_ptr;
  } else if (const AndPtr* e_ptr = std::get_if<AndPtr>(&lhs)) {
    return AndHelper(*e_ptr, rhs);
  }
  return std::make_shared<And>(std::vector{lhs, rhs});
}

Expr operator~(const Expr& expr) {
  if (const auto e = std::get_if<Const>(&expr)) {
    return Const{!(*e).value};
  }
  return std::make_shared<Not>(expr);
}

} // namespace ast

using ast::Expr;

ast::Const Const(bool value) {
  return ast::Const{value};
}

ast::Predicate Predicate(const std::string& name) {
  return ast::Predicate{name};
}

Expr Not(const Expr& arg) {
  return std::make_shared<ast::Not>(arg);
}

Expr And(const std::vector<Expr>& args) {
  return std::make_shared<ast::And>(args);
}

Expr Or(const std::vector<Expr>& args) {
  return std::make_shared<ast::Or>(args);
}

Expr Implies(const Expr& arg1, const Expr& arg2) {
  return ~(arg1) | arg2;
}

Expr Always(const Expr& arg, const std::optional<ast::Interval> interval) {
  return std::make_shared<ast::Always>(arg, interval);
}

Expr Eventually(const Expr& arg, const std::optional<ast::Interval> interval) {
  return std::make_shared<ast::Eventually>(arg, interval);
}

ast::Expr Until(
    const ast::Expr& arg1,
    const ast::Expr& arg2,
    const std::optional<ast::Interval> interval) {
  return std::make_shared<ast::Until>(arg1, arg2, interval);
}

} // namespace signal_tl
