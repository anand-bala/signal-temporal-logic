message(STATUS "Building Tests in ${CMAKE_CURRENT_LIST_DIR}")

function(add_test_executable TARGET)
  add_executable(${TARGET} ${ARGN})
  target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(${TARGET} PUBLIC signaltl::signaltl Catch2::Catch2)
  set_default_compile_options(${TARGET})
  add_coverage_flags(${TARGET})
  catch_discover_tests(${TARGET})
endfunction()

add_test_executable(
  signaltl_tests signaltl_tests.cc test_append_error.cc test_signals.cc
)

add_test_executable(parser_tests signaltl_tests.cc test_parser.cc)

# HACK(anand): Hard coding the path to the directory with examples so that
# Catch2 can load it. I am not sure how to do it otherwise...
#
# NOTE: The double quotes must be escaped correctly.
set_target_properties(
  parser_tests PROPERTIES COMPILE_DEFINITIONS
                          "SIGNALTL_TESTS_DIR=\"${CMAKE_CURRENT_LIST_DIR}\""
)
