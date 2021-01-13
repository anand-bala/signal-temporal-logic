#pragma once

#ifndef SIGNALTL_INTERNAL_FILESYSTEM_HPP
#define SIGNALTL_INTERNAL_FILESYSTEM_HPP

// First, we will check if the source is being compiled with
#if __cplusplus >= 201703L

// The below code is adapted from https://stackoverflow.com/a/53365539 with the
// following edits:
//
// - Alias std::filesystem and std::experimental filesystem to stdfs to prevent
//   undefined behavior of changing namespaces withing std.
//

// We haven't checked which filesystem to include yet
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// Check for feature test macro for <filesystem>
#if defined(__cpp_lib_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0

// Check for feature test macro for <experimental/filesystem>
#elif defined(__cpp_lib_experimental_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// We can't check if headers exist...
// Let's assume experimental to be safe
#elif !defined(__has_include)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Check if the header "<filesystem>" exists
#elif __has_include(<filesystem>)

// Check if we are on MSVC
#ifdef _MSC_VER

// Check if MSVC version is greater than or equal to 15.7 (1914)
#if _MSC_VER >= 1914
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#else
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#endif

// Not on Visual Studio. Let's use the normal version
#else // #ifdef _MSC_VER
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#endif

// Check if the header "<filesystem>" exists
#elif __has_include(<experimental/filesystem>)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Fail if neither header is available with a nice error message
#else
#error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#endif

// We priously determined that we need the exprimental version
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
// Include it
#include <experimental/filesystem> // IWYU pragma: export

// We need the alias from std::experimental::filesystem to std::filesystem
namespace stdfs = std::experimental::filesystem;

// We have a decent compiler and can use the normal version
#else
// Include it
#include <filesystem> // IWYU pragma: export

namespace stdfs = std::filesystem;
#endif

#endif // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

#endif // __cplusplus

#endif
