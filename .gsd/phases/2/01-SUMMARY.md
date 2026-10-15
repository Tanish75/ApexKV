# Plan 2.1 Summary: Modernize HashTable Storage & Comprehensive Unit Test Suite

## Execution Details
- **Phase:** 2 (HashTable and Tests)
- **Plan:** 2.1
- **Status:** Complete
- **Commits:**
  - `99c8a38 refactor(hash_table): replace raw array with std::vector buckets and add find()`
  - `5889c4d test(phase-2): add assert helper, collision, rehash boundary, and recovery tests`

## What Was Done
1. **HashTable Storage Modernization:**
   - Eliminated raw dynamic array `new[]` and `delete[]`.
   - Replaced storage with `std::vector<std::forward_list<Pair>> table_`.
   - Applied snake_case with trailing underscores for private members (`table_`, `cap_`, `sz_`, `max_load_`).
   - Implemented `find(const std::string& key)` and `const find(const std::string& key) const` returning `Pair*` (`nullptr` if absent).
   - Provided proper Rule of 5 move semantics zeroing out source size and capacity.
2. **Comprehensive Unit Testing Suite:**
   - Implemented tiny assert helper macros (`TEST(name)`, `PASS()`) tracking total and passed test counts.
   - Added test for `find()` pointer accessor and in-place value mutation through returned pointer.
   - Added test for forced bucket collisions (all items colliding in bucket 0).
   - Added test for removal from middle of collision chain (head, middle, and tail node deletions).
   - Added test for dynamic rehash at exact 0.75 threshold boundary (capacity doubling on 4th element with initial capacity 4).
   - Added restart recovery test validating persistence across `PersistentKV` instances.

## Verification
- Ran `.\build\test_apexkv.exe`:
  ```
  === Running ApexKV Phase 2 Tests ===
    TEST: insert and search... PASS
    TEST: update existing key... PASS
    TEST: basic remove... PASS
    TEST: size and capacity tracking... PASS
    TEST: find() pointer accessor and in-place mutation... PASS
    TEST: forced collisions in single bucket... PASS
    TEST: removal from head, middle, and tail of collision chain... PASS
    TEST: dynamic rehash at exact 0.75 threshold boundary... PASS
    TEST: copy constructor and assignment (deep copy)... PASS
    TEST: move constructor and assignment... PASS
    TEST: restart recovery with PersistentKV WAL... PASS

  === Test Results: 11/11 PASSED ===
  ```
- Ran `.\build.bat` with zero compiler warnings under `-Wall -Wextra`.
- Ran `.\build\benchmark_apexkv.exe` (PUT throughput: ~3.7M ops/sec, GET throughput: ~33.7M ops/sec).
