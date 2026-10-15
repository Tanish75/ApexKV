---
phase: 5
plan: 1
wave: 1
depends_on: []
files_modified:
  - include/persistent_kv.hpp
  - tests/test_thread_safety.cpp
  - main.cpp
  - CMakeLists.txt
  - build.bat
  - build.sh
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "Coarse std::shared_mutex is integrated directly inside PersistentKV (shared for reads, exclusive for writes)"
    - "Multi-threaded concurrency tests validate thread safety under parallel readers and writers"
    - "Interactive CLI supports SET, GET, DEL, EXPIRE, TTL, SIZE"
  artifacts:
    - "include/persistent_kv.hpp"
    - "tests/test_thread_safety.cpp"
    - "main.cpp"
---

# Plan 5.1: Thread Safety in PersistentKV & Interactive CLI

## Objective
Embed reader-writer locking directly into `PersistentKV` using `std::shared_mutex`, provide high-concurrency multi-threaded unit tests, and turn `main.cpp` into a production-grade interactive CLI supporting `SET`, `GET`, `DEL`, `EXPIRE`, `TTL`, and `SIZE`.

## Tasks

<task type="auto">
  <name>Embed std::shared_mutex into PersistentKV and create multi-threaded tests</name>
  <files>
    include/persistent_kv.hpp
    tests/test_thread_safety.cpp
  </files>
  <action>
    1. Integrate `mutable std::shared_mutex mutex_` inside `PersistentKV`.
    2. Protect reads with `std::shared_lock` and writes with `std::unique_lock`.
    3. Test with multi-threaded writers, readers, and deletes in `tests/test_thread_safety.cpp`.
  </action>
  <verify>
    .\build\test_thread_safety.exe
  </verify>
  <done>
    All thread safety tests pass with zero race conditions.
  </done>
</task>

<task type="auto">
  <name>Implement interactive CLI in main.cpp</name>
  <files>
    main.cpp
  </files>
  <action>
    1. Implement interactive REPL in `main.cpp` connected directly to `PersistentKV`.
    2. Support `SET`, `GET`, `DEL`, `EXPIRE`, `TTL`, `SIZE`, `HELP`, and `QUIT`.
  </action>
  <verify>
    .\build\apexkv.exe
  </verify>
  <done>
    Interactive CLI runs cleanly and supports all operations.
  </done>
</task>
