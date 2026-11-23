cmake_minimum_required(VERSION 3.16)

project(drop_calc LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(ENABLE_SANITIZERS AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "ENABLE_SANITIZERS requires a GCC- or Clang-compatible compiler")
endif()

add_library(drop_calc_library library.cpp provided_coverage.h)
target_include_directories(drop_calc_library PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(drop_calc_library PUBLIC cxx_std_17)
add_executable(drop_calc main.cpp)
target_link_libraries(drop_calc PRIVATE drop_calc_library)

if(ENABLE_SANITIZERS)
    foreach(target IN ITEMS drop_calc_library drop_calc)
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
    endforeach()
    target_link_options(drop_calc PRIVATE -fsanitize=address,undefined)
endif()
