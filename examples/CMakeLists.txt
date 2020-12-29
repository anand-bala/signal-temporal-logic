message(STATUS "Building Examples in ${CMAKE_CURRENT_LIST_DIR}")

unset(CMAKE_CXX_CLANG_TIDY)
unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)

function(add_example TARGET)
  add_executable(${TARGET} ${ARGN})
  target_link_libraries(${TARGET} PUBLIC signaltl)
  # if(SIGNALTL_COVERAGE) add_coverage(${TARGET}) endif()
endfunction()

add_example(basic_formulas ${CMAKE_CURRENT_LIST_DIR}/basic_formulas.cc)
add_example(basic_signal ${CMAKE_CURRENT_LIST_DIR}/basic_signal.cc)
add_example(basic_robustness ${CMAKE_CURRENT_LIST_DIR}/basic_robustness.cc)
