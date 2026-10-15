---
milestone: v2.0
version: 2.0.0
updated: 2026-10-14
---

# Roadmap: ApexKV v2

> **Current Phase:** Phase 1 - Housekeeping and Build
> **Status:** complete
> **Milestone:** v2.0 - Harden & Extend ApexKV C++17 Store

## Must-Haves (from prd.md)

- [x] Clean repository hygiene: no committed binaries/logs/snapshots, correct `.gitignore`
- [x] Unified CMakeLists.txt building `apexkv`, `test_apexkv`, and `benchmark_apexkv` with `-Wall -Wextra` and `ctest`
- [x] Safe `HashTable` backed by `std::vector<std::forward_list<Pair>>` with `find()` returning `Pair*`
- [x] Comprehensive unit tests with tiny assert helper covering collisions, rehash at 0.75 boundary, updates, middle-of-chain removal
- [x] Binary length-prefixed WAL v2 (op, key len, val len, key, val, CRC32) with corruption truncation and atomic snapshot rename
- [x] TTL key expiration subsystem surviving crash restarts via log
- [x] Reader-writer concurrency via `std::shared_mutex` in `PersistentKV` with multi-threaded tests
- [x] Interactive REPL/CLI for `SET`, `GET`, `DEL`, `EXPIRE`
- [x] Automated benchmarks vs `std::unordered_map` with honest measurements
- [x] Clean sanitizer/valgrind passes, GitHub Actions CI workflow, and verified `RESUME_NOTES.md`

---

## Phases

### Phase 1: Housekeeping and Build
**Status:** ✅ Complete
**Objective:** Delete stale `src/hash_table.cpp`, remove committed `.exe`/`.log`/`.snapshot` files, rename `gitignore` to `.gitignore` (ignore `*.exe`, `*.log`, `*.snapshot`, `build/`), fix `CMakeLists.txt` to build `apexkv`, `test_apexkv`, `benchmark_apexkv` with `-Wall -Wextra` and register tests with `ctest`.

---

### Phase 2: HashTable and Tests
**Status:** ✅ Complete
**Objective:** Replace raw `new[]`/`delete[]` with `std::vector<std::forward_list<Pair>>`; add `find()` returning pointer (`nullptr` if missing). Make `tests/test_apexkv.cpp` compile with a tiny assert-based helper. Add tests: forced collisions, rehash at 0.75 boundary, update existing key, remove from middle of chain, restart recovery.
**Depends on:** Phase 1

---

### Phase 3: WAL v2 (Write-Ahead Logging & Crash Consistency)
**Status:** ✅ Complete
**Objective:** Implement binary length-prefixed records (op, key len, value len, key, value, CRC32); keep the log stream open as a member; support keys/values with spaces and newlines; on startup stop at first corrupt/truncated record and truncate the log; write snapshots to a temp file then rename. Unit tests for torn last record and bad checksum.
**Depends on:** Phase 2

---

### Phase 4: Time-To-Live (TTL)
**Status:** ✅ Complete
**Objective:** Support `put` with expiry; lazy removal on `get`/`contains`; ensure expiry timestamps survive restart through the log. Add comprehensive tests for expired vs active keys.
**Depends on:** Phase 3

---

### Phase 5: Thread Safety and CLI
**Status:** ✅ Complete
**Objective:** Add coarse `std::shared_mutex` in `PersistentKV` (shared for reads, exclusive for writes) plus a multi-threaded test. Turn `main.cpp` into a small interactive CLI supporting `SET`, `GET`, `DEL`, `EXPIRE`.
**Depends on:** Phase 4

---

### Phase 6: Quality, CI and Documentation
**Status:** ✅ Complete
**Objective:** Benchmark with pre-generated keys, several repetitions, measuring ops/sec vs `std::unordered_map` (reporting honestly). Run tests under `-fsanitize=address,undefined` and valgrind and fix all issues. Add GitHub Actions workflow. Update `README.md` to document only what exists. Write uncommitted `RESUME_NOTES.md` with verified facts.
**Depends on:** Phase 5

---

## Status Icons
- ⬜ Not Started
- 🔄 In Progress / Planning
- ✅ Complete
- ⏸️ Paused
- ❌ Blocked
