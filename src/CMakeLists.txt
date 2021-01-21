# The trailing slash is important for the installing part
set(SIGNALTL_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include/
    CACHE PATH "Path to the signaltl include directory"
)

set(SIGNALTL_SRCS core/signal.cc core/ast.cc)

if(BUILD_PARSER)
  list(APPEND SIGNALTL_SRCS parser/error_messages.hpp parser/actions.hpp
       parser/parser.cc parser/grammar.hpp
  )
else()
  message(STATUS "Not building parser")
endif()

if(BUILD_ROBUSTNESS)
  list(APPEND SIGNALTL_SRCS robust_semantics/classic_robustness.cc
       robust_semantics/minmax.cc robust_semantics/minmax.hpp
  )
else()
  message(STATUS "Not building robust semantics")
endif()

add_library(signaltl ${SIGNALTL_SRCS})

target_link_libraries(signaltl PUBLIC fmt::fmt)
target_include_directories(
  signaltl PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                  $<BUILD_INTERFACE:${SIGNALTL_INCLUDE_DIRS}>
)
target_compile_features(signaltl PUBLIC cxx_std_17)
set_default_compile_options(signaltl)
set_project_warnings(signaltl)
enable_clang_tidy(signaltl)
enable_include_what_you_use(signaltl)
add_coverage(signaltl)
set_target_properties(signaltl PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(BUILD_PARSER)
  target_link_libraries(signaltl PRIVATE taocpp::pegtl)
  target_include_directories(signaltl PRIVATE parser/)
  set_std_filesystem_options(signaltl)
endif()
if(BUILD_ROBUSTNESS)
  target_include_directories(signaltl PRIVATE robust_semantics)
endif()
