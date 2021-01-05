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

# Try compiling a test program with std::filesystem or one of its alternatives
function(check_filesystem_impl FILESYSTEM_HEADER FILESYSTEM_NAMESPACE OPTIONAL_LIBS
         OUT_RESULT
)
  set(TEST_FILE "test_${OUT_RESULT}.cpp")
  configure_file(${PROJECT_SOURCE_DIR}/cmake/test_filesystem.cpp.in ${TEST_FILE} @ONLY)

  try_compile(
    TEST_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}
                                            CXX_STANDARD 17
  )

  if(NOT TEST_RESULT)
    # Retry with each of the optional libraries
    foreach(OPTIONAL_LIB IN LISTS OPTIONAL_LIBS)
      try_compile(
        TEST_RESULT ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}
        LINK_LIBRARIES ${OPTIONAL_LIB} CXX_STANDARD 17
      )

      if(TEST_RESULT)
        # Looks like the optional library was required, go ahead and add it to the link
        # options.
        message(
          STATUS
            "Adding ${OPTIONAL_LIB} to the PEGTL to build with ${FILESYSTEM_NAMESPACE}."
        )
        target_link_libraries(${PROJECT_NAME} INTERFACE ${OPTIONAL_LIB})
        break()
      endif()
    endforeach(OPTIONAL_LIB)
  endif()

  set(${OUT_RESULT}
      ${TEST_RESULT}
      PARENT_SCOPE
  )
endfunction(check_filesystem_impl)
