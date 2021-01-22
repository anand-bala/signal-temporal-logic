include(CMakeParseArguments)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Choose the type of build." FORCE
  )
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(
    CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                    "RelWithDebInfo"
  )
endif()

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CMAKE_EXPORT_COMPILE_COMMANDS
      ON
      CACHE BOOL "Enable/Disable output of compile commands during generation."
            FORCE
  )
endif()

option(ENABLE_LTO "Enable link time optimization?" OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
  set(ENABLE_LTO
      ON
      CACHE BOOL "Enable link time optimization?" FORCE
  )
endif()

# Set some default options for target
function(set_default_compile_options target)

  if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Force color in compiler output as it will be easier to debug...
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # using Clang
      target_compile_options(${target} PRIVATE -fcolor-diagnostics)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      # using GCC
      target_compile_options(${target} PRIVATE -fdiagnostics-color=always)
    endif()
  endif()

  if(ENABLE_LTO)
    set_target_properties(
      ${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE
    )
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /Zc:__cplusplus)
  endif()

endfunction()

# Set compile and link flags for std::filesystem
function(set_std_filesystem_options target)
  message(CHECK_START "Checking compiler flags for std::filesystem")
  # Check if we need to add -lstdc++fs or -lc++fs or nothing
  if(MSVC)
    set(std_fs_no_lib_needed TRUE)
  else()
    try_compile(
      std_fs_no_lib_needed ${CMAKE_CURRENT_BINARY_DIR}
      SOURCES ${PROJECT_SOURCE_DIR}/cmake/test_std_filesystem.cc
      COMPILE_DEFINITIONS -std=c++17
    )
    try_compile(
      std_fs_needs_stdcxxfs ${CMAKE_CURRENT_BINARY_DIR}
      SOURCES ${PROJECT_SOURCE_DIR}/cmake/test_std_filesystem.cc
      COMPILE_DEFINITIONS -std=c++17
      LINK_LIBRARIES stdc++fs
    )
    try_compile(
      std_fs_needs_cxxfs ${CMAKE_CURRENT_BINARY_DIR}
      SOURCES ${PROJECT_SOURCE_DIR}/cmake/test_std_filesystem.cc
      COMPILE_DEFINITIONS -std=c++17
      LINK_LIBRARIES c++fs
    )
  endif()

  if(${std_fs_needs_stdcxxfs})
    set(std_fs_lib stdc++fs)
    message(CHECK_PASS "${std_fs_lib}")
  elseif(${std_fs_needs_cxxfs})
    set(std_fs_lib c++fs)
    message(CHECK_PASS "${std_fs_lib}")
  elseif(${std_fs_no_lib_needed})
    set(std_fs_lib "")
    message(CHECK_PASS "No flags needed")
  else()
    message(CHECK_FAIL "unknown compiler, not passing -lstdc++fs or -lc++fs")
    set(std_fs_lib "")
  endif()

  target_link_libraries(${target} PUBLIC ${std_fs_lib})
endfunction()
