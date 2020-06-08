cmake_minimum_required(VERSION 3.5.0)
project(timed_rl)

# Add libraries
set(PYBIND11_CPP_STANDARD -std=c++17)
add_subdirectory(third_party)

list(APPEND STL_SOURCES src/ast.cc src/efficient_robustness.cc src/signal.cc)

add_library(libsignaltl SHARED ${STL_SOURCES})
target_include_directories(
  libsignaltl
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(libsignaltl PUBLIC cxx_std_17)

list(APPEND BINDINGS_SOURCES signal_tl/pyast.cc signal_tl/pyrobustness.cc
     signal_tl/pysignal.cc signal_tl/pysignal_tl.cc)

pybind11_add_module(_cext MODULE ${BINDINGS_SOURCES})
target_include_directories(_cext PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/signal_tl)
target_link_libraries(_cext PRIVATE libsignaltl)
