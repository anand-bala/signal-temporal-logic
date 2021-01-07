include(CMakeParseArguments)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Choose the type of build." FORCE
  )
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(
    CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                    "RelWithDebInfo"
  )
endif()

option(ENABLE_LTO "Enable link time optimization?" OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  set(ENABLE_LTO
      ON
      CACHE BOOL "Enable link time optimization?" FORCE
  )
endif()

function(set_default_compile_options target)

  if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Force color in compiler output as it will be easier to debug...
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # using Clang
      target_compile_options(${target} PRIVATE -fcolor-diagnostics)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      # using GCC
      target_compile_options(${target} PRIVATE -fdiagnostics-color=always)
    endif()
  endif()

endfunction()
