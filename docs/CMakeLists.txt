if(NOT BUILD_DOCS)
  return()
endif()

find_package(Doxygen REQUIRED)

set(DOXYGEN_PROJECT_NAME "Signal Temporal Logic")
set(DOXYGEN_PROJECT_NUMBER "${SIGNALTL_FULL_VERSION}")
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/src)
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(DOXYGEN_QUIET YES)
# set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXCLUDE_PATTERNS "*/internal/*" "fmt.hpp")

set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_GENERATE_MAN YES)
set(DOXYGEN_GENERATE_HTML YES)

doxygen_add_docs(
  cpp_docs ${DOXYGEN_INPUT_DIR} COMMENT "Generate docs for C++"
)

set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/xml/index.xml)

find_package(Sphinx REQUIRED)
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
file(GLOB_RECURSE SPHINX_RST_SOURCES "*.rst")

file(GLOB_RECURSE SPHINX_MD_SOURCES "*.md")

set(SPHINX_DEPENDS
    ${PROJECT_SOURCE_DIR}/README.md ${DOXYGEN_INDEX_FILE} ${SPHINX_RST_SOURCES}
    ${SPHINX_MD_SOURCES} ${SPHINX_SOURCE}/conf.py
)

# Only regenerate Sphinx when:
#
# - Doxygen has rerun
# - Our doc files have been updated
# - The Sphinx config has been updated
add_custom_target(
  sphinx_docs
  BYPRODUCTS ${SPHINX_INDEX_FILE}
  COMMAND
    ${SPHINX_EXECUTABLE} -b html
    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.signal_tl=${DOXYGEN_OUTPUT_DIRECTORY}/xml ${SPHINX_SOURCE}
    ${SPHINX_BUILD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${SPHINX_DEPENDS}
  COMMENT "Generating documentation with Sphinx"
  ALL
)
add_dependencies(sphinx_docs cpp_docs)

# Nice named target so we can run the job easily
add_custom_target(docs ALL)
add_dependencies(docs sphinx_docs cpp_docs)

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD} DESTINATION ${CMAKE_INSTALL_DOCDIR})
