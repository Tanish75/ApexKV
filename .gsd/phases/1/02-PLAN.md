---
phase: 1
plan: 2
wave: 2
depends_on: ["01"]
files_modified:
  - CMakeLists.txt
  - main.cpp
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "CMake configures and builds four distinct targets: apexkv_lib, apexkv_cli, apexkv_tests, and apexkv_bench"
    - "CTest or ctest command executes test target cleanly"
    - "Build succeeds on both Linux (GCC/Clang) and Windows (MSVC/MinGW)"
  artifacts:
    - "CMakeLists.txt contains standard C++17 configuration, warning flags, include directories, and test registration"
---

# Plan 1.2: CMake Build Infrastructure

<objective>
Implement a modern, modular `CMakeLists.txt` build configuration that defines clear build targets (`apexkv_lib` static library, `apexkv_cli` executable, `apexkv_tests` test runner, and `apexkv_bench` benchmark runner), sets compiler warning flags, enables C++17 standard, and integrates CTest.
</objective>

<context>
Load for context:
- .gsd/SPEC.md
- .gsd/REQUIREMENTS.md
- .gsd/phases/1/01-PLAN.md
- CMakeLists.txt
- include/hash_table.hpp
- src/hash_table.cpp
- main.cpp
- tests/test_apexkv.cpp
</context>

<tasks>

<task type="auto">
  <name>Construct Modular CMake Configuration</name>
  <files>
    CMakeLists.txt
    main.cpp
  </files>
  <action>
    1. Write `CMakeLists.txt` with:
       - `cmake_minimum_required(VERSION 3.14)`
       - `project(ApexKV VERSION 1.0.0 LANGUAGES CXX)`
       - `set(CMAKE_CXX_STANDARD 17)`
       - `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
       - Strict compiler warning flags: `-Wall -Wextra -Wpedantic` on GNU/Clang and `/W4` on MSVC.
       - Library target: `add_library(apexkv_lib STATIC src/hash_table.cpp)`
       - Target include directory: `target_include_directories(apexkv_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)`
       - CLI application target: `add_executable(apexkv_cli main.cpp)` linking `apexkv_lib`.
       - Test target: `add_executable(apexkv_tests tests/test_apexkv.cpp)` linking `apexkv_lib`.
       - Enable testing via `enable_testing()` and register `add_test(NAME ApexKVUnitTests COMMAND apexkv_tests)`.
       - Benchmark placeholder target: add conditionally if benchmark source exists or prepare `add_executable(apexkv_bench benchmarks/benchmark.cpp)` with proper checks.
    2. In `main.cpp`, update includes to use `#include "hash_table.hpp"` and utilize updated method return types.

    AVOID: Hardcoding platform-specific compiler paths or binary names.
    USE: CMake standard generator expressions and target-based commands (`target_include_directories`, `target_link_libraries`).
  </action>
  <verify>
    cmake -B build -S . && cmake --build build
  </verify>
  <done>
    Build directory generates successfully and compiles `apexkv_lib`, `apexkv_cli`, and `apexkv_tests` with zero warnings.
  </done>
</task>

<task type="auto">
  <name>Validate CTest Integration and Artifact Generation</name>
  <files>
    CMakeLists.txt
  </files>
  <action>
    1. Verify test execution via CTest / test binary execution in build directory.
    2. Ensure debug and release configurations both succeed without missing symbols.
    3. Ensure clean build artifacts and test reports are generated.

    AVOID: Relying on manual shell compile scripts when CMake is available.
    USE: `ctest --test-dir build --output-on-failure` or direct execution of built test binary.
  </action>
  <verify>
    ctest --test-dir build --output-on-failure || ./build/apexkv_tests || ./build/Debug/apexkv_tests.exe || ./build/apexkv_tests.exe
  </verify>
  <done>
    All registered tests run and report 100% pass rate through the CMake test framework.
  </done>
</task>

</tasks>

<verification>
After all tasks, verify:
- [ ] `cmake -B build -S .` configures cleanly
- [ ] `cmake --build build` compiles all targets
- [ ] Test target runs and passes 100%
</verification>

<success_criteria>
- [ ] Modular CMake targets established for library, application, tests, and benchmarks
- [ ] Cross-platform compilation working on Windows and Linux
- [ ] Zero build warnings under `-Wall -Wextra`
</success_criteria>
