if(NOT ENABLE_TESTING)
  return()
endif()

message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

add_executable(
  argus_tests
  test_main.cc
  ast/test_structure.cc
  parser/test_parsing.cc
  parser/test_grammar.cc
  signals/test_signals.cc
  # signaltl_tests.cc
  # test_append_error.cc
  # test_parser.cc
  # test_signals.cc
  utils/test_visit.cc
  custom_generators.hpp
)
target_include_directories(
  argus_tests
  PRIVATE ${CMAKE_CURRENT_LIST_DIR} # Needed for local stuff
          ${PROJECT_SOURCE_DIR}/src # Needed for private sources like utils
)
target_link_libraries(
  argus_tests
  PUBLIC Catch2::Catch2 argus
  PRIVATE taocpp::pegtl magic_enum::magic_enum
)
set_default_compile_options(argus_tests)
add_coverage_flags(argus_tests)
catch_discover_tests(argus_tests)
