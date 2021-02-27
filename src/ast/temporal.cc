#include "argus/ast/expression.hpp"

#include "utils/static_analysis_helpers.hpp"
#include "utils/visit.hpp"

#include <fmt/format.h>
#include <magic_enum.hpp>
#include <range/v3/view.hpp>

#include <limits>
#include <stdexcept>

namespace argus {

namespace ast::details {

Interval::Interval(ExprPtr _low, ExprPtr _high) {
  if (_low == nullptr) {
    this->low = Expr::Constant(static_cast<double>(0));
  } else { // If we weren't given a nullptr low
    bool is_const = std::holds_alternative<Constant>(*_low);
    bool is_param = std::holds_alternative<Parameter>(*_low);
    if (!is_const && !is_param) {
      throw std::invalid_argument(
          "Lower bound for interval was neither a Constant nor a Parameter");
    }
    if (is_const) {
      const auto& val = std::get<Constant>(*_low);
      if (!val.is_nonnegative()) {
        throw std::invalid_argument("Lower bound for interval is negative");
      }
    }
    this->low = std::move(_low);
  }

  if (_high == nullptr) {
    this->high = Expr::Constant(std::numeric_limits<double>::infinity());
  } else { // If we weren't given a nullptr high
    bool is_const = std::holds_alternative<Constant>(*_high);
    bool is_param = std::holds_alternative<Parameter>(*_high);
    if (!is_const && !is_param) {
      throw std::invalid_argument(
          "Upper bound for interval was neither a Constant nor a Parameter");
    }
    if (is_const) {
      const auto& val = std::get<Constant>(*_high);
      if (!val.is_nonnegative()) {
        throw std::invalid_argument("Upper bound for interval is negative");
      }
    }
    this->high = std::move(_high);
  }
}

template <typename L, typename H>
Interval::Interval(L _low, H _high) {
  static_assert(
      std::is_arithmetic<L>() && std::is_arithmetic<H>(),
      "Interval types must be arithmetic types (integral/floating)");

  this->low  = Expr::Constant(_low);
  this->high = Expr::Constant(_high);
}
std::string Interval::to_string() const {
  std::string low_str  = "0";
  std::string high_str = "inf";
  if (low) {
    low_str = low->to_string();
  }
  if (high) {
    high_str = high->to_string();
  }
  return fmt::format("(_ {} {})", low_str, high_str);
}

TemporalOp::TemporalOp(
    Type operation,
    std::vector<ExprPtr> arguments,
    std::shared_ptr<Interval> interval_arg) :
    op{operation}, args{std::move(arguments)}, interval{std::move(interval_arg)} {
  // Here, we need to check:
  //
  // 1. If the type is Unary, then the number of arguments is 1. Otherwise 2.
  // 2. If the type is Prev or Next, it must not contain an Interval.
  // 3. Each argument must be a Predicate, a LogicalOp, or a TemporalOp

  // 1. Check nargs
  size_t required_args = 2;
  switch (op) {
    case Type::Since:
    case Type::Until:
      required_args = 2;
      break;
    default:
      required_args = 1;
      break;
  }
  if (required_args != args.size()) {
    throw std::invalid_argument(fmt::format(
        "Operation `{}` requires exactly {} arguments, got {}.",
        magic_enum::enum_name(op),
        required_args,
        args.size()));
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

std::string TemporalOp::to_string() const {
  std::string_view op_str;
  switch (op) {
    case Type::Next:
      op_str = "next";
      break;
    case Type::Previous:
      op_str = "previous";
      break;
    case Type::Eventually:
      op_str = "eventually";
      break;
    case Type::Once:
      op_str = "once";
      break;
    case Type::Always:
      op_str = "always";
      break;
    case Type::Historically:
      op_str = "historically";
      break;
    case Type::Until:
      op_str = "until";
      break;
    case Type::Since:
      op_str = "since";
      break;
  }
  auto args_str = std::vector<std::string>(args.size());
  for (const auto& sub_expr : args) { args_str.push_back(sub_expr->to_string()); }

  return fmt::format("({} {} {})", op, interval->to_string(), fmt::join(args_str, " "));
}
} // namespace ast::details

std::unique_ptr<Expr> Expr::Next(ExprPtr arg) {
  return make_expr(ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Next, {std::move(arg)}});
}

std::unique_ptr<Expr> Expr::Previous(ExprPtr arg) {
  return make_expr(
      ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Previous, {std::move(arg)}});
}

std::unique_ptr<Expr> Expr::Eventually(ExprPtr arg) {
  return make_expr(
      ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Eventually, {std::move(arg)}});
}

std::unique_ptr<Expr>
Expr::Eventually(ExprPtr arg, std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Eventually, {std::move(arg)}, std::move(interval)});
}

std::unique_ptr<Expr> Expr::Once(ExprPtr arg) {
  return make_expr(ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Once, {std::move(arg)}});
}

std::unique_ptr<Expr>
Expr::Once(ExprPtr arg, std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Once, {std::move(arg)}, std::move(interval)});
}

std::unique_ptr<Expr> Expr::Always(ExprPtr arg) {
  return make_expr(
      ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Always, {std::move(arg)}});
}

std::unique_ptr<Expr>
Expr::Always(ExprPtr arg, std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Always, {std::move(arg)}, std::move(interval)});
}

std::unique_ptr<Expr> Expr::Historically(ExprPtr arg) {
  return make_expr(
      ARGUS_AST_NS::TemporalOp{ast::ModalOpType::Historically, {std::move(arg)}});
}

std::unique_ptr<Expr>
Expr::Historically(ExprPtr arg, std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Historically, {std::move(arg)}, std::move(interval)});
}

std::unique_ptr<Expr> Expr::Until(ExprPtr arg1, ExprPtr arg2) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Until, {std::move(arg1), std::move(arg2)}});
}

std::unique_ptr<Expr> Expr::Until(
    ExprPtr arg1,
    ExprPtr arg2,
    std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Until,
      {std::move(arg1), std::move(arg2)},
      std::move(interval)});
}

std::unique_ptr<Expr> Expr::Since(ExprPtr arg1, ExprPtr arg2) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Since, {std::move(arg1), std::move(arg2)}});
}

std::unique_ptr<Expr> Expr::Since(
    ExprPtr arg1,
    ExprPtr arg2,
    std::shared_ptr<ARGUS_AST_NS::Interval> interval) {
  return make_expr(ARGUS_AST_NS::TemporalOp{
      ast::ModalOpType::Since,
      {std::move(arg1), std::move(arg2)},
      std::move(interval)});
}
} // namespace argus
