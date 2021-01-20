cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

include(FetchContent)
include(CMakePrintHelpers)

if(SIGNALTL_MASTER_PROJECT)
  get_filename_component(fc_base "./.deps"
      REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}")
  set(FETCHCONTENT_BASE_DIR ${fc_base})
endif()

# ##############################################################################
# Core Dependencies
# ##############################################################################

if(NOT BUILD_CORE_ONLY)
  if(NOT NO_BUILD_ROBUSTNESS)
    set(_BUILD_PARSER ON)
  endif()
  if(NOT NO_BUILD_ROBUSTNESS)
    set(_BUILD_ROBUSTNESS ON)
  endif()
endif()

message(CHECK_START "Looking for fmtlib/fmt")
find_package(fmt QUIET)
if(NOT fmt_FOUND)
  message(CHECK_FAIL "system library not found. Downloading.")
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 7.1.3
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(fmt)
  if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    set(FMT_INSTALL
        ON
        CACHE BOOL "Generate the install target for fmtlib." FORCE
    )
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
  endif()
else()
  message(CHECK_PASS "system library found.")
endif()

if(BUILD_PARSER)
  message(CHECK_START "Looking for taocpp/pegtl")
  find_package(pegtl QUIET)
  if(NOT pegtl_FOUND)
    message(CHECK_FAIL "system library not found. Downloading.")
    FetchContent_Declare(
      pegtl
      GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
      GIT_TAG 3.2.0
      GIT_PROGRESS ON
    )

    FetchContent_GetProperties(pegtl)
    if(NOT pegtl_POPULATED)
      FetchContent_Populate(pegtl)
      add_subdirectory(${pegtl_SOURCE_DIR} ${pegtl_BINARY_DIR})
      export(
        EXPORT pegtl-targets
        FILE ${pegtl_BINARY_DIR}/pegtl-targets.cmake
        NAMESPACE taocpp::
      )
    endif()
  else()
    message(CHECK_PASS "system library found.")
  endif()
endif()

# ##############################################################################
# Python Bindings
# ##############################################################################

if(BUILD_PYTHON_BINDINGS)
  message(CHECK_START "Looking for pybind11/pybind11")
  find_package(pybind11 QUIET)
  if(NOT pybind11_FOUND)
    message(CHECK_FAIL "system library not found. Downloading.")
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.6.1
      GIT_PROGRESS ON
    )

    FetchContent_GetProperties(pybind11)
    if(NOT pybind11_POPULATED)
      FetchContent_Populate(pybind11)
      set(pybind11_INSTALL
          ON
          CACHE BOOL "Generate the install target for pybind11lib." FORCE
      )
      add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
    endif()
  else()
    message(CHECK_PASS "system library found.")
  endif()
endif()

# ##############################################################################
# Testing Dependencies
# ##############################################################################

if(ENABLE_TESTING)
  message(CHECK_START "Looking for catchorg/Catch2")
  find_package(Catch2 QUIET)
  if(NOT Catch2_FOUND)
    message(CHECK_FAIL "system library not found. Downloading.")
    FetchContent_Declare(
      catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v2.13.4
      GIT_PROGRESS ON
    )

    FetchContent_GetProperties(catch2)
    if(NOT catch2_POPULATED)
      FetchContent_Populate(catch2)
      cmake_print_variables(catch2_SOURCE_DIR)
      add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
      include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
    endif()
  else()
    message(CHECK_PASS "system library found.")
  endif()
endif()
