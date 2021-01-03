cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check if signaltl is being used directly or via add_subdirectory
set(SIGNALTL_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SIGNALTL_MASTER_PROJECT ON)
endif()

# Set version from a common file.
file(READ "${CMAKE_CURRENT_LIST_DIR}/signal_tl/__init__.py" init_py_data)
string(
  REGEX MATCH
        "__version__ = \"([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[a-z]+)?\""
        _
        ${init_py_data})

set(SIGNALTL_MAJOR "${CMAKE_MATCH_1}")
set(SIGNALTL_MINOR "${CMAKE_MATCH_2}")
set(SIGNALTL_PATCHLEVEL "${CMAKE_MATCH_3}")
set(SIGNALTL_PRERELASE "${CMAKE_MATCH_4}")
set(SIGNALTL_VERSION "${SIGNALTL_MAJOR}.${SIGNALTL_MINOR}.${SIGNALTL_PATCHLEVEL}${SIGNALTL_PRERELASE}")
message(STATUS "signal_tl: v${SIGNALTL_VERSION}")

project(
  signal_tl
  VERSION "${SIGNALTL_MAJOR}.${SIGNALTL_MINOR}.${SIGNALTL_PATCHLEVEL}"
  DESCRIPTION "A library for efficiently working with Signal Temporal Logic (STL) and its quantitative semantics"
  LANGUAGES CXX)

# Options
option(BUILD_EXAMPLES "Build the examples?" ${SIGNALTL_MASTER_PROJECT})
option(BUILD_PYTHON_BINDINGS "Build the Python extension?" ${SIGNALTL_MASTER_PROJECT})

option(ENABLE_TESTING "Build signaltl test suite?" ${SIGNALTL_MASTER_PROJECT})
option(ENABLE_COVERAGE "Generate coverage.xml for test suite?" OFF)
option(ENABLE_ALL_STATIC_ANALYZERS "Enable clang-tidy and include-what-you-use" OFF)

option(INSTALL_PACKAGE "Configure package to be installed." ON)

include(StandardProjectSettings)
include(PreventInSourceBuilds)
include(CompilerWarnings)
include(Conan)
include(StaticAnalyzers)

include(CTest)

set(_SIGNALTL_BUILD_THE_TESTS
    OFF
    CACHE INTERNAL "Easy option to build the tests")
if((SIGNALTL_MASTER_PROJECT AND ENABLE_TESTING) AND BUILD_TESTING)
  set(_SIGNALTL_BUILD_THE_TESTS
      ON
      CACHE INTERNAL "Easy option to build the tests")
endif()

include(Codecov)
set(LCOV_REMOVE_PATTERNS
    "/usr/*"
    "$ENV{HOME}/.cache/*"
    "${CMAKE_CURRENT_LISTS_DIR}/tests/*"
    "${CMAKE_CURRENT_LISTS_DIR}/examples/*")

set(CMAKE_CXX_STANDARD
    17
    CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Third party libraries and dependencies
find_package(fmt REQUIRED)

add_subdirectory(core)
add_library(signal_tl::core ALIAS signaltl)
add_subdirectory(parser)
add_subdirectory(python_bindings)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(_SIGNALTL_BUILD_THE_TESTS)
  add_subdirectory(tests)
  coverage_evaluate()
endif()

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_LIBRARY_OUTPUT_DIRECTORY)

# Installing
if(INSTALL_PACKAGE)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)
  set(SIGNALTL_CMAKECONFIG_INSTALL_DIR
      "${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}"
      CACHE STRING "install path for signal_tlConfig.cmake")
  set(INSTALL_CONFIGDIR ${SIGNALTL_CMAKECONFIG_INSTALL_DIR})

  install(
    TARGETS signaltl
    EXPORT signal_tl-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

  install(DIRECTORY core/include parser/include TYPE INCLUDE)

  install(
    EXPORT signal_tl-targets
    FILE signal_tlTargets.cmake
    NAMESPACE signal_tl::
    DESTINATION ${INSTALL_CONFIGDIR})

  # Create a ConfigVersion.cmake file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

  configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/signal_tlConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

  # Install the config, configversion and custom find modules
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfigVersion.cmake DESTINATION ${INSTALL_CONFIGDIR})

  export(
    EXPORT signal_tl-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/signal_tlTargets.cmake
    NAMESPACE signal_tl::)

  # Register package in user's package registry
  export(PACKAGE signal_tl)
endif()
