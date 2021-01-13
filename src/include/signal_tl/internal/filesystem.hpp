#pragma once

#ifndef SIGNALTL_INTERNAL_FILESYSTEM_HPP
#define SIGNALTL_INTERNAL_FILESYSTEM_HPP

// First, we will check if the source is being compiled with
#if __cplusplus >= 201703L

#if __has_include(<filesystem>)
#include <filesystem>
namespace stdfs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#else
#error "Missing the <filesystem> and <experimental/filesystem> headers."
#endif

#else
#error "Need to compile with C++17 support"
#endif

#endif
