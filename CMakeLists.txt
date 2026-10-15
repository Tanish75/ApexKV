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

# Threading support
find_package(Threads REQUIRED)

# Include directories
set(APEXKV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# CLI Target
add_executable(apexkv main.cpp)
target_include_directories(apexkv PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(apexkv PRIVATE Threads::Threads)

# Benchmark Target
add_executable(benchmark_apexkv benchmarks/benchmark.cpp)
target_include_directories(benchmark_apexkv PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(benchmark_apexkv PRIVATE Threads::Threads)

# Test Targets
add_executable(test_apexkv tests/test_apexkv.cpp)
target_include_directories(test_apexkv PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(test_apexkv PRIVATE Threads::Threads)

add_executable(test_wal tests/test_wal.cpp)
target_include_directories(test_wal PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(test_wal PRIVATE Threads::Threads)

add_executable(test_ttl tests/test_ttl.cpp)
target_include_directories(test_ttl PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(test_ttl PRIVATE Threads::Threads)

add_executable(test_thread_safety tests/test_thread_safety.cpp)
target_include_directories(test_thread_safety PRIVATE ${APEXKV_INCLUDE_DIR})
target_link_libraries(test_thread_safety PRIVATE Threads::Threads)

# CTest Registration
enable_testing()
add_test(NAME test_apexkv COMMAND test_apexkv)
add_test(NAME test_wal COMMAND test_wal)
add_test(NAME test_ttl COMMAND test_ttl)
add_test(NAME test_thread_safety COMMAND test_thread_safety)
