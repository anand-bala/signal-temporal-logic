cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check if signaltl is being used directly or via add_subdirectory
set(SIGNALTL_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SIGNALTL_MASTER_PROJECT ON)
endif()

# Set version from a common file.
file(READ "${CMAKE_CURRENT_LIST_DIR}/signal_tl/__init__.py" init_py_data)
string(REGEX MATCH "__version__ = \"([0-9]+)\\.([0-9]+)\\.([0-9]+)-([a-z]+)?\""
             _ ${init_py_data}
)

set(signaltl_MAJOR "${CMAKE_MATCH_1}")
set(signaltl_MINOR "${CMAKE_MATCH_2}")
set(signaltl_PATCHLEVEL "${CMAKE_MATCH_3}")
set(signaltl_VERSION_TYPE "${CMAKE_MATCH_4}")
set(signaltl_VERSION
    "${signaltl_MAJOR}.${signaltl_MINOR}.${signaltl_PATCHLEVEL}${signaltl_PRERELASE}"
)
message(STATUS "signaltl: v${signaltl_VERSION}")

project(
  signaltl
  VERSION "${signaltl_MAJOR}.${signaltl_MINOR}.${signaltl_PATCHLEVEL}"
  DESCRIPTION
    "A library for efficiently working with Signal Temporal Logic (STL) and its quantitative semantics"
  LANGUAGES CXX
)

# Options
option(BUILD_EXAMPLES "Build the examples?" ${SIGNALTL_MASTER_PROJECT})
option(BUILD_PYTHON_BINDINGS "Build the Python extension?"
       ${SIGNALTL_MASTER_PROJECT}
)

option(ENABLE_TESTING "Build signaltl test suite?" ${SIGNALTL_MASTER_PROJECT})
option(ENABLE_COVERAGE "Generate coverage.xml for test suite?" OFF)
option(ENABLE_STATIC_ANALYSIS "Enable clang-tidy and include-what-you-use" OFF)

option(INSTALL_PACKAGE "Configure package to be installed." ON)

include(StandardProjectSettings)
include(PreventInSourceBuilds)
include(CompilerWarnings)
include(StaticAnalyzers)

include(CTest)

set(_SIGNALTL_BUILD_THE_TESTS
    OFF
    CACHE INTERNAL "Easy option to build the tests"
)
if((SIGNALTL_MASTER_PROJECT AND ENABLE_TESTING) AND BUILD_TESTING)
  set(_SIGNALTL_BUILD_THE_TESTS
      ON
      CACHE INTERNAL "Easy option to build the tests"
  )
endif()

include(Codecov)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Third party libraries and dependencies
include(Conan)
find_package(fmt REQUIRED)
find_package(pegtl REQUIRED)
if(ENABLE_TESTING)
  find_package(Catch2 REQUIRED)
  # include(${Catch2_SOURCE_DIR}/contrib/Catch.cmake)
  include(Catch)
endif()
if(BUILD_PYTHON_BINDINGS)
  find_package(pybind11 REQUIRED)
endif()

add_subdirectory(src)
add_library(signaltl::core ALIAS core)
add_library(signaltl::parser ALIAS parser)

add_library(signaltl INTERFACE)
target_link_libraries(signaltl INTERFACE signaltl::core signaltl::parser)
add_library(signaltl::signaltl ALIAS signaltl)
add_subdirectory(python_bindings)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(_SIGNALTL_BUILD_THE_TESTS)
  add_subdirectory(tests)
  coverage_evaluate()
endif()

# Installing
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(SIGNALTL_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for signaltlConfig.cmake"
)
set(INSTALL_CONFIGDIR ${SIGNALTL_CMAKECONFIG_INSTALL_DIR})

install(
  TARGETS core parser signaltl
  EXPORT signaltl-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${SIGNALTL_INCLUDE_DIRS} TYPE INCLUDE)

install(
  EXPORT signaltl-targets
  FILE signaltlTargets.cmake
  NAMESPACE signaltl::
  DESTINATION ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/signaltlConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/signaltlConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/signaltlConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/signaltlConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/signaltlConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
)

export(
  EXPORT signaltl-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/signaltlTargets.cmake
  NAMESPACE signaltl::
)

# Register package in user's package registry
export(PACKAGE signaltl)
