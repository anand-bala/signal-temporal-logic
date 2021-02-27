cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

include(FetchContent)
include(CMakePrintHelpers)

if(ARGUS_MASTER_PROJECT OR NOT FETCHCONTENT_BASE_DIR)
  get_filename_component(
    fc_base "./.cache/deps" REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}"
  )
  set(FETCHCONTENT_BASE_DIR ${fc_base})
endif()

# ##############################################################################
# Core Dependencies
# ##############################################################################

unset(CMAKE_CXX_CLANG_TIDY)
unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)

message(CHECK_START "Looking for fmtlib/fmt")
find_package(fmt QUIET)
if(NOT fmt_FOUND)
  message(CHECK_FAIL "system library not found (using fetched version).")
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 7.1.3
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(fmt)
  if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
  endif()
else()
  message(CHECK_PASS "system library found.")
endif()

message(CHECK_START "Looking for Neargye/magic_enum")
find_package(magic_enum QUIET)
if(NOT magic_enum_FOUND)
  message(CHECK_FAIL "system library not found (using fetched version).")
  FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.7.2
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(magic_enum)
  if(NOT magic_enum_POPULATED)
    FetchContent_Populate(magic_enum)
    add_subdirectory(${magic_enum_SOURCE_DIR} ${magic_enum_BINARY_DIR})
  endif()
else()
  message(CHECK_PASS "system library found.")
endif()

message(CHECK_START "Looking for taocpp/pegtl")
find_package(pegtl QUIET)
if(NOT pegtl_FOUND)
  message(CHECK_FAIL "system library not found (using fetched version).")
  FetchContent_Declare(
    pegtl
    GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
    GIT_TAG f34a1170afba0207a3c29ec08959591e3e7068c5
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(pegtl)
  if(NOT pegtl_POPULATED)
    FetchContent_Populate(pegtl)
    # EXCLUDE_FROM_ALL is needed to prevent installation of the private
    # dependency
    add_subdirectory(${pegtl_SOURCE_DIR} ${pegtl_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
else()
  message(CHECK_PASS "system library found.")
endif()

message(CHECK_START "Looking for ericniebler/range-v3")
find_package(range-v3 QUIET)
if(NOT range-v3_FOUND)
  message(CHECK_FAIL "system library not found (using fetched version).")
  FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG 0.11.0
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(range-v3)
  if(NOT range-v3_POPULATED)
    FetchContent_Populate(range-v3)
    add_subdirectory(${range-v3_SOURCE_DIR} ${range-v3_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
else()
  message(CHECK_PASS "system library found.")
endif()


# ##############################################################################
# Python Bindings
# ##############################################################################

# if(BUILD_PYTHON_BINDINGS) message(CHECK_START "Looking for pybind11/pybind11")
# find_package(pybind11 QUIET) if(NOT pybind11_FOUND) message(CHECK_FAIL "system
# library not found (using fetched version).") FetchContent_Declare( pybind11
# GIT_REPOSITORY https://github.com/pybind/pybind11.git GIT_TAG v2.6.1
# GIT_PROGRESS ON )
#
# FetchContent_GetProperties(pybind11) if(NOT pybind11_POPULATED)
# FetchContent_Populate(pybind11) set(PYBIND11_INSTALL ON CACHE BOOL "Install
# pybind11 header files" FORCE ) add_subdirectory(${pybind11_SOURCE_DIR}
# ${pybind11_BINARY_DIR}) endif() else() message(CHECK_PASS "system library
# found.") endif() endif()

# ##############################################################################
# Testing Dependencies
# ##############################################################################

if(ENABLE_TESTING)
  message(CHECK_START "Looking for catchorg/Catch2")
  find_package(Catch2 QUIET)
  if(NOT Catch2_FOUND)
    message(CHECK_FAIL "system library not found (using fetched version).")
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

# ##############################################################################
# Documentation dependencies
# ##############################################################################

function(get_mcss_repo mcss_dir)
  message(STATUS "Fetching mosra/m.css for documentation")
  FetchContent_Declare(
    mcss
    GIT_REPOSITORY https://github.com/mosra/m.css.git
    GIT_TAG 42d4a9a48f31f5df6e246c948403b54b50574a2a
    GIT_PROGRESS ON
  )

  FetchContent_GetProperties(mcss)
  if(NOT mcss_POPULATED)
    FetchContent_Populate(mcss)
  endif()
  set(${mcss_dir}
      ${mcss_SOURCE_DIR}
      PARENT_SCOPE
  )
endfunction()
