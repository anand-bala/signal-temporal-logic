if(NOT ENABLE_CACHE)
  return()
endif()

set(CACHE_OPTION_VALUES "sccache" "ccache")

message(STATUS "Compiler caching enabled")
message(CHECK_START "Finding suitable compiler caching tool")
foreach(cache_option ${CACHE_OPTION_VALUES})
  # Try to find a suitable cache compiler. Provide PROJECT_SOURCE_DIR and
  # FETCHCONTENT_BASE_DIR as hints as we will download the Linux sccache
  # binaries there in the CI.
  find_program(
    CACHE_BINARY ${cache_option} HINTS "${PROJECT_SOURCE_DIR}"
                                       "${FETCHCONTENT_BASE_DIR}"
  )
  if(CACHE_BINARY)
    break()
  endif()
endforeach()

if(NOT CACHE_BINARY)
  message(CHECK_FAIL "not found")
  message(WARNING "Compiler caching is enabled but not using it.")
  return()
endif()

message(CHECK_PASS "found ${CACHE_BINARY} (enabled)")

set(C_LAUNCHER "${CACHE_BINARY}")
set(CXX_LAUNCHER "${CACHE_BINARY}")

if(CMAKE_GENERATOR STREQUAL "Xcode")
  configure_file(${CMAKE_CURRENT_LIST_DIR}/launch-c.in launch-c)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/launch-cxx.in launch-cxx)

  execute_process(
    COMMAND chmod a+rx "${CMAKE_CURRENT_BINARY_DIR}/launch-c"
            "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx"
  )

  # Set Xcode project attributes to route compilation and linking through our
  # scripts
  set(CMAKE_XCODE_ATTRIBUTE_CC
      "${CMAKE_CURRENT_BINARY_DIR}/launch-c"
      CACHE INTERNAL ""
  )
  set(CMAKE_XCODE_ATTRIBUTE_CXX
      "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx"
      CACHE INTERNAL ""
  )
  set(CMAKE_XCODE_ATTRIBUTE_LD
      "${CMAKE_CURRENT_BINARY_DIR}/launch-c"
      CACHE INTERNAL ""
  )
  set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS
      "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx"
      CACHE INTERNAL ""
  )
else()
  # Support Unix Makefiles and Ninja
  set(CMAKE_C_COMPILER_LAUNCHER
      "${C_LAUNCHER}"
      CACHE INTERNAL ""
  )
  set(CMAKE_CXX_COMPILER_LAUNCHER
      "${CXX_LAUNCHER}"
      CACHE INTERNAL ""
  )
endif()
