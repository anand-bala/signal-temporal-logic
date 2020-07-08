cmake_minimum_required(VERSION 3.5.0)
project(
  signal_tl
  VERSION 0.1.0
  DESCRIPTION
    "A library for efficiently working with Signal Temporal Logic (STL) and its quantitative semantics"
  LANGUAGES CXX)

# Check if signaltl is being used directly or via add_subdirectory
set(SIGNALTL_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SIGNALTL_MASTER_PROJECT ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakePrintHelpers)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

include(ThirdPartyLibs)

# Options
option(SIGNALTL_TEST "Build signaltl test suite?" ${SIGNALTL_MASTER_PROJECT})
option(SIGNALTL_EXAMPLES "Build the examples?" ${SIGNALTL_MASTER_PROJECT})
option(SIGNALTL_INSTALL "Install the library and header files?"
       ${SIGNALTL_MASTER_PROJECT})
option(SIGNALTL_PYTHON_BINDINGS "Build the Python extension?" ON)

set(_SIGNALTL_BUILD_THE_TESTS
    OFF
    CACHE INTERNAL "Easy option to build the tests")
if((SIGNALTL_MASTER_PROJECT OR SIGNALTL_TEST) AND BUILD_TESTING)
  set(_SIGNALTL_BUILD_THE_TESTS
      ON
      CACHE INTERNAL "Easy option to build the tests")
endif()

if(MSVC)
    set(PYBIND11_CPP_STANDARD /std:c++17)
else()
    set(PYBIND11_CPP_STANDARD -std=c++17)
endif()
set(CMAKE_CXX_EXTENSIONS ON)

# Third party libraries and dependencies
add_subdirectory(third_party)

# Add libraries

set(STL_SOURCES src/ast.cc src/classic_robustness.cc src/signal.cc)

if(NOT MSVC)
add_library(signaltl SHARED ${STL_SOURCES})
else()
# NOTE: This is some weird stuff for MSVC
add_library(signaltl STATIC ${STL_SOURCES})
endif()
add_library(signal_tl::signal_tl ALIAS signaltl)
target_include_directories(
  signaltl PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
target_compile_features(signaltl PUBLIC cxx_std_17)
target_link_libraries(signaltl PRIVATE mono_wedge::mono_wedge)



if(SIGNALTL_PYTHON_BINDINGS)

  set(BINDINGS_SOURCES signal_tl/pyast.cc signal_tl/pyrobustness.cc
                       signal_tl/pysignal.cc signal_tl/pysignal_tl.cc)

  pybind11_add_module(_cext MODULE ${BINDINGS_SOURCES})
  target_include_directories(_cext PRIVATE ${CMAKE_CURRENT_LIST_DIR}/signal_tl)
  target_link_libraries(_cext PUBLIC signaltl)
endif()

if(SIGNALTL_EXAMPLES)
  add_subdirectory(examples)
endif()

if(_SIGNALTL_BUILD_THE_TESTS)
  find_package(Catch2 QUIET)
  if(Catch2_FOUND)
    message(STATUS "System installation of Catch2 found")
  else()
    git_submodule(PATH third_party/Catch2 WORKING_DIRECTORY
                  ${CMAKE_CURRENT_LIST_DIR})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/Catch2)
    list(APPEND CMAKE_MODULE_PATH
         "${CMAKE_CURRENT_LIST_DIR}/third_party/Catch2/contrib")
  endif()

  add_subdirectory(tests)
endif()

# Installing
if(SIGNALTL_INSTALL)
  set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/signal_tl)

  install(
    TARGETS signaltl mono_wedge
    EXPORT signal_tl-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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
    ${CMAKE_CURRENT_LIST_DIR}/cmake/signal_tlConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

  # Install the config, configversion and custom find modules
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/signal_tlConfigVersion.cmake
          DESTINATION ${INSTALL_CONFIGDIR})

  export(
    EXPORT signal_tl-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/signal_tlTargets.cmake
    NAMESPACE signal_tl::)

  # Register package in user's package registry
  export(PACKAGE signal_tl)

endif()
