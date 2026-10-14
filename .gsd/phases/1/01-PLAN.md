---
phase: 1
plan: 1
wave: 1
depends_on: []
files_modified:
  - include/hash_table.hpp
  - src/hash_table.cpp
  - gitignore
  - .gitignore
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "HashTable is header-defined so apexkv and test_apexkv build directly from include/hash_table.hpp without src/hash_table.cpp"
    - "No compiled binaries, logs, or snapshot files are tracked in git"
    - ".gitignore is properly named with leading dot and ignores *.exe, *.log, *.snapshot, build/"
  artifacts:
    - "include/hash_table.hpp contains complete inline/header implementation"
    - ".gitignore exists with required ignore patterns"
---

# Plan 1.1: Repository Hygiene & Stale Source Cleanup

## Objective
Clean up legacy repository artifacts by inlining `HashTable` implementation into `include/hash_table.hpp`, removing the stale `src/hash_table.cpp`, removing committed `.exe` binaries from git tracking, and renaming `gitignore` to `.gitignore` with updated exclusion rules.

## Context
- `prd.md`
- `.gsd/SPEC.md`
- `.gsd/ARCHITECTURE.md`
- `include/hash_table.hpp`
- `src/hash_table.cpp`
- `gitignore`

## Tasks

<task type="auto">
  <name>Consolidate HashTable into include/hash_table.hpp and delete stale src/hash_table.cpp</name>
  <files>
    include/hash_table.hpp
    src/hash_table.cpp
  </files>
  <action>
    1. Move method implementations from `src/hash_table.cpp` into `include/hash_table.hpp` (inline member functions) so that `HashTable` is self-contained.
    2. Maintain existing behavior, Rule of 5 semantics, and member names (`table`, `cap`, `sz`, `maxLoad`).
    3. Delete `src/hash_table.cpp` using git or filesystem removal.
    4. Verify that `main.cpp` and `tests/test_apexkv.cpp` compile directly against `include/` without needing `src/hash_table.cpp`.

    AVOID: Breaking existing method signatures or introducing new dependencies.
    USE: Plain standard C++17 inline methods inside `include/hash_table.hpp`.
  </action>
  <verify>
    g++ -std=c++17 -Wall -Wextra -I include tests/test_apexkv.cpp -o test_apexkv.exe && ./test_apexkv.exe
  </verify>
  <done>
    `src/hash_table.cpp` is removed, and `test_apexkv.cpp` compiles cleanly and passes all tests using only `include/hash_table.hpp`.
  </done>
</task>

<task type="auto">
  <name>Remove committed binaries and rename gitignore to .gitignore</name>
  <files>
    gitignore
    .gitignore
  </files>
  <action>
    1. Untrack committed executable binaries: `apexkv.exe` and `test_apexkv.exe` using `git rm --cached apexkv.exe test_apexkv.exe`.
    2. Untrack any committed `.log` or `.snapshot` files if present.
    3. Rename `gitignore` to `.gitignore`.
    4. Ensure `.gitignore` explicitly includes:
       - `*.exe`
       - `*.o`
       - `*.a`
       - `*.log`
       - `*.snapshot`
       - `build/`
       - `.vscode/`
       - `.DS_Store`
       - `Thumbs.db`
    5. Check `git status` to verify `.exe` files and build outputs are ignored and untracked.

    AVOID: Leaving binaries or un-dotted `gitignore` in source control.
  </action>
  <verify>
    git status --ignored
  </verify>
  <done>
    `git ls-files` shows no `.exe`, `.log`, or `.snapshot` files, and `.gitignore` correctly ignores build artifacts.
  </done>
</task>

## Success Criteria
- [ ] `src/hash_table.cpp` is deleted
- [ ] `include/hash_table.hpp` provides complete self-contained `HashTable`
- [ ] `test_apexkv.cpp` compiles and passes all unit tests
- [ ] `.gitignore` is named correctly and active
- [ ] Zero `.exe`, `.log`, or `.snapshot` files tracked in git
