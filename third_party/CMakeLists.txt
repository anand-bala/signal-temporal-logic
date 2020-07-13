add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/mono_wedge)

# fmtlib
find_package(fmt QUIET)
if(fmt_FOUND)
  message(STATUS "System installation of fmt found")
else()
  message(STATUS "Using bundled third_party/fmt.")
  git_submodule(PATH fmt WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
  # NOTE: Workaround when building SIGNALTL
  set(FMT_INSTALL
      ${SIGNALTL_INSTALL}
      CACHE BOOL "")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt)
  set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Python Bindings
if(SIGNALTL_PYTHON_BINDINGS)
  if(MSVC)
    set(PYBIND11_CPP_STANDARD /std:c++17)
  else()
    set(PYBIND11_CPP_STANDARD -std=c++17)
  endif()
  find_package(pybind11 QUIET)
  if(pybind11_FOUND)
    message(STATUS "System installation of pybind11 found")
  else()
    message(STATUS "Using bundled third_party/pybind11.")
    git_submodule(PATH pybind11 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
  endif()
endif()

# Catch2
if(_SIGNALTL_BUILD_THE_TESTS)
  find_package(Catch2 QUIET)
  if(Catch2_FOUND)
    message(STATUS "System installation of Catch2 found")
  else()
    message(STATUS "Using bundled third_party/Catch2.")
    git_submodule(PATH Catch2 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Catch2)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Catch2/contrib")
  endif()
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    PARENT_SCOPE)
