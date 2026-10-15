# JOURNAL.md — Session Log

> **Purpose**: Chronicle of work sessions for context continuity.

---

## Sessions

## Session: 2026-10-07 22:50

### Objective
Execute Phase 1: Engine Hardening & CMake Build Infrastructure (implement Rule of 5, const-correctness, status indicators, `CMakeLists.txt`, cross-platform build runners, and verify test suite).

### Accomplished
- ✅ Hardened `include/hash_table.hpp` & `src/hash_table.cpp` with copy constructor, copy assignment, move constructor, move assignment, destructor, `clear()`, and `freeTable()`.
- ✅ Added `std::optional<int>` return for `get()`, `contains()`, `bool put()` (insert vs update), and `bool remove()`.
- ✅ Updated `tests/test_apexkv.cpp` with deep copy, move semantics, and dynamic rehash assertions.
- ✅ Created `CMakeLists.txt` with C++17 standard, warning flags, modular targets (`apexkv_lib`, `apexkv_cli`, `apexkv_tests`, `apexkv_bench`), and CTest registration.
- ✅ Added `build.bat` and `build.sh` convenience wrappers for local and EC2 deployments.
- ✅ Verified 100% test pass rate on `test_apexkv.exe` and `apexkv.exe`.
- ✅ Generated `01-SUMMARY.md`, `02-SUMMARY.md`, and `VERIFICATION.md` (Verdict: PASS).

### Verification
- [x] Rule of 5 memory lifecycle verified.
- [x] Dynamic rehash from capacity 16 to 128 verified.
- [x] All Phase 1 tests pass cleanly with zero compiler warnings under `-Wall -Wextra`.

### Blockers Encountered
*None.*

### Handoff Notes
- Next command: `/plan 2` to create execution plans for Phase 2 (Comprehensive Unit Testing & Validation).

---

## Session: 2026-10-15 15:35

### Objective
Complete all remaining phases for ApexKV v2 milestone: Phase 2 (HashTable modern vector backing & find() accessor), Phase 3 (WAL v2 formalization), Phase 4 (TTL key expiration), Phase 5 (Thread safety & CLI REPL), and Phase 6 (Quality, CI, benchmarking, and documentation).

### Accomplished
- ✅ **Phase 2:** Refactored `HashTable` in `include/hash_table.hpp` to `std::vector<std::forward_list<Pair>> table_`, added `Pair* find()` pointer accessor, and expanded `tests/test_apexkv.cpp` to 11 tests.
- ✅ **Phase 3:** Formalized binary length-prefixed WAL v2 with IEEE 802.3 CRC32, crash auto-truncation, and atomic snapshots (`tests/test_wal.cpp`: 11 tests).
- ✅ **Phase 4:** Implemented Time-To-Live (TTL) key expiration with lazy deletion and WAL persistence (`tests/test_ttl.cpp`: 4 tests).
- ✅ **Phase 5:** Embedded coarse `std::shared_mutex` in `PersistentKV` for parallel reader concurrency and exclusive write serialization; built interactive CLI shell in `main.cpp` (`tests/test_thread_safety.cpp`: 4 tests).
- ✅ **Phase 6:** Implemented 5-pass comparative benchmark vs `std::unordered_map` in `benchmarks/benchmark.cpp`, configured GitHub Actions CI with AddressSanitizer and Valgrind in `.github/workflows/ci.yml`, refreshed `README.md`, and generated uncommitted `RESUME_NOTES.md`.

### Verification
- [x] All 4 test suites pass 100% (30/30 tests pass).
- [x] All 6 targets build cleanly under `-Wall -Wextra -O2`.
- [x] Comparative benchmark verified.
- [x] Full milestone v2.0 complete.

---

*Last updated: 2026-10-15*

