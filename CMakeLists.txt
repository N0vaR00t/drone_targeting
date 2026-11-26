cmake_minimum_required(VERSION 3.16)
project(DropCalc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(ENABLE_SANITIZERS AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "ENABLE_SANITIZERS requires a GCC- or Clang-compatible compiler")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_drop_calc_testing_default ON)
else()
    set(_drop_calc_testing_default OFF)
endif()
option(BUILD_TESTING "Build and run tests as part of the Debug build" ${_drop_calc_testing_default})

add_library(drop_calc_library STATIC library.cpp provided_coverage.h)
target_include_directories(drop_calc_library PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(drop_calc main.cpp)
target_link_libraries(drop_calc PRIVATE drop_calc_library)

if(BUILD_TESTING)
    include(CTest)
    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0)
    FetchContent_MakeAvailable(googletest)

    add_executable(drop_core_tests
        testing/library_unit_tests.cpp
        testing/library_regression_tests.cpp
    )
    target_link_libraries(drop_core_tests PRIVATE drop_calc_library GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(drop_core_tests DISCOVERY_MODE PRE_TEST)

    add_custom_target(check_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS drop_core_tests
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        USES_TERMINAL
        COMMENT "Running DropCalc tests"
    )
endif()

if(ENABLE_SANITIZERS)
    target_compile_options(drop_calc_library PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    set(_drop_calc_sanitizer_targets drop_calc)
    if(BUILD_TESTING)
        list(APPEND _drop_calc_sanitizer_targets drop_core_tests)
    endif()
    foreach(target IN LISTS _drop_calc_sanitizer_targets)
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${target} PRIVATE
            -fsanitize=address,undefined
        )
    endforeach()
endif()
