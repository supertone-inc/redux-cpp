project(redux-test)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    store.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        deps/doctest/doctest
        deps/trompeloeil/include
        deps/RxCpp/Rx/v2/src
)

target_link_libraries(
    ${PROJECT_NAME}
    redux
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/doctest/scripts/cmake)
include(doctest)
doctest_discover_tests(${PROJECT_NAME})
