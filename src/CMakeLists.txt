set(SIGNALTL_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
    CACHE INTERNAL ""
)

add_library(core core/ast.cc core/classic_robustness.cc core/minmax.cc core/signal.cc)
target_include_directories(
  core PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
              $<BUILD_INTERFACE:${SIGNALTL_INCLUDE_DIRS}>
)
target_compile_features(core PUBLIC cxx_std_17)
target_link_libraries(core PUBLIC fmt::fmt)
set_target_properties(
  core PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME signaltlcore
)
set_default_compile_options(core)
set_project_warnings(core)
enable_clang_tidy(core)
enable_include_what_you_use(core)
add_coverage(core)

add_library(parser parser/parser.cc parser/grammar.cc)
target_include_directories(
  parser PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${SIGNALTL_INCLUDE_DIRS}>
)
target_compile_features(parser PUBLIC cxx_std_17)
target_link_libraries(
  parser
  PRIVATE taocpp::pegtl
  PUBLIC core
)

include(CMakePrintHelpers)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(parser PUBLIC c++fs)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_link_libraries(parser PUBLIC stdc++fs)
endif()
set_target_properties(
  parser PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME signaltlparser
)
set_default_compile_options(parser)
set_project_warnings(parser)
enable_clang_tidy(parser)
enable_include_what_you_use(parser)
add_coverage(parser)
