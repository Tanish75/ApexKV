# SPEC.md — Project Specification

> **Status**: `FINALIZED`
>
> ⚠️ **Planning Lock**: No code may be written until this spec is marked `FINALIZED`.

## Vision
ApexKV is a fast, lightweight in-memory key-value store built from scratch in C++17 utilizing a separate-chaining hash table with dynamic rehashing and load factor management. Designed as a clean systems programming portfolio project for SDE/backend roles at top tech companies, ApexKV v1.0 focuses on clean OOP design, empirical benchmarking, comprehensive unit testing, production-ready documentation, interactive live demos, and deployment on an AWS EC2 Linux server.

## Goals
1. **Engine Polish & Robustness**: Finalize and harden the current C++17 separate-chaining hash table engine (`put`, `get`, `remove`, `getSize`, `loadFactor`, `rehash`) with rule-of-five compliance, clean memory management, edge-case safety, and modern CMake build configuration.
2. **Comprehensive Unit Testing**: Build a rigorous test suite covering CRUD operations, hash collisions, load factor triggers, dynamic rehashing integrity, and boundary conditions.
3. **Empirical Benchmarking**: Implement an automated benchmark suite measuring throughput (ops/sec) and latency for sequential and random insert, search, update, and delete workloads.
4. **AWS EC2 Live Deployment**: Deploy and run ApexKV on an AWS EC2 Linux instance with automated setup scripts/systemd service configuration so it is live and testable.
5. **Interactive Demo & Portfolio Documentation**: Deliver an interactive CLI demo and a comprehensive, professional `README.md` containing architectural breakdowns, time complexity analysis, benchmark results, deployment steps, and visual demo artifacts.

## Non-Goals (Explicitly Deferred to v2.0)
- Multi-threaded concurrent TCP server & Redis RESP network protocol (v2.0).
- Write-Ahead Log (WAL) & Point-in-Time snapshotting persistence (v2.0).
- LRU caching and memory-bounded eviction policies (v2.0).
- Key expiration / TTL subsystem (v2.0).
- LSM-Tree on-disk storage engine, replication, and distributed clustering (v2.0).

## Users & Target Audience
- **Hiring Managers & Systems Interviewers**: Reviewing clean C++ memory management, algorithmic data structure implementation, benchmarking rigor, and cloud deployment.
- **Backend / Systems Developers**: Using ApexKV as an embedded or standalone in-memory key-value data structure.

## Constraints
- **Language**: Modern C++17 standard with standard library only.
- **Platform**: Linux (primary deployment target on AWS EC2) and Windows (local development environment).
- **Build System**: Clean modular CMake configuration supporting GCC, Clang, and MSVC/MinGW.
- **Memory Safety**: Zero memory leaks (valgrind / AddressSanitizer clean).

## Success Criteria
- [ ] Core `HashTable` engine passes 100% of unit tests with full coverage across edge cases, heavy collisions, and dynamic resizing.
- [ ] CMake build targets (`apexkv_lib`, `apexkv_cli`, `apexkv_tests`, `apexkv_benchmark`) compile cleanly without warnings.
- [ ] Benchmark suite produces reproducible throughput metrics (ops/sec) under 10k, 100k, and 1M key operations.
- [ ] ApexKV is successfully deployed and running on an AWS EC2 Linux server with documented verification steps.
- [ ] Interactive CLI demo showcases real-time inserts, updates, lookups, deletions, and dynamic rehash diagnostics.
- [ ] Comprehensive `README.md` is complete with architecture diagrams, complexity tables, benchmark graphs/tables, and deployment instructions.

## Technical Requirements

| Requirement | Priority | Notes |
|-------------|----------|-------|
| Robust Separate Chaining Hash Table | Must-have | Hardened C++17 memory management & Rule of 5 |
| CMake Build System | Must-have | Modular library, CLI app, tests, and benchmark targets |
| Comprehensive Unit Test Suite | Must-have | Boundary conditions, collisions, rehashing |
| Automated Benchmark Harness | Must-have | Measures ops/sec and latency for CRUD operations |
| Interactive CLI & Demo Mode | Must-have | Terminal interface for live interaction & demo records |
| AWS EC2 Deployment Setup | Must-have | Linux build scripts, systemd/runner configuration |
| Production-Grade README & Docs | Must-have | Architecture breakdown, complexity analysis, benchmark charts |

---

*Last updated: 2026-10-07*
