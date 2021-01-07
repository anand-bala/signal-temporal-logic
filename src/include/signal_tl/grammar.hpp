#pragma once

#ifndef SIGNALTL_GRAMMAR_HPP
#define SIGNALTL_GRAMMAR_HPP

#include <cstddef>

namespace signal_tl {

namespace grammar {
/// Forward declaration of the root of the grammar.
struct SpecificationFile;

namespace internal {

/// **INTERNAL USE ONLY**
///
/// This is used to call `tao::pagtl::contrib::analyze`, a function that
/// analyzes the parser grammar for construction errors like unresolved cycles,
/// etc. Used in the tests to check the grammar and is useful only for
/// developers of this library.
size_t analyze(int verbose = 1);
} // namespace internal

} // namespace grammar
} // namespace signal_tl

#endif /* end of include guard: SIGNALTL_GRAMMAR_HPP */
