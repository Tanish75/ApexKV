# Plan 3.1 Summary: Write-Ahead Logging (WAL v2) & Crash Consistency

## Execution Details
- **Phase:** 3 (WAL v2)
- **Status:** Complete
- **Commits:**
  - `77300c8 feat: add binary WAL with CRC32 checksums and recovery tests`

## What Was Done
1. **Binary Length-Prefixed WAL:**
   - Designed binary frame format: `[total_len: 4B] [op: 1B] [key_len: 4B] [val_len: 4B] [key] [val] [crc32: 4B]`.
   - Implemented table-based IEEE 802.3 CRC32 checksum calculation.
   - Maintained open `std::ofstream wal_stream_` member for high-throughput appends without reopening per write.
   - Handled arbitrary characters (spaces, newlines, binary data) cleanly.
2. **Crash Consistency & Auto-Truncation:**
   - Implemented replay mechanism on initialization that verifies total length, frame bounds, and CRC32 checksum.
   - Automatically halts replay on first torn or corrupted record and truncates the WAL to the last known-good byte boundary.
3. **Atomic Snapshotting:**
   - Implemented temporary file write (`snapshot.tmp`) followed by atomic rename to guarantee consistency during concurrent snapshotting.
4. **Unit Test Suite (`tests/test_wal.cpp`):**
   - 11 comprehensive tests validating basic CRUD, int overload, restart recovery, torn record truncation, corrupted checksum rejection, snapshot rename, and complex binary strings.

## Verification
- Ran `.\build\test_wal.exe`:
  ```
  === WAL Tests ===
    TEST: basic put and get... PASS
    TEST: put with int overload... PASS
    TEST: delete key... PASS
    TEST: update existing key... PASS
    TEST: restart recovery from WAL... PASS
    TEST: torn (truncated) last record... PASS
    TEST: bad CRC32 checksum... PASS
    TEST: snapshot save and existence... PASS
    TEST: keys and values with spaces and newlines... PASS
    TEST: recovery from empty/nonexistent WAL... PASS
    TEST: many operations survive restart... PASS

  === WAL Tests: 11/11 PASSED ===
  ```
