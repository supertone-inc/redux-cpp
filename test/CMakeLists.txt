project(redux-test)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(deps/doctest)

target_include_directories(${PROJECT_NAME} PRIVATE deps/RxCpp/Rx/v2/src)

target_link_libraries(
    ${PROJECT_NAME}
    doctest
    redux
)

list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)
include(doctest)
doctest_discover_tests(${PROJECT_NAME})
