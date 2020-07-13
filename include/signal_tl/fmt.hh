#pragma once

#ifndef __SIGNAL_TEMPORAL_LOGIC_FMT_HH__
#define __SIGNAL_TEMPORAL_LOGIC_FMT_HH__

#include <fmt/format.h>

#include <cmath>

#include "signal_tl/ast.hh"
#include "signal_tl/signal.hh"

template <>
struct fmt::formatter<signal_tl::ast::Expr> : fmt::dynamic_formatter<> {
  template <typename FormatContext>
  auto format(const signal_tl::ast::Expr& v, FormatContext& ctx) {
    return std::visit(
        [&](const auto& val) { return dynamic_formatter<>::format(*val, ctx); }, v);
  }
};

inline constexpr auto default_parse(fmt::format_parse_context& ctx) {
  auto it = ctx.begin(), end = ctx.end();
  if (it != end && *it != '}')
    throw fmt::format_error("invalid format");
  return it;
}

template <>
struct fmt::formatter<signal_tl::ast::Const> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Const& e, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", e.value);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Predicate> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
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
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Not& e, FormatContext& ctx) {
    return format_to(ctx.out(), "~{}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::And> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::And& e, FormatContext& ctx) {
    return format_to(ctx.out(), "({})", fmt::join(e.args, " & "));
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Or> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Or& e, FormatContext& ctx) {
    return format_to(ctx.out(), "({})", fmt::join(e.args, " & "));
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Always> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Always& e, FormatContext& ctx) {
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "G {}", e.arg);
      }
      return format_to(ctx.out(), "G[{},{}] {}", a, b, e.arg);
    }
    return format_to(ctx.out(), "G {}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Eventually> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Eventually& e, FormatContext& ctx) {
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "F {}", e.arg);
      }
      return format_to(ctx.out(), "F[{},{}] {}", a, b, e.arg);
    }
    return format_to(ctx.out(), "F {}", e.arg);
  }
};

template <>
struct fmt::formatter<signal_tl::ast::Until> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::ast::Until& e, FormatContext& ctx) {
    const auto [e1, e2] = e.args;
    if (e.interval.has_value()) {
      const auto [a, b] = e.interval.value();
      if (std::isinf(b)) {
        return format_to(ctx.out(), "{} U {}", e1, e2);
      }
      return format_to(ctx.out(), "{} U [{},{}] {}", e1, a, b, e2);
    }
    return format_to(ctx.out(), "{} U {}", e1, e2);
  }
};

template <>
struct fmt::formatter<signal_tl::signal::Sample> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::signal::Sample& s, FormatContext& ctx) {
    return format_to(ctx.out(), "({}, {})", s.time, s.value);
  }
};

template <>
struct fmt::formatter<signal_tl::signal::Signal> {
  constexpr auto parse(format_parse_context& ctx) {
    return default_parse(ctx);
  }

  template <typename FormatContext>
  auto format(const signal_tl::signal::Signal& s, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", fmt::join(s, ", "));
  }
};

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_FMT_HH__ */
