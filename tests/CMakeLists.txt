find_package(Catch2 REQUIRED)
message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

add_executable(signaltl_tests signaltl_tests.cc test_append_error.cc test_signals.cc)
target_include_directories(signaltl_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(signaltl_tests PUBLIC signal_tl::core Catch2::Catch2)

add_executable(parser_tests signaltl_tests.cc test_parser.cc)
target_include_directories(parser_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(parser_tests PUBLIC signal_tl::parser Catch2::Catch2)

include(Catch)
catch_discover_tests(signaltl_tests)
catch_discover_tests(parser_tests)

if(ENABLE_COVERAGE)
  add_coverage_flags(signaltl_tests)
  add_coverage_flags(parser_tests)
endif()
