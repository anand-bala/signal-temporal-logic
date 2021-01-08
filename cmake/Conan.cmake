# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
       "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
  )
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

# conan_add_remote(
#   NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan
# )

conan_cmake_run(
  CONANFILE conanfile.txt
  BUILD outdated
  BASIC_SETUP CMAKE_TARGETS OUTPUT_QUIET
)

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake")
  include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
else()
  message(FATAL_ERROR "Conan did not generate paths file!")
endif()
