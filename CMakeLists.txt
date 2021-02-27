cmake_minimum_required(VERSION 3.11)

# Check if argus is being used directly or via add_subdirectory
set(ARGUS_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(ARGUS_MASTER_PROJECT ON)
endif()

include(cmake/Version.cmake)
message(STATUS "Configuring argus: v${ARGUS_FULL_VERSION}")

project(
  argus
  VERSION "${ARGUS_VERSION}"
  DESCRIPTION "Temporal Logic Monitoring Tool"
  LANGUAGES CXX
)

include(CMakeDependentOption)

option(BUILD_DOCS "Build the documentation?" OFF)
option(BUILD_EXAMPLES "Build the examples?" ${ARGUS_MASTER_PROJECT})

option(ENABLE_CACHE "Enable cache if available" ${ARGUS_MASTER_PROJECT})
option(ENABLE_TESTING "Build argus test suite?" ${ARGUS_MASTER_PROJECT})
# Coverage needs to be explicitly turned on, but available only if Testing is
# enabled.
cmake_dependent_option(
  ENABLE_COVERAGE "Generate coverage information from test suite?" OFF
  "ENABLE_TESTING" OFF
)
option(ENABLE_STATIC_ANALYSIS "Enable clang-tidy and include-what-you-use" OFF)

# ##############################################################################
# Include CMake Modules  #
# ##############################################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(StandardProjectSettings)
include(PreventInSourceBuilds)
include(CompilerWarnings)
include(Cache)
include(StaticAnalyzers)

include(CTest)
include(Codecov)

include(GNUInstallDirs)

# ##############################################################################
# Some Global Configuration  #
# ##############################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# ##############################################################################
# Helper targets/functions  #
# ##############################################################################

set(argus_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algorithms.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/algebraic_semantics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/cumulative_semantics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/filtering_semantics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/monitor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/monitor_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/algos/robust_semantics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/argus.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/ast_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/atoms.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/attributes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/expression.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/functions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/manipulate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/propositional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/signal.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/ast/temporal.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/automata/automaton.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/core.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/internal/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/signals.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/signals/base_signal.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/signals/pwc.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argus/signals/pwl.hpp
)

# Interface library for the header files. Useful for generating
# compile_commands.json
add_library(argus-headers-only INTERFACE)
target_sources(
  argus-headers-only INTERFACE "$<BUILD_INTERFACE:${argus_headers}>"
)
target_include_directories(
  argus-headers-only
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  TARGETS argus-headers-only
  EXPORT argusTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Add an OBJECT library for a component of libargus. A thin wrapper around
# `add_library(OBJECT)`. Thus, it takes as the first argument a `target` name,
# and the rest is assumed to be the sources making up the Object Library.
function(add_argus_component target)
  add_library(${target} OBJECT ${ARGN})
  target_link_libraries(${target} PUBLIC argus-headers-only)
  target_include_directories(
    ${target} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  )
  target_compile_features(${target} PUBLIC cxx_std_17)
  set_default_compile_options(${target})
  set_project_warnings(${target})
  enable_clang_tidy(${target})
  enable_include_what_you_use(${target})
  enable_cppcheck(${target})
  add_coverage(${target})

  install(
    TARGETS ${target}
    EXPORT argusTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endfunction()

# ##############################################################################
# Third Party dependencies  #
# ##############################################################################

include(ThirdPartyDeps)

# ##############################################################################
# Sources  #
# ##############################################################################

add_subdirectory(src)

# add_subdirectory(examples)

set(_ARGUS_BUILD_THE_TESTS
    OFF
    CACHE INTERNAL "Easy option to build the tests"
)
if((ARGUS_MASTER_PROJECT AND ENABLE_TESTING) AND BUILD_TESTING)
  set(_ARGUS_BUILD_THE_TESTS
      ON
      CACHE INTERNAL "Easy option to build the tests"
  )
endif()
# add_subdirectory(tests)

# ##############################################################################
# Installation  #
# ##############################################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ARGUS_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for argusConfig.cmake"
)
set(INSTALL_CONFIGDIR ${ARGUS_CMAKECONFIG_INSTALL_DIR})

install(
  EXPORT argusTargets
  FILE argusTargets.cmake
  NAMESPACE argus::
  DESTINATION ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/argusConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/argusConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/argusConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the include dir
install(DIRECTORY include/argus TYPE INCLUDE)

# Install the config, configversion and custom find modules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/argusConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/argusConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
)

install(EXPORT argusTargets DESTINATION ${INSTALL_CONFIGDIR})

install(FILES LICENSE LICENSE-3rdParty DESTINATION ${CMAKE_INSTALL_DOCDIR})

# export( EXPORT argusTargets FILE
# ${CMAKE_CURRENT_BINARY_DIR}/argusTargets.cmake NAMESPACE argus:: )
#
# # Register package in user's package registry export(PACKAGE argus)
