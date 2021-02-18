# AST stuff
add_argus_component(argusast ast/ast.cc)

# Parser stuff
add_argus_component(
  argusparser parser/error_messages.hpp parser/actions.hpp parser/parser.cc
  parser/grammar.hpp
)
target_link_libraries(argusparser PUBLIC taocpp::pegtl)
set_std_filesystem_options(argusparser)

# Signals
add_argus_component(argussignals signals/signal.cc)

# Automata
add_subdirectory(automata)

# Algorithms
add_subdirectory(algorithms)

# argus library
add_library(argus libargus.cc)
add_library(argus::argus ALIAS argus)
target_include_directories(
  argus PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(argus PUBLIC taocpp::pegtl argusparser argusast)
target_compile_features(argus PUBLIC cxx_std_17)
set_default_compile_options(argus)
set_project_warnings(argus)
enable_clang_tidy(argus)
enable_include_what_you_use(argus)
add_coverage(argus)

set_target_properties(argus PROPERTIES VERSION ${ARGUS_VERSION})
set_target_properties(argus PROPERTIES SOVERSION ${ARGUS_VERSION_MAJOR})

include(GNUInstallDirs)

install(
  TARGETS argus
  EXPORT argusTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
