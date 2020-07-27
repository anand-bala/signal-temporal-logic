#!/bin/bash

set -euo pipefail

mkdir -p build && cd build

cmake --version

# Configure
cmake -DSIGNALTL_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug \
  -DPYTHON_EXECUTABLE=$(python-config --prefix)/bin/python3.6 \
  -DPYTHON_LIBRARY=$(python-config --prefix)/lib/libpython3.6m.so \
  -DPYTHON_INCLUDE_DIR=$(python-config --prefix)/include/python3.6m \
  ..
# Build (for Make on Unix equivalent to `make -j $(nproc)`)
cmake --build . --config Debug -- -j $(nproc)
# Test
ctest -j $(nproc) --output-on-failure

cmake --build . --config Debug --target gcov
