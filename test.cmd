@echo off

git submodule update --init || exit $?

cmake -B build -D REDUX_BUILD_TESTING=ON || exit $?
cmake --build build || exit $?
ctest --test-dir build --output-on-failure --no-tests=error || exit $?
