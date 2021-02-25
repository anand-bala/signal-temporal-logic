#include "argus/ast/expression.hpp"

#include <atomic>

/// Counter for `Expr`
static std::size_t get_next_id() {
  static std::atomic_size_t id = 0;
  return id++;
}

namespace argus {

namespace nodes = ast::details;

template <typename ExprType>
std::unique_ptr<Expr> Expr::make_expr(ExprType arg) {
  auto expr  = std::make_unique<Expr>(std::move(arg));
  expr->m_id = get_next_id();
  return expr;
}

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

std::unique_ptr<Expr> Expr::Function(
    ast::FnType op,
    std::vector<ExprPtr> args,
    std::set<nodes::Attribute, nodes::Attribute::KeyCompare> attrs) {
  return make_expr(nodes::Function{op, std::move(args), std::move(attrs)});
}

std::unique_ptr<Expr> Expr::Function(
    std::string op,
    std::vector<ExprPtr> args,
    std::set<Attribute, Attribute::KeyCompare> attrs) {
  return make_expr(nodes::Function{std::move(op), std::move(args), std::move(attrs)});
}

std::unique_ptr<Expr> Expr::Add(std::vector<ExprPtr> args) {
  return make_expr(nodes::Function{ast::FnType::Add, std::move(args)});
}

std::unique_ptr<Expr> Expr::Mul(std::vector<ExprPtr> args) {
  return make_expr(nodes::Function{ast::FnType::Mul, std::move(args)});
}

std::unique_ptr<Expr> Expr::Subtract(ExprPtr lhs, ExprPtr rhs) {
  return make_expr(nodes::Function{ast::FnType::Sub, {std::move(lhs), std::move(rhs)}});
}

std::unique_ptr<Expr> Expr::Div(ExprPtr numerator, ExprPtr denominator) {
  return make_expr(nodes::Function{
      ast::FnType::Sub, {std::move(numerator), std::move(denominator)}});
}

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

std::unique_ptr<Expr> Expr::Next(ExprPtr arg) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{ast::ModalOpType::Next, std::move(arg_array)});
}

std::unique_ptr<Expr> Expr::Previous(ExprPtr arg) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{ast::ModalOpType::Previous, std::move(arg_array)});
}

std::unique_ptr<Expr> Expr::Eventually(ExprPtr arg, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Eventually, std::move(arg_array), std::move(interval)});
}

std::unique_ptr<Expr> Expr::Once(ExprPtr arg, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Once, std::move(arg_array), std::move(interval)});
}

std::unique_ptr<Expr> Expr::Always(ExprPtr arg, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Always, std::move(arg_array), std::move(interval)});
}

std::unique_ptr<Expr> Expr::Historically(ExprPtr arg, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg), nullptr};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Historically, std::move(arg_array), std::move(interval)});
}

std::unique_ptr<Expr> Expr::Until(ExprPtr arg1, ExprPtr arg2, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg1), std::move(arg2)};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Until, std::move(arg_array), std::move(interval)});
}

std::unique_ptr<Expr> Expr::Since(ExprPtr arg1, ExprPtr arg2, ExprPtr interval) {
  std::array<ExprPtr, 2> arg_array = {std::move(arg1), std::move(arg2)};
  return make_expr(nodes::TemporalOp{
      ast::ModalOpType::Since, std::move(arg_array), std::move(interval)});
}

} // namespace argus
