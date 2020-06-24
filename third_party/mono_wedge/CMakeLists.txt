cmake_minimum_required(VERSION 3.5.0)
project(mono_wedge)

set(_MONO_WEDGE_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/include"
    CACHE INTERNAL "")

add_library(mono_wedge INTERFACE)
add_library(mono_wedge::mono_wedge ALIAS mono_wedge)
target_include_directories(
  mono_wedge INTERFACE $<BUILD_INTERFACE:${_MONO_WEDGE_INCLUDE_DIR}>
                       $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(mono_wedge INTERFACE cxx_std_11)
