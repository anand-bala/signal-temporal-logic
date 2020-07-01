cmake_minimum_required(VERSION 3.5.0)
project(signal_temporal_logic)

# Options
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TESTS "Build tests" ON)

# Add libraries
set(PYBIND11_CPP_STANDARD -std=c++17)
set(CMAKE_CXX_EXTENSIONS ON)
add_subdirectory(third_party)

set(STL_SOURCES src/ast.cc src/minmax.cc src/signal.cc
                src/classic_robustness.cc src/efficient_robustness.cc)

add_library(signaltl SHARED ${STL_SOURCES})
target_include_directories(
  signaltl PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(signaltl PUBLIC cxx_std_17)
target_compile_options(signaltl PUBLIC -std=c++17)
target_link_libraries(signaltl PRIVATE mono_wedge::mono_wedge)

set(BINDINGS_SOURCES signal_tl/pyast.cc signal_tl/pyrobustness.cc
                     signal_tl/pysignal.cc signal_tl/pysignal_tl.cc)

pybind11_add_module(_cext MODULE ${BINDINGS_SOURCES})
target_include_directories(_cext PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/signal_tl)
target_link_libraries(_cext PRIVATE signaltl)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
