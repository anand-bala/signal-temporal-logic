if(NOT BUILD_PYTHON_BINDINGS)
  return()
endif()

find_package(pybind11 REQUIRED)
set(BINDINGS_SOURCES
    pyast.cc
    pyrobustness.cc
    pysignal.cc
    pysignal_tl.cc)

pybind11_add_module(_cext MODULE ${BINDINGS_SOURCES})
target_include_directories(_cext PRIVATE ${CMAKE_CURRENT_LISTS_DIR})
target_link_libraries(_cext PUBLIC signaltl)
add_coverage_flags(_cext)
