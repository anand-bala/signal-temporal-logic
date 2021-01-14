#include <iostream>

#if __cplusplus >= 201703L

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Missing the <filesystem> and <experimental/filesystem> headers."
#endif

#else
#error "Need to compile with C++17 support"
#endif

int main() {
  try {
    fs::path path("/root");
    (void)path;
  } catch (const fs::filesystem_error& e) {
    std::cerr << "filesystem_error workaround: " << e.what() << std::endl;
  }
}
