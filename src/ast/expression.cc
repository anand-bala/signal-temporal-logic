#include "argus/ast/expression.hpp"
#include "utils/visit.hpp" // for visit

#include <cstddef>     // for size_t
#include <functional>  // for hash
#include <map>         // for map, map<>::mapped_type
#include <memory>      // for hash, shared_ptr, weak_ptr, make_shared
#include <mutex>       // for mutex, lock_guard
#include <optional>    // for optional
#include <string_view> // for hash, string_view
#include <type_traits> // for decay_t

#include <magic_enum.hpp> // for enum_name

/// Combine multiple hashes.
///
/// Copied straight from Boost.container_hash
template <typename SizeT>
static constexpr void hash_combine(SizeT& seed, SizeT value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); // NOLINT
}

namespace std {
using namespace argus;
using namespace argus::ast::details;

template <>
struct hash<Attribute> {
  size_t operator()(const Attribute&) const;
};

template <>
struct hash<Constant> {
  size_t operator()(const Constant&) const;
};

template <>
struct hash<Variable> {
  size_t operator()(const Variable&) const;
};

template <>
struct hash<Parameter> {
  size_t operator()(const Parameter&) const;
};

template <>
struct hash<Function> {
  size_t operator()(const Function&) const;
};

template <>
struct hash<PredicateOp> {
  size_t operator()(const PredicateOp&) const;
};

template <>
struct hash<LogicalOp> {
  size_t operator()(const LogicalOp&) const;
};

template <>
struct hash<Interval> {
  size_t operator()(const Interval&) const;
};

template <>
struct hash<TemporalOp> {
  size_t operator()(const TemporalOp&) const;
};

template <>
struct hash<Expr> {
  size_t operator()(const Expr&) const;
};

template <>
struct hash<Expr*> {
  size_t operator()(const Expr*) const;
};

/// A constant is hashed by it's value
size_t hash<Constant>::operator()(const Constant& expr) const {
  return std::hash<PrimitiveTypes>{}(dynamic_cast<const PrimitiveTypes&>(expr));
}

/// A variable is hashed by it's name and type.
size_t hash<Variable>::operator()(const Variable& expr) const {
  size_t result          = std::hash<std::string>{}(expr.name);
  const string_view type = magic_enum::enum_name(expr.type);
  hash_combine(result, std::hash<std::string_view>{}(type));
  return result;
}

/// A parameter is hashed by it's name and type.
size_t hash<Parameter>::operator()(const Parameter& expr) const {
  size_t result          = std::hash<std::string>{}(expr.name);
  const string_view type = magic_enum::enum_name(expr.type);
  hash_combine(result, std::hash<std::string_view>{}(type));
  return result;
}

/// A function is hashed by its type/name, the hashes of its arguments, and that of
/// its attributes.
size_t hash<Function>::operator()(const Function& expr) const {
  size_t result = expr.custom_fn.has_value()
                      ? std::hash<std::string>{}(*expr.custom_fn)
                      : std::hash<std::string_view>{}(magic_enum::enum_name(expr.fn));

  const auto arg_hasher  = std::hash<ExprPtr>{};
  const auto attr_hasher = std::hash<Attribute>{};
  for (const auto& arg : expr.args) { hash_combine(result, arg_hasher(arg)); }
  for (const auto& attr : expr.attrs) { hash_combine(result, attr_hasher(attr)); }
  return result;
}

size_t hash<PredicateOp>::operator()(const PredicateOp& expr) const {
  size_t result = std::hash<std::string_view>{}(magic_enum::enum_name(expr.op));

  const auto arg_hasher = std::hash<ExprPtr>{};
  hash_combine(result, arg_hasher(expr.lhs));
  hash_combine(result, arg_hasher(expr.rhs));
  return result;
}

/// A logical operation is hashed by its type/name, and the hashes of its arguments.
size_t hash<LogicalOp>::operator()(const LogicalOp& expr) const {
  size_t result = std::hash<std::string_view>{}(magic_enum::enum_name(expr.op));

  const auto arg_hasher = std::hash<ExprPtr>{};
  for (const auto& arg : expr.args) { hash_combine(result, arg_hasher(arg)); }
  return result;
}

/// A Temporal operation is hashed by its type/name, and the hashes of its arguments.
size_t hash<TemporalOp>::operator()(const TemporalOp& expr) const {
  size_t result = std::hash<std::string_view>{}(magic_enum::enum_name(expr.op));

  const auto arg_hasher = std::hash<ExprPtr>{};
  for (const auto& arg : expr.args) { hash_combine(result, arg_hasher(arg)); }
  if (expr.interval != nullptr) {
    hash_combine(result, std::hash<Interval>{}(*expr.interval));
  }
  return result;
}

/// The hash of an expression is the hash of the type of expression it holds.
///
/// Since we inherit from a variant, we need to upcast the expression to the variant
/// type and then call the associated hash function. This is easier than writing your
/// own variant hash (commented below) as we shouldn't deal with magic numbers.
size_t hash<Expr>::operator()(const Expr& expr) const {
  std::size_t result =
      std::hash<ast::ExprTypes>{}(dynamic_cast<const ast::ExprTypes&>(expr));
  // expr.valueless_by_exception()
  //     ? 299792458 // Random value chosen by the universe upon creation
  //     : utils::visit(
  //           [](const auto& alt) {
  //             using alt_type   = std::decay_t<decltype(alt)>;
  //             using value_type = std::remove_const_t<alt_type>;
  //             return hash<value_type>{}(alt);
  //           },
  //           expr);
  return result;
}

size_t hash<Expr*>::operator()(const Expr* expr) const {
  return std::hash<Expr>{}(*expr);
}

size_t hash<Interval>::operator()(const Interval& interval) const {
  const auto arg_hasher = std::hash<ExprPtr>{};
  size_t result         = arg_hasher(interval.low);
  hash_combine(result, arg_hasher(interval.high));
  return result;
}

size_t hash<Attribute>::operator()(const Attribute& attr) const {
  size_t result = std::hash<std::string>{}(attr.key);
  for (const auto& value : attr.values) {
    hash_combine(
        result,
        utils::visit(
            [](const auto& val) -> size_t {
              return std::hash<std::decay_t<decltype(val)>>{}(val);
            },
            value));
  }
  return result;
}
} // namespace std

namespace argus {
ExprPtr Expr::make_expr(Expr&& expr) {
  // Here, we are defining a factory function that maintains a cache of all Expr
  // created (using the factory interface in Expr -- there are no guarantees if
  // someone eschews this).
  static auto cache  = std::map<size_t, std::weak_ptr<Expr>>{}; // Static cache.
  static auto vault  = std::mutex{};                            // Guard for the cache.
  static auto hasher = std::hash<Expr>{};                       // Static Expr hasher.
  // To compute the key for an expression into the cache, we need to:
  //
  // 1. Hash the given expr.
  // 2. Check if it is there in the cache.
  // 3. Return a shared_ptr to either a newly constructed expr or an existing one.
  std::lock_guard<std::mutex> close(vault); // RAII-style locking
  size_t id = hasher(expr);
  auto ret  = cache[id].lock();
  if (!ret) { // If cache[id] is expired or was empty
    ret       = std::make_shared<Expr>(std::move(expr)); // Create a new shared_ptr
    ret->m_id = id;                                      // Let it's ID be the hash
    cache[id] = ret; // Add a weak_ptr to the new Expr to the cache.
  }
  return ret;
}

std::string Expr::to_string() const {
  return utils::visit([](const auto& expr) { return expr.to_string(); }, *this);
} // namespace argus

} // namespace argus
