cmake_minimum_required(VERSION 3.5.0)

# External utility libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mono_wedge)

# Python Bindings
if(SIGNALTL_PYTHON_BINDINGS)
  find_package(pybind11 QUIET)
  if(pybind11_FOUND)
    message(STATUS "System installation of pybind11 found")
  else()
    git_submodule(PATH pybind11 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
  endif()
endif()
