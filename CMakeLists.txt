cmake_minimum_required(VERSION 3.14)
project(ApexKV VERSION 2.0.0 LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Include directories
set(APEXKV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Targets
add_executable(apexkv main.cpp)
target_include_directories(apexkv PRIVATE ${APEXKV_INCLUDE_DIR})

add_executable(test_apexkv tests/test_apexkv.cpp)
target_include_directories(test_apexkv PRIVATE ${APEXKV_INCLUDE_DIR})

add_executable(benchmark_apexkv benchmarks/benchmark.cpp)
target_include_directories(benchmark_apexkv PRIVATE ${APEXKV_INCLUDE_DIR})

# CTest Registration
enable_testing()
add_test(NAME test_apexkv COMMAND test_apexkv)
