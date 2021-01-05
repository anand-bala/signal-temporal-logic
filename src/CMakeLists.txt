
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

# Try compiling a minimal program with each header/namespace, in order of preference:
# C++17: #include <filesystem> // std::filesystem Experimental C++17: #include
# <experimental/filesystem> // std::experimental::filesystem Boost.Filesystem: #include
# <boost/filesystem.hpp> // boost::filesystem
check_filesystem_impl("filesystem" "std::filesystem" "stdc++fs;c++fs" STD_FILESYSTEM)
if(STD_FILESYSTEM)
  message(STATUS "Using std::filesystem")
else()
  check_filesystem_impl(
    "experimental/filesystem" "std::experimental::filesystem" "stdc++fs;c++fs"
    STD_EXPERIMENTAL_FILESYSTEM
  )
  if(STD_EXPERIMENTAL_FILESYSTEM)
    target_compile_definitions(
      ${PROJECT_NAME} INTERFACE TAO_PEGTL_STD_EXPERIMENTAL_FILESYSTEM
    )
    message(WARNING "Using std::experimental::filesystem as a fallback")
  else()
    message(
      FATAL_ERROR
        "Requires C++17, including an implementation of std::filesystem, which your compiler toolchain does not seem to support."
    )
  endif()
endif()

add_library(parser parser/parser.cc)
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
set_target_properties(
  parser PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME signaltlparser
)
set_default_compile_options(parser)
set_project_warnings(parser)
enable_clang_tidy(parser)
enable_include_what_you_use(parser)
add_coverage(parser)
