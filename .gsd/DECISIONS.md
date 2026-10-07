# DECISIONS.md — Architecture Decision Records

> **Purpose**: Log significant technical decisions and their rationale.

---

## Decisions

### [DECISION-001] Milestone Scoping: Finalize Working Core for v1.0, Defer Add-ons to v2.0

**Date**: 2026-10-07
**Status**: Accepted

#### Context
ApexKV has a functional C++17 separate-chaining hash table. We need a solid, robust, and deployable v1.0 release demonstrating production-level code hygiene, tests, benchmarks, cloud deployment on AWS EC2, and documentation before layering complex distributed/server features.

#### Decision
Focus v1.0 strictly on:
1. Hardening existing `HashTable` (memory safety, copy/move semantics, edge case handling).
2. Modern modular CMake build system (`apexkv_lib`, `apexkv_cli`, `apexkv_tests`, `apexkv_bench`).
3. Comprehensive unit test suite covering collisions, resizing, and boundaries.
4. High-precision benchmarking suite measuring throughput (ops/sec) and latency.
5. AWS EC2 Linux deployment scripts and live verification.
6. Interactive CLI REPL demo and production-grade `README.md`.

Advanced features (TCP server, WAL, snapshotting, LRU eviction, TTL, clustering) are formally scheduled for v2.0.

#### Rationale
Delivers an immediate, polished, deployed, and testable portfolio milestone demonstrating strong systems programming fundamentals and cloud deployment skills.

---

### [DECISION-002] Modern C++17 with Modular CMake Targets

**Date**: 2026-10-07
**Status**: Accepted

#### Context
The project needs to build cleanly on both Windows (local dev) and Linux (AWS EC2) with distinct build artifacts for the core library, interactive CLI, unit tests, and benchmark harness.

#### Decision
Structure `CMakeLists.txt` with isolated targets:
- `apexkv_lib`: Static/interface library containing core `HashTable`.
- `apexkv_cli`: Interactive CLI application.
- `apexkv_tests`: Unit test runner.
- `apexkv_bench`: Benchmark suite.

#### Rationale
Provides clean modularity, standard CMake integration (`cmake --build build`), and seamless CI/CD and deployment on AWS EC2.

---

*Last updated: 2026-10-07*
