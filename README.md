# ApexKV

A high-performance, thread-safe, persistent embedded key-value storage engine in modern ISO C++17 featuring Write-Ahead Logging (WAL), IEEE 802.3 CRC32 data integrity verification, Time-To-Live (TTL) key expiration, and log compaction.

Built exclusively with the C++17 Standard Library. Zero external dependencies.

[![CI](https://github.com/Tanish75/ApexKV/actions/workflows/ci.yml/badge.svg)](https://github.com/Tanish75/ApexKV/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/c%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Dependencies](https://img.shields.io/badge/dependencies-0-success.svg)](STDLIB.md)
[![Live Demo](https://img.shields.io/badge/demo-interactive_web-teal.svg)](https://tanish75.github.io/ApexKV/)

---

## Architecture & Features

- **Class-Template Hash Table Engine (`include/hash_table.hpp`):**
  - Separate chaining backed by `std::vector<std::forward_list<std::pair<Key, Value>>>`.
  - Direct pointer accessors via `find()` (`nullptr` on miss) for in-place mutation without redundant lookups.
  - Automatic dynamic resizing (doubling capacity) at a `0.75` load factor boundary.
  - Standard copy and move semantics managed through standard library container RAII.

- **Write-Ahead Logging (WAL) & Crash Consistency (`include/persistent_kv.hpp`):**
  - Binary length-prefixed record framing with table-based IEEE 802.3 CRC32 checksums.
  - Support for arbitrary string keys and values (including spaces, newlines, empty strings, and binary bytes).
  - Open append stream (`std::ofstream`) for sequential write performance.
  - Resilient startup replay: bounds-checks all records, uses 64-bit arithmetic to prevent length overflows, enforces a 64 MB maximum record limit, halts at the first corrupted or torn record, and truncates invalid trailing bytes.

- **Log Compaction (`compact()`):**
  - Under exclusive lock, active non-expired keys are written to a temporary compacted log file.
  - Expired keys and obsolete historical mutations are purged from disk and memory.
  - Atomically replaces the WAL file and reopens the append stream. Any leftover crash temporary files are cleanly overwritten.

- **Time-To-Live (TTL) Expiration:**
  - Support for `put(key, value, ttl_seconds)` and manual `expire(key, seconds)`.
  - Lazy deletion upon access (`get`, `contains`, `ttl`) and active eviction during compaction.
  - Expiry timestamps survive process restarts via binary `PUT_TTL` log records.

- **Reader-Writer Concurrency:**
  - Embedded `std::shared_mutex` in `PersistentKV`.
  - Non-blocking concurrent readers (`get`, `contains`, `size`) via `std::shared_lock`.
  - Serialized writers (`put`, `remove`, `expire`, `compact`) via `std::unique_lock`.

- **Interactive CLI REPL (`main.cpp`):**
  - Shell supporting `SET`, `GET`, `DEL`, `EXPIRE`, `TTL`, `SIZE`, `COMPACT`, `HELP`, and `QUIT`.

---

## WAL Binary Record Format

Every WAL record on disk is binary and length-prefixed:

```
PUT:     [total_len: 4B] [op=1: 1B] [key_len: 4B] [val_len: 4B] [key] [val] [crc32: 4B]
DELETE:  [total_len: 4B] [op=2: 1B] [key_len: 4B] [val_len=0: 4B] [key] [crc32: 4B]
PUT_TTL: [total_len: 4B] [op=3: 1B] [key_len: 4B] [val_len: 4B] [expire_at: 8B] [key] [val] [crc32: 4B]
EXPIRE:  [total_len: 4B] [op=4: 1B] [key_len: 4B] [expire_at: 8B] [key] [crc32: 4B]
```

- `total_len` (uint32_t): Length of the record body including payload and the trailing 4-byte CRC32 checksum. Minimum record length is 13 bytes; maximum is capped at 64 MB.
- `op` (uint8_t): Operation identifier (`1` = PUT, `2` = DELETE, `3` = PUT_TTL, `4` = EXPIRE).
- `crc32` (uint32_t): Standard IEEE 802.3 CRC32 checksum computed over the record payload (from `op` through the end of `val`), initialized via thread-safe function-local static lookup table.

---

## Project Structure

```
ApexKV/
├── include/
│   ├── hash_table.hpp         # Templated separate-chaining hash table
│   └── persistent_kv.hpp      # WAL, CRC32, TTL, compaction, and locking
├── tests/
│   ├── test_apexkv.cpp        # 12 tests: collisions, rehash, find(), copy/move
│   ├── test_wal.cpp           # 18 tests: framing, CRC, truncation, bit-flips, compaction
│   ├── test_ttl.cpp           # 4 tests: lazy expiration, TTL query, restart recovery
│   └── test_thread_safety.cpp # 4 tests: concurrent readers/writers/deletes
├── benchmarks/
│   └── benchmark.cpp          # 5-pass benchmark vs std::unordered_map
├── main.cpp                   # Interactive REPL shell
├── index.html                 # Interactive in-browser visualizer and demo
├── CMakeLists.txt             # Unified CMake build file with CTest
├── build.bat                  # Windows build runner (MSYS2 / MinGW)
├── build.sh                   # Linux/macOS build runner
├── STDLIB.md                  # Standard library zero-dependency mapping document
└── .github/workflows/ci.yml   # GitHub Actions CI workflow (ASan, UBSan, Valgrind)
```

---

## Building & Testing

### Windows (MSYS2 / MinGW / GCC)
```cmd
build.bat
```
Run any compiled binary from `build\`:
```cmd
.\build\apexkv.exe               :: Interactive CLI
.\build\test_apexkv.exe          :: Core Hash Table Tests (12/12)
.\build\test_wal.exe             :: WAL, Compaction & Replay Tests (18/18)
.\build\test_ttl.exe             :: TTL Expiration Tests (4/4)
.\build\test_thread_safety.exe   :: Concurrency Tests (4/4)
.\build\benchmark_apexkv.exe     :: Comparative Benchmark
```

### Linux / macOS
```bash
chmod +x build.sh
./build.sh
```

### CMake (Cross-Platform)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Interactive CLI Session

Real output captured from an interactive execution of `./build/apexkv`:

```
========================================
         ApexKV v2 — Interactive CLI    
========================================
Recovered 0 key(s) from WAL.
Type a command (or HELP):

apexkv> SET user:101 Alice 60
(new) user:101 = Alice (TTL: 60s)
apexkv> GET user:101
user:101 = Alice
apexkv> TTL user:101
60 second(s) remaining
apexkv> COMPACT
(compacted)
apexkv> SIZE
1 key(s)
apexkv> DEL user:101
(deleted)
apexkv> GET user:101
(nil)
apexkv> QUIT
Bye.
```

---

## Concurrency & Durability Model

### Thread-Safety Model
`PersistentKV` embeds a coarse `std::shared_mutex` to coordinate access:
- **Concurrent Readers:** Operations that only read state (`get`, `contains`, `size`) acquire a `std::shared_lock`. Multiple reader threads execute concurrently without blocking each other. Note that if a key has expired, readers escalate to an exclusive lock to perform lazy deletion.
- **Serialized Writers:** Operations that modify state (`put`, `remove`, `expire`, `compact`) acquire a `std::unique_lock`. All write operations are serialized against both readers and other writers.

### Durability Guarantees
- After every write operation (`append_put`, `append_delete`, `append_put_ttl`, `append_expire`), `PersistentKV` calls `std::ofstream::flush()`.
- **Note on `flush()` vs `fsync()`:** `flush()` writes buffer contents from user space into the operating system's kernel page cache. It guarantees data preservation across regular process termination or unexpected application crashes (e.g. `SIGKILL`). However, `flush()` does **not** invoke hardware disk sync (`fsync()` on POSIX or `FlushFileBuffers()` on Windows); uncommitted kernel page cache writes could be lost during a full host operating system crash or physical power loss.

---

## Benchmarks vs `std::unordered_map`

Measured over 50,000 unique string keys across 5 repetitions (`-O2` optimization):

- **Platform:** 13th Gen Intel(R) Core(TM) i5-13420H (8 Cores, 12 Threads)
- **OS:** Windows 11 (64-bit)
- **Compiler:** GCC 16.2.0 (MSYS2 UCRT64)

```
========================================================
                      FINAL RESULTS                     
========================================================
Operation | ApexKV (avg ops/sec) | std::map (avg ops/sec) | Ratio (Apex / std)
----------|----------------------|------------------------|-------------------
PUT       |              3511450 |                7374500 |            0.48x
GET       |             23250359 |               30330042 |            0.77x
========================================================
```

ApexKV delivers ~3.51 million inserts/sec and ~23.25 million lookups/sec in-memory with separate-chaining bucket collision handling, while remaining header-only and dependency-free.

---

## Test Suite Coverage

All 38 automated unit tests pass 100%:

| Test Suite | Tests | Coverage |
|---|:---:|---|
| **`tests/test_apexkv.cpp`** | 12 | Basic inserts/lookups, updates, deletions, size tracking, `find()` pointer mutations, forced single-bucket collisions, chain head/middle/tail removals, exact 0.75 load factor rehash doubling, copy constructor/assignment, move semantics, template instantiations (`<string, int>`, `<string, string>`), and restart recovery. |
| **`tests/test_wal.cpp`** | 18 | Binary length-prefixed framing, PUT/DELETE/PUT_TTL/EXPIRE records, CRC32 checksum mismatch detection, torn record recovery, full byte-by-byte offset truncation sweep, bit-flip corruption isolation, compaction file shrinking, compaction expired key dropping, leftover `.tmp` crash recovery, empty log handling, spaces and newlines preservation, empty string values, and multi-round overwrite recovery. |
| **`tests/test_ttl.cpp`** | 4 | Basic TTL insertion and lazy eviction, `ttl()` inspection (-2, -1, remaining seconds), manual `expire()` assignment, TTL reset/override on updates, and TTL expiry timestamp preservation across process restarts. |
| **`tests/test_thread_safety.cpp`** | 4 | Single-threaded baseline, multi-threaded concurrent writers (4 threads writing 1,000 unique keys without data loss), concurrent interleaved readers and writers, and multi-threaded disjoint delete workers. |

---

## Known Limitations

1. **Memory Bound:** ApexKV is an in-memory hash store backed by a sequential write-ahead log; all active keys and values must fit in RAM.
2. **Coarse-Grained Locking:** Concurrency is protected by a single reader-writer mutex. High-volume concurrent write throughput is serialized by the mutex and the single append file stream.
3. **Values Stored in RAM:** Unlike Bitcask, which stores only key offsets in RAM and streams large values directly from log files on disk, ApexKV stores values in memory for instant lookups.