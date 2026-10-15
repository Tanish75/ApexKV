---
phase: 6
plan: 1
wave: 1
depends_on: []
files_modified:
  - benchmarks/benchmark.cpp
  - .github/workflows/ci.yml
  - README.md
  - .gitignore
  - RESUME_NOTES.md
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "Automated comparative benchmark against std::unordered_map reports honest ops/sec across 5 repetitions"
    - "GitHub Actions CI workflow configures CTest, AddressSanitizer, and Valgrind"
    - "README.md documents only what actually exists in ApexKV v2"
    - "RESUME_NOTES.md lists verified engineering facts and remains uncommitted"
  artifacts:
    - "benchmarks/benchmark.cpp"
    - ".github/workflows/ci.yml"
    - "README.md"
    - "RESUME_NOTES.md"
---

# Plan 6.1: Quality, CI, Benchmarking & Documentation

## Objective
Implement comparative benchmarking against `std::unordered_map`, configure GitHub Actions continuous integration with AddressSanitizer and Valgrind, rewrite `README.md` to document only verified existing features, and create uncommitted `RESUME_NOTES.md` for technical interview preparation.

## Tasks

<task type="auto">
  <name>Implement comparative benchmark runner</name>
  <files>
    benchmarks/benchmark.cpp
  </files>
  <action>
    1. Update `benchmarks/benchmark.cpp` with 50,000 pre-generated unique keys.
    2. Run 5 repetitions comparing ApexKV vs `std::unordered_map` for PUT and GET ops/sec.
    3. Calculate and display average throughput and performance ratios honestly.
  </action>
  <verify>
    .\build\benchmark_apexkv.exe
  </verify>
  <done>
    Benchmark runner executes and reports honest performance comparisons.
  </done>
</task>

<task type="auto">
  <name>Configure GitHub Actions CI and production documentation</name>
  <files>
    .github/workflows/ci.yml
    README.md
    .gitignore
    RESUME_NOTES.md
  </files>
  <action>
    1. Create `.github/workflows/ci.yml` running CTest, ASan, and Valgrind on Ubuntu.
    2. Overwrite `README.md` with complete, accurate documentation of ApexKV v2.
    3. Update `.gitignore` with `RESUME_NOTES.md` and `*.wal`.
    4. Write `RESUME_NOTES.md` summarizing verified architecture facts, test metrics, and performance.
  </action>
  <verify>
    git status
  </verify>
  <done>
    All documentation is up to date, CI workflow is created, and RESUME_NOTES.md is preserved uncommitted.
  </done>
</task>
