project(tests)

add_executable(
    ${PROJECT_NAME}
    basic.cpp
    main.cpp
    middleware.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        deps/doctest/doctest
        deps/trompeloeil/include
        deps/RxCpp/Rx/v2/src
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

target_link_libraries(${PROJECT_NAME} redux)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/doctest/scripts/cmake)
include(doctest)
doctest_discover_tests(${PROJECT_NAME})
