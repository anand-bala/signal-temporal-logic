#pragma once

#ifndef SIGNAL_TEMPORAL_LOGIC_EXCEPTION_HPP
#define SIGNAL_TEMPORAL_LOGIC_EXCEPTION_HPP

#include <exception>
#include <string>
#include <utility>

// LCOV_EXCL_START
namespace signal_tl {

struct not_implemented_error : public std::exception {
 private:
  const std::string reason;

 public:
  not_implemented_error(std::string what_arg) : reason{std::move(what_arg)} {}
  not_implemented_error(const char* what_arg) : reason{what_arg} {}

  not_implemented_error(const not_implemented_error& other) = default;

  [[nodiscard]] const char* what() const noexcept override {
    return this->reason.c_str();
  }
};

} // namespace signal_tl
// LCOV_EXCL_STOP

#endif /* end of include guard: __SIGNAL_TEMPORAL_LOGIC_EXCEPTION_HH__ */
