---
phase: 1
plan: 2
wave: 2
depends_on:
  - 1.1
files_modified:
  - benchmarks/benchmark.cpp
  - CMakeLists.txt
  - build.bat
  - build.sh
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "CMakeLists.txt builds apexkv, test_apexkv, and benchmark_apexkv with -Wall -Wextra"
    - "test_apexkv is registered as a test with ctest"
    - "build.bat and build.sh build all three executables cleanly with g++"
  artifacts:
    - "benchmarks/benchmark.cpp exists as valid benchmark runner target"
    - "CMakeLists.txt defines apexkv, test_apexkv, benchmark_apexkv and ctest integration"
---

# Plan 1.2: CMake Targets, Benchmark Stub & Build Runner Updates

## Objective
Update `CMakeLists.txt` to build `apexkv`, `test_apexkv`, and `benchmark_apexkv` with `-Wall -Wextra` warnings and register tests with `ctest`. Create the `benchmarks/benchmark.cpp` baseline entry point and update the convenience build scripts (`build.bat` and `build.sh`) to build all three binaries.

## Context
- `prd.md`
- `.gsd/SPEC.md`
- `CMakeLists.txt`
- `build.bat`
- `build.sh`

## Tasks

<task type="auto">
  <name>Create baseline benchmark runner in benchmarks/benchmark.cpp</name>
  <files>
    benchmarks/benchmark.cpp
  </files>
  <action>
    1. Create `benchmarks/benchmark.cpp`.
    2. Implement a minimal, clean C++17 benchmark entry point that includes `hash_table.hpp`.
    3. Output initial baseline messages (e.g. running basic sequential insert timing with `std::chrono::high_resolution_clock`) without inventing false benchmark numbers.
    4. Ensure it compiles cleanly with zero warnings under `-std=c++17 -Wall -Wextra`.

    AVOID: Complex benchmarking frameworks or fake benchmark numbers.
    USE: Plain standard library `<chrono>`, `<iostream>`, `<string>` and `hash_table.hpp`.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include benchmarks/benchmark.cpp -o build/benchmark_apexkv.exe && ./build/benchmark_apexkv.exe
  </verify>
  <done>
    `benchmarks/benchmark.cpp` compiles cleanly and executes a baseline timing check.
  </done>
</task>

<task type="auto">
  <name>Update CMakeLists.txt, build.bat, and build.sh</name>
  <files>
    CMakeLists.txt
    build.bat
    build.sh
  </files>
  <action>
    1. In `CMakeLists.txt`:
       - Set C++17 standard required.
       - Apply `-Wall -Wextra` flags for GCC/Clang (and `/W4` on MSVC).
       - Add executable `apexkv` using `main.cpp`.
       - Add executable `test_apexkv` using `tests/test_apexkv.cpp`.
       - Add executable `benchmark_apexkv` using `benchmarks/benchmark.cpp`.
       - Add include directory `include` to all three targets.
       - Enable testing via `enable_testing()` and register `add_test(NAME test_apexkv COMMAND test_apexkv)`.
    2. Update `build.bat` and `build.sh`:
       - Compile `apexkv.exe`, `test_apexkv.exe`, and `benchmark_apexkv.exe` into `build/` using `g++ -std=c++17 -Wall -Wextra -O2 -I include`.
       - Verify exit codes for each step.
  </action>
  <verify>
    cmd.exe /c build.bat
  </verify>
  <done>
    `CMakeLists.txt` defines `apexkv`, `test_apexkv`, `benchmark_apexkv` with `ctest`, and `build.bat` successfully compiles all 3 targets.
  </done>
</task>

## Success Criteria
- [ ] `benchmarks/benchmark.cpp` exists and compiles cleanly
- [ ] `CMakeLists.txt` defines `apexkv`, `test_apexkv`, `benchmark_apexkv` with `-Wall -Wextra` and `ctest` test registration
- [ ] `build.bat` executes without error and builds all 3 executables
- [ ] All targets compile with zero warnings
