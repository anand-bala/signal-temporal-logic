#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_FMT_HH__
#define __SIGNAL_TEMPORAL_LOGIC_FMT_HH__

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

// NOTE(anand): This seems to fix the segfault that happens when formatting Expr. I have
// no idea why...
inline std::ostream& operator<<(std::ostream& os, const signal_tl::ast::Expr& expr) {
  std::string s = std::visit([](const auto e) { return fmt::to_string(*e); }, expr);
  return os << s;
}

template <>
struct fmt::formatter<signal_tl::ast::Expr> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Expr& expr, FormatContext& ctx) {
    return std::visit(
        [&ctx](const auto& e) { return format_to(ctx.out(), "{}", *e); }, expr);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Const> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Const& e, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", e.value);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Predicate> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Predicate& e, FormatContext& ctx) {
    std::string op = ">=";
    switch (e.op) {
      case signal_tl::ast::ComparisonOp::GE:
        op = ">=";
        break;
      case signal_tl::ast::ComparisonOp::GT:
        op = ">";
        break;
      case signal_tl::ast::ComparisonOp::LE:
        op = "<=";
        break;
      case signal_tl::ast::ComparisonOp::LT:
        op = "<";
        break;
    }
    return format_to(ctx.out(), "{} {} {}", e.name, op, e.lhs);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Not> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Not& e, FormatContext& ctx) {
    return format_to(ctx.out(), "~{}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::And> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::And& e, FormatContext& ctx) {
    return format_to(ctx.out(), "({})", fmt::join(e.args, " & "));
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Or> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Or& e, FormatContext& ctx) {
    return format_to(ctx.out(), "({})", fmt::join(e.args, " & "));
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Always> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Always& e, FormatContext& ctx) {
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "G[{}, int) {}", a, e.arg);
      }
      return format_to(ctx.out(), "G[{},{}] {}", a, b, e.arg);
    }
    return format_to(ctx.out(), "G {}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Eventually> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Eventually& e, FormatContext& ctx) {
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "F[{}, inf] {}", a, e.arg);
      }
      return format_to(ctx.out(), "F[{},{}] {}", a, b, e.arg);
    }
    return format_to(ctx.out(), "F {}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Until> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Until& e, FormatContext& ctx) {
    const auto [e1, e2] = e.args;
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "{} U[{}, inf) {}", e1, a, e2);
      }
      return format_to(ctx.out(), "{} U[{},{}] {}", e1, a, b, e2);
    }
    return format_to(ctx.out(), "{} U {}", e1, e2);
  }
};

template <>
struct fmt::formatter<signal_tl::signal::Sample> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::signal::Sample& s, FormatContext& ctx) {
    return format_to(ctx.out(), "({}, {})", s.time, s.value);
  }
};

template <>
struct fmt::formatter<signal_tl::signal::Signal> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const signal_tl::signal::Signal& s, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", fmt::join(s, ", "));
  }
};

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_FMT_HH__ */
