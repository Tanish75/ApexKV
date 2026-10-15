# Verification: Phase 4 — Time-To-Live (TTL)

## Verdict: PASS

## Goal-Backward Verification Matrix

| Must-Have Requirement | Empirical Proof / Test | Status |
|------------------------|-------------------------|--------|
| `put` with expiry | `TEST: basic put with TTL and lazy expiration` passed. | PASS |
| Lazy removal on `get`/`contains` | Verified in `test_ttl.cpp` with 1200ms sleep past 1s TTL. Key removed on access. | PASS |
| Expiry survives restart via the log | `TEST: TTL expiration timestamps survive WAL restart` passed across distinct instances. | PASS |
| All TTL tests pass | `build\test_ttl.exe` completed with 4/4 tests passing. | PASS |

## Test Execution Output

```
=== Running ApexKV TTL Tests ===
  TEST: basic put with TTL and lazy expiration... PASS
  TEST: ttl() query and manual expire() command... PASS
  TEST: updating key resets or modifies TTL... PASS
  TEST: TTL expiration timestamps survive WAL restart... PASS

=== TTL Tests: 4/4 PASSED ===
```
