# Plan 4.1 Summary: Time-To-Live (TTL) Key Expiration Subsystem

## Execution Details
- **Phase:** 4 (TTL)
- **Status:** Complete

## What Was Done
1. **TTL Key Expiration Mechanics:**
   - Added `WalOp::PUT_TTL` (op=3) and `WalOp::EXPIRE` (op=4) to WAL framing protocol.
   - Added `std::unordered_map<std::string, int64_t> expiries_` storing epoch expiration timestamps in seconds.
   - Implemented `put(key, value, ttl_seconds)` overloads for string and integer values.
   - Implemented `expire(key, ttl_seconds)` for attaching expiration to existing keys.
   - Implemented `ttl(key)` query returning remaining seconds, -1 (no expiry), or -2 (not found/expired).
   - Implemented lazy removal on `get()` and `contains()`: expired keys are purged from both memory and WAL log on first subsequent access.
   - Implemented crash persistence: TTL timestamps survive store reboots and replay accurately.
2. **Comprehensive Unit Tests (`tests/test_ttl.cpp`):**
   - Tested basic TTL expiration with sleep intervals.
   - Tested `ttl()` query return codes and manual `expire()` calls.
   - Tested update overriding and resetting TTL.
   - Tested restart recovery ensuring active TTLs survive while past expiries are lazily purged.

## Verification
- Ran `.\build\test_ttl.exe`:
  ```
  === Running ApexKV TTL Tests ===
    TEST: basic put with TTL and lazy expiration... PASS
    TEST: ttl() query and manual expire() command... PASS
    TEST: updating key resets or modifies TTL... PASS
    TEST: TTL expiration timestamps survive WAL restart... PASS

  === TTL Tests: 4/4 PASSED ===
  ```
