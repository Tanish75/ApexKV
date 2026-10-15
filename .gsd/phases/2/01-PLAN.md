---
phase: 2
plan: 1
wave: 1
depends_on: []
files_modified:
  - include/hash_table.hpp
  - tests/test_apexkv.cpp
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "HashTable is backed by std::vector<std::forward_list<Pair>> eliminating raw new[]/delete[] manual array management"
    - "HashTable provides find() returning Pair* (and const Pair*) with nullptr when key is not present"
    - "test_apexkv.cpp runs with a tiny assert-based helper and passes tests for collisions, rehash boundary, update, chain removal, and restart recovery"
  artifacts:
    - "include/hash_table.hpp"
    - "tests/test_apexkv.cpp"
---

# Plan 2.1: Modernize HashTable Storage & Comprehensive Unit Test Suite

## Objective
Refactor `HashTable` from raw pointer array management (`new[]`/`delete[]`) to safe `std::vector<std::forward_list<Pair>>` storage with trailing underscore members and a `Pair* find()` accessor. Expand `tests/test_apexkv.cpp` with a tiny assert-based test helper and comprehensive unit tests covering forced collisions, rehash threshold boundary (0.75), middle-of-chain deletion, and restart recovery.

## Context
- `prd.md` Phase 2
- `.gsd/SPEC.md`
- `.gsd/ARCHITECTURE.md`
- `include/hash_table.hpp`
- `tests/test_apexkv.cpp`
- `include/persistent_kv.hpp`

## Tasks

<task type="auto">
  <name>Refactor HashTable to std::vector<std::forward_list<Pair>> with find() accessor</name>
  <files>
    include/hash_table.hpp
  </files>
  <action>
    1. Define alias `using Pair = std::pair<std::string, int>;` inside `HashTable`.
    2. Replace raw dynamic array `std::forward_list<Pair>* table;` with `std::vector<std::forward_list<Pair>> table_;`.
    3. Update member names with trailing underscores: `table_`, `cap_`, `sz_`, `max_load_`.
    4. Implement `Pair* find(const std::string& key)` and `const Pair* find(const std::string& key) const` returning pointer to `Pair` or `nullptr` if not found.
    5. Simplify Rule of 5: `std::vector` naturally manages memory safely; default destructor, copy/move constructors, and assignment operators.
    6. Update `put()`, `get()`, `contains()`, `remove()`, `clear()`, and `rehash()` to use the vector of forward lists.
    7. Ensure `getCapacity()`, `getSize()`, `getLoadFactor()`, `getMaxLoadFactor()` remain backward compatible for existing callers.
    8. Adhere to style rules: snake_case, trailing underscore on members, plain C++17, 2nd-year CS student readability.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include tests/test_apexkv.cpp -o build/test_apexkv.exe
  </verify>
  <done>
    `include/hash_table.hpp` uses `std::vector` buckets without any raw `new[]`/`delete[]`, provides `find()` returning `Pair*`, and compiles with zero warnings.
  </done>
</task>

<task type="auto">
  <name>Expand test_apexkv.cpp with tiny assert helper and rigorous test coverage</name>
  <files>
    tests/test_apexkv.cpp
  </files>
  <action>
    1. Implement tiny assert-based helper macros (`TEST(name)` and `PASS()`, maintaining passed and total counters).
    2. Retain existing core tests: insert/search, size/capacity, copy and move semantics.
    3. Add test for `find()` pointer accessor: found returns valid pointer, modifying `pair->second` updates value in place, missing key returns `nullptr`.
    4. Add test for forced collisions: insert multiple keys that hash to identical bucket index in small table and verify retrieval.
    5. Add test for removal from middle of chain: insert 3+ colliding keys in same bucket, delete head, middle, and tail nodes, ensuring chain integrity.
    6. Add test for dynamic rehash at exact 0.75 boundary: track element counts at threshold transition (e.g. at cap 4, inserting 4th element exceeds 0.75 * 4 = 3 and triggers doubling to cap 8).
    7. Add test for restart recovery: use `PersistentKV` with WAL to verify writes survive simulated restart.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include tests/test_apexkv.cpp -o build/test_apexkv.exe && .\build\test_apexkv.exe
  </verify>
  <done>
    All unit tests run and pass 100% with the tiny assert helper reporting detailed test outputs.
  </done>
</task>

## Success Criteria
- [ ] No raw `new[]` or `delete[]` in `include/hash_table.hpp`.
- [ ] `find()` returns `Pair*` (`nullptr` if absent).
- [ ] `tests/test_apexkv.cpp` covers:
  - CRUD operations
  - Copy and move semantics
  - Pointer accessor (`find`)
  - Forced collisions
  - Middle-of-chain removal
  - Rehash at 0.75 boundary
  - Restart recovery with WAL
- [ ] Clean build and 100% test pass on `build.bat` and `ctest`.
