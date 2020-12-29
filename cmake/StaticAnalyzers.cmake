option(ENABLE_ALL_STATIC_ANALYZERS "Enable clang-tidy and include-what-you-use"
       OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy"
       ${ENABLE_ALL_STATIC_ANALYZERS})
option(ENABLE_INCLUDE_WHAT_YOU_USE
       "Enable static analysis with include-what-you-use"
       ${ENABLE_ALL_STATIC_ANALYZERS})

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
                             -extra-arg=-Wno-unknown-warning-option)
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(
      SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()
