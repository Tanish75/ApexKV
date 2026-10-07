---
milestone: v1.0
version: 1.0.0
updated: 2026-10-07
---

# Roadmap

> **Current Phase:** Not started
> **Status:** planning
> **Milestone:** v1.0 - Polish, Testing, Benchmarks, Deployment & Documentation

## Must-Haves (from SPEC)

- [ ] Hardened C++17 `HashTable` engine with clean memory management & Rule of 5 semantics
- [ ] Modular CMake build configuration for library, CLI application, tests, and benchmarks
- [ ] Comprehensive unit test suite with 100% test pass rate across CRUD, collisions, and rehashing
- [ ] Automated benchmark suite measuring throughput (ops/sec) and latency
- [ ] Interactive CLI demo mode for live testing and visual demonstrations
- [ ] AWS EC2 deployment automation and live setup verification
- [ ] Comprehensive production-grade `README.md` with architecture, benchmark metrics, and setup guide

---

## Phases

### Phase 1: Engine Hardening & CMake Build Infrastructure
**Status:** ⬜ Not Started
**Objective:** Finalize the core C++17 `HashTable` implementation (memory safety, copy/move constructors, robust return types/edge cases) and construct a complete CMake build configuration (`apexkv_lib`, `apexkv_cli`, `apexkv_tests`, `apexkv_bench`).
**Requirements:** REQ-01, REQ-02, NFR-01, NFR-02, NFR-03, NFR-04

**Plans:**
- [ ] Plan 1.1: Engine Hardening & Memory Lifecycle
- [ ] Plan 1.2: CMake Build Infrastructure

---

### Phase 2: Comprehensive Unit Testing & Validation
**Status:** ⬜ Not Started
**Objective:** Build an exhaustive unit testing framework covering CRUD operations, hash collisions, dynamic rehashing accuracy, boundary values, empty keys, and memory lifecycle checks.
**Depends on:** Phase 1
**Requirements:** REQ-03, NFR-02

**Plans:**
- [ ] Plan 2.1: Expand unit test suite in `tests/test_apexkv.cpp` with structured test cases and assertions
- [ ] Plan 2.2: Add collision stress testing, rehash boundary assertions, and memory integrity validation

---

### Phase 3: Benchmarking Suite & Performance Analysis
**Status:** ⬜ Not Started
**Objective:** Build an automated benchmark executable (`benchmarks/benchmark.cpp`) to measure insert, lookup, update, and deletion throughput (ops/second) and latency percentiles across varying key counts (10k, 100k, 1M).
**Depends on:** Phase 2
**Requirements:** REQ-04, NFR-01

**Plans:**
- [ ] Plan 3.1: Implement high-resolution benchmark runner measuring operations/sec for sequential and random workloads
- [ ] Plan 3.2: Run benchmark suite, record performance baselines, and analyze load factor impact

---

### Phase 4: Interactive CLI, Demo Experience & AWS EC2 Deployment
**Status:** ⬜ Not Started
**Objective:** Create an interactive REPL/CLI for ApexKV with formatted outputs, build an automated deployment script for AWS EC2 Linux instances, and prepare demo visual artifacts.
**Depends on:** Phase 3
**Requirements:** REQ-05, REQ-06

**Plans:**
- [ ] Plan 4.1: Build an interactive CLI (`main.cpp` / REPL) supporting `PUT`, `GET`, `REMOVE`, `STATS`, `DEMO`, `HELP`, `EXIT`
- [ ] Plan 4.2: Create AWS EC2 deployment automation scripts (`deploy_ec2.sh`, systemd unit / runner) and deployment instructions

---

### Phase 5: Production Documentation & Milestone Finalization
**Status:** ⬜ Not Started
**Objective:** Write a comprehensive, industry-standard `README.md` featuring architecture overview, complexity analysis, benchmark results, EC2 deployment guide, demo recordings/screenshots, and prepare roadmap for v2.0.
**Depends on:** Phase 4
**Requirements:** REQ-07, NFR-05

**Plans:**
- [ ] Plan 5.1: Create production-grade `README.md` with ASCII/Mermaid architecture diagrams, benchmark tables, and quickstart guide
- [ ] Plan 5.2: Final milestone audit, verification, and tag v1.0 release

---

## Progress Summary

| Phase | Status | Plans | Complete |
|-------|--------|-------|----------|
| 1. Engine Hardening & CMake | ⬜ Not Started | 0/2 | 0% |
| 2. Comprehensive Unit Testing | ⬜ Not Started | 0/2 | 0% |
| 3. Benchmarking Suite | ⬜ Not Started | 0/2 | 0% |
| 4. CLI, Demo & AWS EC2 Deployment | ⬜ Not Started | 0/2 | 0% |
| 5. Documentation & Release | ⬜ Not Started | 0/2 | 0% |

---

## Timeline

| Phase | Started | Completed | Duration |
|-------|---------|-----------|----------|
| 1 | — | — | — |
| 2 | — | — | — |
| 3 | — | — | — |
| 4 | — | — | — |
| 5 | — | — | — |

---

## Status Icons

- ⬜ Not Started
- 🔄 In Progress
- ✅ Complete
- ⏸️ Paused
- ❌ Blocked
