option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy"
       ${ENABLE_STATIC_ANALYSIS}
)
option(ENABLE_INCLUDE_WHAT_YOU_USE
       "Enable static analysis with include-what-you-use"
       ${ENABLE_STATIC_ANALYSIS}
)

option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" ${ENABLE_STATIC_ANALYSIS})

include(CMakeParseArguments)

if(ENABLE_CLANG_TIDY)
  message(CHECK_START "Finding clang-tidy")
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    message(CHECK_PASS "found: ${CLANGTIDY}")
    set(CLANG_TIDY_CMD
        ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option
        CACHE INTERNAL "Command for clang-tidy"
    )
  else()
    message(CHECK_FAIL "not found")
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

function(enable_clang_tidy target)
  message(STATUS "Enabling clang-tidy checks for ${target}")

  if(ENABLE_CLANG_TIDY)
    set_target_properties(
      ${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}"
    )
  endif()
endfunction()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
  message(CHECK_START "Finding include-what-you-use")
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    message(CHECK_PASS "found: ${INCLUDE_WHAT_YOU_USE}")

    set(IWYU_MAPPING_FILE ${PROJECT_SOURCE_DIR}/tools/argus.imp)
    set(INCLUDE_WHAT_YOU_USE_CMD
        ${INCLUDE_WHAT_YOU_USE} -Xiwyu --transitive_includes_only
        -Xiwyu --cxx17ns
        -Xiwyu --mapping_file=${IWYU_MAPPING_FILE}
        CACHE INTERNAL "Command for include-what-you-use"
    )
  else()
    message(CHECK_FAIL "not found")
    message(
      SEND_ERROR "include-what-you-use requested but executable not found"
    )
  endif()
endif()

function(enable_include_what_you_use target)
  message(STATUS "Enabling include-what-you-use checks for ${target}")

  if(ENABLE_INCLUDE_WHAT_YOU_USE)
    set_target_properties(
      ${target} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE
                           "${INCLUDE_WHAT_YOU_USE_CMD}"
    )
  endif()
endfunction()

if(ENABLE_CPPCHECK)
  message(CHECK_START "Finding cppcheck")
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    message(CHECK_PASS "found: ${CPPCHECK}")

    set(CPPCHECK_CMD
        ${CPPCHECK} --enable=information 
        CACHE INTERNAL "Command for cppcheck"
    )
  else()
    message(CHECK_FAIL "not found")
    message(
      SEND_ERROR "cppcheck requested but executable not found"
    )
  endif()
endif()

function(enable_cppcheck target)
  message(STATUS "Enabling cppcheck checks for ${target}")

  get_target_property(target_include_dirs_1 ${target} INCLUDE_DIRECTORIES)
  get_target_property(target_include_dirs_2 ${target} INTERFACE_INCLUDE_DIRECTORIES)
  set(target_includes
      ${_target_include_dirs_1}
      ${_target_include_dirs_2})

  set(cmd ${CPPCHECK_CMD})
  foreach(inc ${target_includes})
    list(APPEND cmd -I ${inc})
  endforeach()

  if(ENABLE_CPPCHECK)
    set_target_properties(
      ${target} PROPERTIES CXX_CPPCHECK
                           "${cmd}"
    )
  endif()
endfunction()

