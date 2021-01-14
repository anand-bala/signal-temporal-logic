# The trailing slash is important for the installing part
set(SIGNALTL_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include/
    CACHE INTERNAL ""
)

# set default flags for signaltl libraries
function(signaltl_flags target)
  target_include_directories(
    ${target} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                     $<BUILD_INTERFACE:${SIGNALTL_INCLUDE_DIRS}>
  )
  target_compile_features(${target} PUBLIC cxx_std_17)
  set_default_compile_options(${target})
  set_project_warnings(${target})
  enable_clang_tidy(${target})
  enable_include_what_you_use(${target})
  add_coverage(${target})
  set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

add_library(
  core core/ast.cc core/classic_robustness.cc core/minmax.cc core/signal.cc
)
target_link_libraries(core PUBLIC fmt::fmt)
set_target_properties(core PROPERTIES OUTPUT_NAME signaltlcore)
signaltl_flags(core)

add_library(parser parser/parser.cc parser/grammar.hpp parser/parser_errors.hpp)
target_link_libraries(
  parser
  PRIVATE taocpp::pegtl
  PUBLIC core
)
target_include_directories(parser PRIVATE parser/)
set_target_properties(parser PROPERTIES OUTPUT_NAME signaltlparser)
signaltl_flags(parser)
set_std_filesystem_options(parser)
