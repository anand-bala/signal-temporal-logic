include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.4
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 7.1.3
)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG stable
)

FetchContent_Declare(
  PEGTL
  GIT_REPOSITORY https://github.com/taocpp/PEGTL
  GIT_TAG 3.1.0
)

FetchContent_MakeAvailable(Catch2 fmt PEGTL pybind11)
