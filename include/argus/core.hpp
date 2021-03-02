/// @file     argus/ast.hpp
/// @brief    Interface file for the Argus specification language AST
///
/// We primarily use this file to export the contents of other AST files.

#pragma once
#ifndef ARGUS_CORE_HPP
#define ARGUS_CORE_HPP

// IWYU pragma: begin_exports
#include "argus/ast/atoms.hpp"
#include "argus/ast/functions.hpp"
#include "argus/ast/propositional.hpp"
#include "argus/ast/temporal.hpp"

#include "argus/ast/expression.hpp"
#include "argus/ast/manipulate.hpp"
// IWYU pragma: end_exports

#endif /* end of include guard: ARGUS_CORE_HPP */
