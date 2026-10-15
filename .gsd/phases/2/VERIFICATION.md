# Verification: Phase 2 — HashTable and Tests

## Verdict: PASS

## Goal-Backward Verification Matrix

| Must-Have Requirement | Empirical Proof / Test | Status |
|------------------------|-------------------------|--------|
| Replace raw `new[]`/`delete[]` with `std::vector<std::forward_list<Pair>>` | Inspected `include/hash_table.hpp` lines 18-35; memory lifecycle managed by standard containers. Zero raw `new`/`delete` in header. | PASS |
| `find()` returning `Pair*` (`nullptr` if missing) | `TEST: find() pointer accessor and in-place mutation` passed in `tests/test_apexkv.cpp`. | PASS |
| Tiny assert-based test helper | Implemented `TEST(name)` and `PASS()` macros tracking total (11) and passed (11). | PASS |
| Forced collisions test | `TEST: forced collisions in single bucket` passed with 5 colliding keys in capacity 1 table. | PASS |
| Middle-of-chain removal test | `TEST: removal from head, middle, and tail of collision chain` passed with head, middle, and tail removals. | PASS |
| Rehash at 0.75 boundary test | `TEST: dynamic rehash at exact 0.75 threshold boundary` passed with initial capacity 4, capacity jump to 8 on 4th element. | PASS |
| Restart recovery test | `TEST: restart recovery with PersistentKV WAL` passed with multiple mutations surviving store reload. | PASS |
| Clean compilation under `-Wall -Wextra` | `build.bat` compiles `apexkv`, `test_apexkv`, and `benchmark_apexkv` with zero warnings. | PASS |

## Test Execution Output

```
=== Running ApexKV Phase 2 Tests ===
  TEST: insert and search... PASS
  TEST: update existing key... PASS
  TEST: basic remove... PASS
  TEST: size and capacity tracking... PASS
  TEST: find() pointer accessor and in-place mutation... PASS
  TEST: forced collisions in single bucket... PASS
  TEST: removal from head, middle, and tail of collision chain... PASS
  TEST: dynamic rehash at exact 0.75 threshold boundary... PASS
  TEST: copy constructor and assignment (deep copy)... PASS
  TEST: move constructor and assignment... PASS
  TEST: restart recovery with PersistentKV WAL... PASS

=== Test Results: 11/11 PASSED ===
```
