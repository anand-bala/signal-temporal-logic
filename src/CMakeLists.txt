set(STL_SOURCES ast.cc classic_robustness.cc minmax.cc signal.cc)

add_library(signaltl ${STL_SOURCES})
add_library(signal_tl::signal_tl ALIAS signaltl)
target_include_directories(
  signaltl PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_compile_features(signaltl PUBLIC cxx_std_17)
target_link_libraries(signaltl PRIVATE mono_wedge::mono_wedge)
target_link_libraries(signaltl PUBLIC fmt::fmt)
set_target_properties(signaltl PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(signaltl PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(signaltl PRIVATE -pedantic -Wall -Wextra -Werror)
endif()

# if(SIGNALTL_COVERAGE)
# add_coverage(signaltl)
# endif()
