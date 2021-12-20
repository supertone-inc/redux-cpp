#!/bin/bash

set -e

git submodule update --init

cmake -B build -D REDUX_BUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure --no-tests=error
