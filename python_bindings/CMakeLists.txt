if(NOT BUILD_PYTHON_BINDINGS)
  return()
endif()

# We will setup __init__.py with the full version string (PEP440 compatible).
configure_file(init.py.in ${PROJECT_SOURCE_DIR}/signal_tl/__init__.py @ONLY)

set(BINDINGS_SOURCES pyast.cc pyrobustness.cc pysignal.cc pysignal_tl.cc)

pybind11_add_module(_cext MODULE ${BINDINGS_SOURCES})
target_include_directories(_cext PRIVATE ${CMAKE_CURRENT_LISTS_DIR})
target_link_libraries(_cext PUBLIC signaltl::signaltl)
add_coverage_flags(_cext)
