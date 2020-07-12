message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

find_package(Catch2 CONFIG REQUIRED)
if(Catch2_FOUND)
    message(STATUS "Found Catch2 ${Catch2_VERSION}")
endif()

set(TEST_SRCS ${CMAKE_CURRENT_LIST_DIR}/test_signals.cc)

# include(CMakePrintHelpers) cmake_print_properties(TARGETS Catch2::Catch2
# PROPERTIES INTERFACE_INCLUDE_DIRECTORIES)

add_executable(signaltl_tests ${TEST_SRCS})
target_include_directories(signaltl_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(signaltl_tests PUBLIC signal_tl::signal_tl Catch2::Catch2)

include(Catch)
catch_discover_tests(signaltl_tests)
