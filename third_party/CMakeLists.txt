cmake_minimum_required(VERSION 3.5.0)

# Some helper functions

find_package(Git QUIET)
if(GIT_FOUND)
  function(git_submodule path_to_dep)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init -- ${path_to_dep}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endfunction()
endif()

# External utility libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mono_wedge)

# Python Bindings
if(SIGNALTL_PYTHON_BINDINGS)
  find_package(pybind11 QUIET)
  if(pybind11_FOUND)
    message(STATUS "System pybind11 found")
  else()
    message(STATUS "Using bundled third_party/pybind11.")
    git_submodule(pybind11)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pybind11)
  endif()
endif()
