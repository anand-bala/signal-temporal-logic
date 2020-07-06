
message(STATUS "Building Examples in ${CMAKE_CURRENT_LIST_DIR}")

add_executable(basic_signal ${CMAKE_CURRENT_LIST_DIR}/basic_signal.cc)
target_link_libraries(basic_signal PRIVATE signaltl)

add_executable(basic_robustness ${CMAKE_CURRENT_LIST_DIR}/basic_robustness.cc)
target_link_libraries(basic_robustness PRIVATE signaltl)
