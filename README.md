# ApexKV v2

A high-performance, thread-safe, persistent C++17 key-value store with write-ahead logging (WAL), CRC32 data integrity verification, and Time-To-Live (TTL) key expiration.

Zero external dependencies. Plain standard C++17.

---

## Features

- **Header-Only Hash Table Engine (`include/hash_table.hpp`):**
  - Separate chaining backed by `std::vector<std::forward_list<Pair>>`.
  - Zero raw pointers or manual `new[]`/`delete[]`. Full Rule of 5 semantics.
  - In-place mutation and lookup via `find()` pointer accessor (`nullptr` on miss).
  - Automatic dynamic resizing (doubling capacity) at `0.75` load factor boundary.

- **Write-Ahead Logging (WAL v2) & Crash Consistency (`include/persistent_kv.hpp`):**
  - Binary length-prefixed framing: `[total_len: 4B] [op: 1B] [key_len: 4B] [val_len: 4B] [payload] [crc32: 4B]`.
  - Table-based IEEE 802.3 CRC32 checksums on every record.
  - Member `std::ofstream` kept open for high-throughput appends.
  - Automatic crash recovery with auto-truncation: halts replay at first torn or corrupted record and truncates invalid trailing bytes.
  - Atomic snapshot persistence via temp-file write and atomic rename.

- **Time-To-Live (TTL) Key Expiration:**
  - Support for `put(key, value, ttl_seconds)` and manual `expire(key, seconds)`.
  - Lazy deletion on subsequent `get()`, `contains()`, and `ttl()` accesses.
  - Expiry timestamps survive process restarts via binary `PUT_TTL` and `EXPIRE` log records.

- **Thread-Safe Reader-Writer Concurrency:**
  - Embedded coarse `std::shared_mutex` in `PersistentKV`.
  - Parallel non-blocking concurrent readers (`get`, `contains`, `size`) via `std::shared_lock`.
  - Exclusive serialized writers (`put`, `remove`, `expire`, `save_snapshot`) via `std::unique_lock`.

- **Interactive CLI Shell (`main.cpp`):**
  - Interactive prompt supporting `SET`, `GET`, `DEL`, `EXPIRE`, `TTL`, `SIZE`, `HELP`, `EXIT`.

---

## Project Structure

```
ApexKV/
├── include/
│   ├── hash_table.hpp         # Vector-backed separate chaining hash table
│   └── persistent_kv.hpp      # WAL, CRC32, TTL, and reader-writer locking
├── tests/
│   ├── test_apexkv.cpp        # 11 tests: collisions, rehash, copy/move, find()
│   ├── test_wal.cpp           # 11 tests: framing, torn records, CRC corruption
│   ├── test_ttl.cpp           # 4 tests: lazy expiration, TTL query, restart recovery
│   └── test_thread_safety.cpp # 4 tests: concurrent readers/writers/deletes
├── benchmarks/
│   └── benchmark.cpp          # 5-pass benchmark vs std::unordered_map
├── main.cpp                   # Interactive REPL shell
├── CMakeLists.txt             # Unified CMake build file with CTest
├── build.bat                  # Windows build runner
├── build.sh                   # Linux/macOS build runner
└── .github/workflows/ci.yml   # GitHub Actions CI workflow
```

---

## Building & Running

### Windows (MSYS2 / MinGW / GCC)
```cmd
build.bat
```
Run any binary from `build\`:
```cmd
.\build\apexkv.exe               :: Interactive CLI
.\build\test_apexkv.exe          :: Core Hash Table Tests
.\build\test_wal.exe             :: WAL & Crash Recovery Tests
.\build\test_ttl.exe             :: TTL Expiration Tests
.\build\test_thread_safety.exe   :: Concurrency Tests
.\build\benchmark_apexkv.exe     :: Benchmark Suite
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

## Interactive CLI Usage

Launch the CLI:
```
$ ./build/apexkv
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
58 second(s) remaining
apexkv> DEL user:101
(deleted)
apexkv> SIZE
0 key(s)
apexkv> QUIT
Bye.
```

---

## Benchmarks vs `std::unordered_map`

Measured over 50,000 keys across 5 repetitions (`g++ -O2`, Windows 11):

| Operation | ApexKV (avg ops/sec) | `std::unordered_map` (avg ops/sec) | Notes |
|-----------|----------------------|-----------------------------------|-------|
| **PUT**   | ~1,328,000 ops/sec   | ~3,138,000 ops/sec                | Safe forward_list bucket inserts |
| **GET**   | ~9,888,000 ops/sec   | ~77,815,000 ops/sec               | Sequential bucket search |

*ApexKV prioritizes clean 2nd-year CS student readability, zero dependencies, and crash consistency over micro-optimized hash tables.*

---

## Testing & Quality

All 30 unit tests pass 100%:
- `test_apexkv`: 11/11 PASSED
- `test_wal`: 11/11 PASSED
- `test_ttl`: 4/4 PASSED
- `test_thread_safety`: 4/4 PASSED
- Compiler flags: `-Wall -Wextra -Werror` clean pass
- CI: Automated build and test on Ubuntu with AddressSanitizer, UndefinedBehaviorSanitizer, and Valgrind