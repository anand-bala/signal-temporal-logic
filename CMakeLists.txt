cmake_minimum_required(VERSION 3.5.0)
project(timed_rl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PYBIND11_CPP_STANDARD -std=c++17)

# Add libraries
add_subdirectory(third_party)

list(
  APPEND
  STL_SOURCES
  csrc/ast.hh
  csrc/robustness.cc
  csrc/robustness.hh
  csrc/signal.cc
  csrc/signal.hh)

add_library(libsignaltl SHARED ${STL_SOURCES})
target_include_directories(libsignaltl PUBLIC csrc/)

pybind11_add_module(_cext MODULE csrc/bindings.cc)
target_link_libraries(_cext PRIVATE libsignaltl)

add_executable(test test/test.cpp)
target_link_libraries(test libsignaltl)
