#!/bin/bash

set -e

cmake -B build -D REDUX_BUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure --no-tests=error
