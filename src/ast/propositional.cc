#include "argus/ast/expression.hpp"

#include "utils/static_analysis_helpers.hpp"
#include "utils/visit.hpp"

#include <stdexcept>

#include <fmt/format.h>
#include <magic_enum.hpp>
#include <range/v3/view.hpp>

namespace argus {

namespace ast::details {

PredicateOp::PredicateOp(Type cmp, ExprPtr arg1, ExprPtr arg2) :
    op{cmp}, lhs{std::move(arg1)}, rhs{std::move(arg2)} {
  // A predicate must have as argument either Constants, Variables, or Functions. Let us
  // check this.

  // 1. Check if the arguments exist.
  if (lhs == nullptr || rhs == nullptr) {
    throw std::invalid_argument("Predicate has non-existent (nullptr) arguments");
  }
  // 2. Since neither of the arguments are nullptr, we can check their types.
  { // First check the LHS
    bool is_valid = utils::visit(
        utils::overloaded{
            [](const Constant&) { return true; },
            [](const Parameter&) { return true; },
            [](const Variable&) { return true; },
            [](const Function&) { return true; },
            [](const auto&) {
              return false;
            }},
        *lhs);
    if (!is_valid) {
      throw std::invalid_argument(
          "LHS of Predicate is invalid: not a Constant, Parameter, Variable, or Function");
    }
  }

  { // Now check the RHS
    bool is_valid = utils::visit(
        utils::overloaded{
            [](const Constant&) { return true; },
            [](const Parameter&) { return true; },
            [](const Variable&) { return true; },
            [](const Function&) { return true; },
            [](const auto&) {
              return false;
            }},
        *rhs);
    if (!is_valid) {
      throw std::invalid_argument(
          "RHS of Predicate is invalid: not a Constant, Parameter, Variable, or Function");
    }
  }
}

std::string PredicateOp::to_string() const {
  std::string_view op_str;
  switch (op) {
    case Type::LE:
      op_str = "<=";
      break;
    case Type::LT:
      op_str = "<";
      break;
    case Type::GE:
      op_str = ">=";
      break;
    case Type::GT:
      op_str = ">";
      break;
    case Type::EQ:
      op_str = "eq";
      break;
    case Type::NE:
      op_str = "neq";
      break;
  }
  return fmt::format("({} {} {})", op, lhs->to_string(), rhs->to_string());
}

LogicalOp::LogicalOp(Type operation, std::vector<ExprPtr> operands) :
    op{operation}, args{std::move(operands)} {
  // Here, we need to check:
  //
  // 1. If the type is Not, then the number of arguments is 1. Otherwise >= 2.
  // 2. Each argument must be a Predicate, a LogicalOp, or a TemporalOp

  // 1. Check nargs
  if (op == Type::Not) {
    if (args.size() != 1) {
      throw std::invalid_argument(fmt::format(
          "Unary `not` operation expects exactly 1 argument, got {}", args.size()));
    }
  } else { // And or Or
    if (args.size() < 2) {
      throw std::invalid_argument(fmt::format(
          "N-ary `{}` operation expects at least 2 arguments, got {}",
          magic_enum::enum_name(op),
          args.size()));
    }
  }

  // 2. Check argument type
  for (const auto& [idx, expr] : ranges::views::enumerate(args)) {
    if (expr != nullptr) {
      throw std::invalid_argument(fmt::format("Argument at position {} is null", idx));
    }
    bool is_valid = utils::visit(
        utils::overloaded{
            [](const Constant&) { return false; },
            [](const Parameter&) { return false; },
            [](const Variable&) { return false; },
            [](const Function&) { return false; },
            [](const auto&) { // Can be anything but the above.
              return true;
            }},
        *expr);
    if (!is_valid) {
      throw std::invalid_argument(fmt::format(
          "Argument at position {} is not valid: must be a Predicate, a Logical Operation, or a Temporal Operation",
          idx));
    }
  }
}

std::string LogicalOp::to_string() const {
  std::string_view op_str;
  switch (op) {
    case Type::Not:
      op_str = "not";
      break;
    case Type::And:
      op_str = "and";
      break;
    case Type::Or:
      op_str = "or";
      break;
  }

  auto args_str = std::vector<std::string>(args.size());
  for (const auto& sub_expr : args) { args_str.push_back(sub_expr->to_string()); }
  return fmt::format("({} {})", op, fmt::join(args_str, " "));
}

} // namespace ast::details

namespace nodes = ast::details;

std::unique_ptr<Expr> Expr::Eq(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::EQ, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Neq(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::NE, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Lt(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::LT, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Le(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::LE, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Gt(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::GT, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Ge(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::PredicateOp{ast::CmpOp::GE, std::move(lhs), std::move(rhs)});
}

std::unique_ptr<Expr> Expr::Not(ExprPtr arg) {
  return make_expr(nodes::LogicalOp{ast::LogicOpType::Not, {std::move(arg)}});
}

std::unique_ptr<Expr> Expr::And(std::vector<ExprPtr> arg) {
  return make_expr(nodes::LogicalOp{ast::LogicOpType::And, std::move(arg)});
}

std::unique_ptr<Expr> Expr::Or(std::vector<ExprPtr> arg) {
  return make_expr(nodes::LogicalOp{ast::LogicOpType::Or, std::move(arg)});
}

std::unique_ptr<Expr> Expr::Implies(ExprPtr x, ExprPtr y) {
  ExprPtr not_x = Not(std::move(x));
  return Or({std::move(not_x), std::move(y)});
}

std::unique_ptr<Expr> Expr::Xor(ExprPtr x, ExprPtr y) {
  ExprPtr not_x    = Not(x);
  ExprPtr not_y    = Not(y);
  ExprPtr x_or_y   = Or({std::move(x), std::move(y)});
  ExprPtr nx_or_ny = Or({std::move(not_x), std::move(not_y)});
  return And({std::move(x_or_y), std::move(nx_or_ny)});
}

std::unique_ptr<Expr> Expr::Iff(ExprPtr x, ExprPtr y) {
  ExprPtr not_x     = Not(x);
  ExprPtr not_y     = Not(y);
  ExprPtr x_and_y   = And({std::move(x), std::move(y)});
  ExprPtr nx_and_ny = And({std::move(not_x), std::move(not_y)});
  return Or({std::move(x_and_y), std::move(nx_and_ny)});
}
} // namespace argus
