message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

set(TEST_SRCS signaltl_tests.cc test_append_error.cc test_signals.cc)

add_executable(signaltl_tests ${TEST_SRCS})
target_include_directories(signaltl_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(signaltl_tests PUBLIC signal_tl::signal_tl Catch2::Catch2)

include(Catch)
if(SIGNALTL_COVERAGE)
  add_coverage(signaltl_tests)
endif()
catch_discover_tests(signaltl_tests)
