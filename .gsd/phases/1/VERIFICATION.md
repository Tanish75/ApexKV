# Phase 1 Verification: Housekeeping and Build

## Must-Haves Verification

| Must-Have Requirement | Status | Evidence |
|-----------------------|--------|----------|
| Delete stale `src/hash_table.cpp` | **VERIFIED** | `Test-Path src/hash_table.cpp` returns `False`; `HashTable` is fully self-contained in `include/hash_table.hpp`. |
| Remove committed `.exe`, `.log`, `.snapshot` files | **VERIFIED** | `git ls-files '*.exe' '*.log' '*.snapshot'` returns 0 files; `apexkv.exe` and `test_apexkv.exe` removed from git index. |
| Rename `gitignore` to `.gitignore` and ignore `*.exe`, `*.log`, `*.snapshot`, `build/` | **VERIFIED** | `Test-Path .gitignore` is `True`, `gitignore` is `False`. `.gitignore` contains rules for `*.exe`, `*.o`, `*.a`, `build/`, `*.log`, `*.snapshot`. |
| Fix `CMakeLists.txt` to build `apexkv`, `test_apexkv`, `benchmark_apexkv` with `-Wall -Wextra` | **VERIFIED** | Targets `apexkv`, `test_apexkv`, and `benchmark_apexkv` configured in `CMakeLists.txt` with `-Wall -Wextra` compile options. |
| Register test target with `ctest` | **VERIFIED** | `enable_testing()` and `add_test(NAME test_apexkv COMMAND test_apexkv)` present in `CMakeLists.txt`. |
| Unit tests compile and pass | **VERIFIED** | All tests in `test_apexkv.cpp` compile with `-Wall -Wextra` and pass with 100% success rate. |

---

## Empirical Verification Evidence

```
[1/3] Compiling interactive CLI (apexkv.exe)...
  - Built build\apexkv.exe
[2/3] Compiling Unit Test Runner (test_apexkv.exe)...
  - Built build\test_apexkv.exe
[3/3] Compiling Benchmark Runner (benchmark_apexkv.exe)...
  - Built build\benchmark_apexkv.exe

[SUCCESS] Build complete!

=== Running ApexKV Phase 1 Tests ===
PASS: insert and search
PASS: update
PASS: remove
PASS: size and capacity
PASS: copy semantics
PASS: move semantics
PASS: dynamic rehash
=== ALL PHASE 1 TESTS PASSED ===
```

---

## Verdict: PASS
All requirements from `prd.md` for Phase 1 are satisfied.
