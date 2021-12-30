cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 14)

project(redux)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(REDUX_BUILD_TESTING "Build the tests" OFF)
if(REDUX_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
