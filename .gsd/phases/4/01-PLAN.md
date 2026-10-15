---
phase: 4
plan: 1
wave: 1
depends_on: []
files_modified:
  - include/persistent_kv.hpp
  - tests/test_ttl.cpp
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "PersistentKV supports put with TTL expiration seconds"
    - "Expired keys are lazily removed on get() and contains()"
    - "TTL expiration timestamps survive process restart via WAL"
    - "Unit test suite in tests/test_ttl.cpp passes 100%"
  artifacts:
    - "include/persistent_kv.hpp"
    - "tests/test_ttl.cpp"
---

# Plan 4.1: Time-To-Live (TTL) Key Expiration Subsystem

## Objective
Implement Time-To-Live (TTL) support in `PersistentKV`. Keys can be inserted with an optional expiration time in seconds, or assigned TTL via `expire()`. Expiration is lazily enforced on `get()` and `contains()`. Expiration timestamps are serialized into the binary WAL using `WalOp::PUT_TTL` and `WalOp::EXPIRE`, enabling full recovery of active and expired states across process restarts.

## Context
- `prd.md` Phase 4
- `include/persistent_kv.hpp`
- `tests/test_wal.cpp`

## Tasks

<task type="auto">
  <name>Implement TTL and WAL logging in PersistentKV</name>
  <files>
    include/persistent_kv.hpp
  </files>
  <action>
    1. Add `WalOp::PUT_TTL = 3` and `WalOp::EXPIRE = 4`.
    2. Add `std::unordered_map<std::string, int64_t> expiries_` to track expiration timestamps (Unix epoch seconds).
    3. Implement `append_put_ttl()` and `append_expire()` with CRC32 checksums.
    4. Implement `put(key, value, ttl_seconds)` overload (for string and int values).
    5. Implement `expire(key, ttl_seconds)` and `ttl(key)`.
    6. Implement lazy removal in `get()` and `contains()`.
    7. Update `replay_wal()` to restore TTL timestamps from `PUT_TTL` and `EXPIRE` log records.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include tests/test_wal.cpp -o build/test_wal.exe && .\build\test_wal.exe
  </verify>
  <done>
    `include/persistent_kv.hpp` implements TTL and all WAL tests continue to pass.
  </done>
</task>

<task type="auto">
  <name>Create comprehensive TTL test suite in tests/test_ttl.cpp</name>
  <files>
    tests/test_ttl.cpp
  </files>
  <action>
    1. Create `tests/test_ttl.cpp` with tiny assert test helpers (`TEST`, `PASS`).
    2. Test basic TTL expiration (put with short TTL, verify alive immediately, sleep past TTL, verify lazy removal on get/contains).
    3. Test update preserves or changes TTL.
    4. Test manual `expire()` command on existing key without TTL.
    5. Test `ttl()` query returns remaining seconds, -1 for persistent key, -2 for non-existent.
    6. Test WAL restart recovery: keys with future TTL remain valid after restart; expired keys remain expired after restart.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include tests/test_ttl.cpp -o build/test_ttl.exe && .\build\test_ttl.exe
  </verify>
  <done>
    All TTL tests compile cleanly and pass 100%.
  </done>
</task>
