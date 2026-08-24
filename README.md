# ApexKV

Concurrent Key-Value Storage Engine in C++

## Features

- Hash table with chaining
- O(1) average-case operations
- Dynamic resizing (coming soon)
- Thread-safe (coming soon)
- Persistence with WAL (coming soon)

## Build

```bash
g++ -o apexkv main.cpp src/hash_table.cpp
./apexkv
```

## Progress

- [x] Day 1: Basic hash table
- [ ] Day 2: Dynamic resizing
- [ ] Day 3: Unit tests
- [ ] Day 4: Concurrency
- [ ] Day 5: WAL
- [ ] Day 6: TTL
- [ ] Day 7: Benchmarks
