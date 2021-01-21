# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(set_project_warnings project_name)
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" TRUE)

  set(MSVC_WARNINGS
      /W4 # Baseline reasonable warnings
      /w14640 # Enable warning on thread un-safe static member initialization
      /permissive- # standards conformance mode for MSVC compiler.
      /ignore:4099 # Ignore the "missing PDB" warnings that are issued by fmtlib
  )

  set(CLANG_WARNINGS
      -Wall
      -Wextra # reasonable and standard
      -Wshadow # warn the user if a variable declaration shadows one from a
               # parent context
      -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                         # non-virtual destructor. This helps catch hard to
                         # track down memory errors
      -Wold-style-cast # warn for c-style casts
      -Wunused # warn on anything being unused
      -Wpedantic # warn if non-standard C++ is used
      -Wformat=2 # warn on security issues around functions that format output
                 # (ie printf)
  )

  if(WARNINGS_AS_ERRORS)
    set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
  endif()

  set(GCC_WARNINGS ${CLANG_WARNINGS})

  if(MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else()
    message(
      AUTHOR_WARNING
        "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler."
    )
  endif()

  target_compile_options(${project_name} PRIVATE ${PROJECT_WARNINGS})

endfunction()
