set(STL_SOURCES
    ast.cc
    classic_robustness.cc
    minmax.cc
    signal.cc)

set(SIGNALTL_CORE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(signaltl ${STL_SOURCES})
target_include_directories(signaltl PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                                           $<BUILD_INTERFACE:${SIGNALTL_CORE_INCLUDE_DIRS}>)
target_compile_features(signaltl PUBLIC cxx_std_17)
target_link_libraries(signaltl PUBLIC fmt::fmt)
set_target_properties(signaltl PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_default_compile_options(signaltl)
set_project_warnings(signaltl)
enable_clang_tidy(signaltl)
enable_include_what_you_use(signaltl)
add_coverage(signaltl)

