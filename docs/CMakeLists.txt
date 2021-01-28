if(NOT BUILD_DOCS)
  return()
endif()

include(CMakePrintHelpers)

option(MCSS_DIR "Path to the root of the m.css repository" "")
if(NOT MCSS_DIR)
  # Fetch the repo
  get_mcss_repo(MCSS_DIR)
endif()

if(EXISTS "${MCSS_DIR}/documentation/doxygen.py")
  set(DOXY_PY "${MCSS_DIR}/documentation/doxygen.py")
else()
  message(SEND_ERROR "mcss directory doesn't exist: ${MCSS_DIR}")
  message(FATAL_ERROR "Unable to generate documentation")
endif()

find_package(Doxygen REQUIRED)

set(DOXYGEN_PROJECT_NAME "Signal Temporal Logic")
set(DOXYGEN_PROJECT_NUMBER "${SIGNALTL_FULL_VERSION}")
set(DOXYGEN_PROJECT_BRIEF "")
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
set(DOXYGEN_INPUT_DIRS_LIST ${PROJECT_SOURCE_DIR}/src/include ${CMAKE_CURRENT_LIST_DIR})

# Configure the Doxyfile.in
string(REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT_DIRS_LIST}")
configure_file(Doxyfile.in Doxyfile @ONLY)
# Copy the conf.py into the build dir
file(COPY conf.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(docs COMMAND ${DOXY_PY} ${CMAKE_CURRENT_BINARY_DIR}/conf.py --debug
  DEPENDS Doxyfile.in conf.py
  COMMENT "Generate documentation in ${DOXYGEN_OUTPUT_DIR}")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/html DESTINATION ${CMAKE_INSTALL_DOCDIR})
