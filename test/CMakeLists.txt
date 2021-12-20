project(redux-test)

add_executable(${PROJECT_NAME} main.cpp)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v2.13.7
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
    RxCpp
    GIT_REPOSITORY  https://github.com/ReactiveX/RxCpp.git
    GIT_TAG         v4.1.1
    GIT_SHALLOW     TRUE
)
if(NOT rxcpp_POPULATED)
    FetchContent_Populate(RxCpp)
    include_directories(${rxcpp_SOURCE_DIR}/Rx/v2/src)
endif()

target_link_libraries(
    ${PROJECT_NAME}
    Catch2::Catch2
    redux
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
