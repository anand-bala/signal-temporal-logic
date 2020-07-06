message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

set(TEST_SRCS ${CMAKE_CURRENT_LIST_DIR}/test_signals.cc)

# include(CMakePrintHelpers) cmake_print_properties(TARGETS Catch2::Catch2
# PROPERTIES INTERFACE_INCLUDE_DIRECTORIES)

add_executable(signaltl_tests ${TEST_SRCS})
target_include_directories(signaltl_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(signaltl_tests PUBLIC signal_tl::signal_tl Catch2::Catch2)

include(Catch)
catch_discover_tests(signaltl_tests)
