# SPEC.md — Project Specification: ApexKV v2

> **Status**: `FINALIZED`
>
> ⚠️ **Planning Lock**: No code may be written until this spec is marked `FINALIZED`.

## Vision
ApexKV v2 hardens and extends an existing C++17 in-memory key-value store with durability (binary length-prefixed WAL with CRC32 checksums), atomic crash-consistent snapshotting, time-to-live (TTL) expiration, thread safety (`std::shared_mutex`), and an interactive CLI. Designed to be cleanly explainable in top-tier technical interviews by a 3rd-year B.Tech CS student with solid DSA and C++ fundamentals.

## Goals
1. **Housekeeping & Build**: Clean repository hygiene (delete stale `src/hash_table.cpp`, untrack executables, fix `.gitignore`), and modern standard CMake build targets (`apexkv`, `test_apexkv`, `benchmark_apexkv`) with `-Wall -Wextra` and `ctest` support.
2. **Robust HashTable**: Vector-backed separate chaining (`std::vector<std::forward_list<Pair>>`) eliminating raw `new[]`/`delete[]`, with pointer-returning `find()` to avoid redundant lookups. Comprehensive tests covering collisions, 0.75 rehash threshold, update semantics, and middle-of-chain removal.
3. **Crash-Consistent WAL v2**: Binary length-prefixed write-ahead logging (op, key len, value len, key, value, CRC32) supporting arbitrary keys/values (spaces, newlines, binary). Atomic temp-file snapshots with rename, corruption truncation on recovery, and torn-write / bad-checksum unit tests.
4. **Time-To-Live (TTL)**: Lazy expiration on access (`get`/`contains`) with persistent expiration timestamps surviving crash restarts through the log.
5. **Thread Safety & CLI**: Concurrent read/write safety via coarse `std::shared_mutex` (shared for reads, exclusive for writes), tested under multi-threaded load. Interactive CLI supporting `SET`, `GET`, `DEL`, `EXPIRE`.
6. **Empirical Quality & Documentation**: Rigorous benchmark comparisons against `std::unordered_map` with honest metrics, memory validation via `-fsanitize=address,undefined` and valgrind, GitHub Actions CI workflow, and clear documentation.

## Hard Rules
- Code simplicity: Written as a 3rd-year B.Tech CS student with solid C++ and DSA basics. No template metaprogramming, no over-abstraction. Short comments only where non-obvious.
- Interview explainability: Every line must be defensible in an engineering interview.
- Honesty: Never invent benchmark numbers or claim unimplemented features.
- Atomic commits: One small atomic commit per task with plain, clear messages. Never alter commit dates.
- Style: `snake_case`, trailing underscore on private/member variables (`table_`, `cap_`), plain C++17, standard library only, zero dependencies.

## Constraints
- **Language Standard**: C++17 (`-std=c++17`).
- **Dependencies**: Standard library only (Zero third-party libraries).
- **Tooling**: CMake 3.14+, GCC/Clang with `-Wall -Wextra`.

---

*Last updated: 2026-10-14*
