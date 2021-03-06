cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check if signaltl is being used directly or via add_subdirectory
set(SIGNALTL_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SIGNALTL_MASTER_PROJECT ON)
endif()

include(cmake/Version.cmake)
message(STATUS "signaltl: v${SIGNALTL_FULL_VERSION}")

project(
  signaltl
  VERSION "${SIGNALTL_VERSION}"
  DESCRIPTION
    "A library for efficiently working with Signal Temporal Logic (STL) and its quantitative semantics"
  LANGUAGES CXX
)

include(CMakeDependentOption)

# Options
option(BUILD_CORE_ONLY "Build only the core AST and Signal" OFF)
cmake_dependent_option(
  BUILD_PARSER "Don't build the parser" ON "NOT BUILD_CORE_ONLY" OFF
)
cmake_dependent_option(
  BUILD_ROBUSTNESS "Don't build the robust semantics for STL" ON
  "NOT BUILD_CORE_ONLY" OFF
)

option(BUILD_DOCS "Build the documentation?" OFF)
option(BUILD_EXAMPLES "Build the examples?" ${SIGNALTL_MASTER_PROJECT})

# TODO: Turn this on once the library is stable.
option(BUILD_PYTHON_BINDINGS "Build the Python extension?"
  OFF
)

option(ENABLE_CACHE "Enable cache if available" ${SIGNALTL_MASTER_PROJECT})
option(ENABLE_TESTING "Build signaltl test suite?" ${SIGNALTL_MASTER_PROJECT})
# Coverage needs to be explicitly turned on, but available only if Testing is
# enabled.
cmake_dependent_option(
  ENABLE_COVERAGE "Generate coverage.xml for test suite?" OFF "ENABLE_TESTING"
  OFF
)
option(ENABLE_STATIC_ANALYSIS "Enable clang-tidy and include-what-you-use" OFF)

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

if(NOT BUILD_CORE_ONLY)
  if(NOT NO_BUILD_PARSER)
    set(BUILD_PARSER
        ON
        CACHE BOOL "Build the parser"
    )
  endif()
  if(NOT NO_BUILD_ROBUSTNESS)
    set(BUILD_ROBUSTNESS
        ON
        CACHE BOOL "Build the robustness"
    )
  endif()
endif()

# ##############################################################################
# Include CMake Modules  #
# ##############################################################################

include(StandardProjectSettings)
include(PreventInSourceBuilds)
include(CompilerWarnings)
include(Cache)
include(StaticAnalyzers)

include(CTest)
include(Codecov)

# ##############################################################################
# Some Global Configuration  #
# ##############################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# ##############################################################################
# Third Party dependencies  #
# ##############################################################################

include(ThirdPartyDeps)

# ##############################################################################
# Sources  #
# ##############################################################################

add_subdirectory(src)
add_library(signaltl::signaltl ALIAS signaltl)
add_subdirectory(python_bindings)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(ENABLE_TESTING)
  add_subdirectory(tests)
  coverage_evaluate()
endif()

if(BUILD_DOCS)
  add_subdirectory(docs)
endif()

# ##############################################################################
# Installation  #
# ##############################################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(SIGNALTL_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for signaltlConfig.cmake"
)
set(INSTALL_CONFIGDIR ${SIGNALTL_CMAKECONFIG_INSTALL_DIR})

install(
  TARGETS signaltl
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
