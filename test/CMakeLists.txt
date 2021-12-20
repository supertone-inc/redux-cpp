project(redux-test)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(deps/Catch2)

target_include_directories(${PROJECT_NAME} PRIVATE deps/RxCpp/Rx/v2/src)

target_link_libraries(
    ${PROJECT_NAME}
    Catch2::Catch2
    redux
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/Catch2/contrib)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
