#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "argus/argus.hpp"
#include "argus/core.hpp"

int main(int argc, char* argv[]) {
  int result = Catch::Session().run(argc, argv);

  return result;
}
