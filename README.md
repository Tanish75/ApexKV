# ApexKV

A lightweight, in-memory key-value store written in C++17 using a custom hash table with separate chaining. Built to explore hash table internals, dynamic rehashing, and collision management.

## Quick Example

```cpp
#include "hash_table.hpp"
#include <iostream>

int main() {
    HashTable kv;

    // Insert & Update
    kv.put("server_port", 8080);
    kv.put("max_clients", 100);
    kv.put("server_port", 9000); // updates existing key

    // Lookup (-1 if key does not exist)
    std::cout << "Port: " << kv.get("server_port") << "\n";

    // Delete
    kv.remove("max_clients");

    // Current key count
    std::cout << "Total keys: " << kv.getSize() << "\n";

    return 0;
}
```

## Architecture & Internals

- **Storage & Chaining:** Buckets are backed by an array of `std::forward_list<std::pair<std::string, int>>`. Collisions are resolved using singly-linked list chaining to minimize memory per node.
- **Hashing:** Maps keys using `std::hash<std::string> % capacity`.
- **Dynamic Resizing:** Initial bucket capacity starts at 16 with a maximum load factor threshold of `0.75`. When `size / capacity > 0.75`, the table automatically doubles its bucket count and rehashes all existing entries to maintain $O(1)$ average time complexity.

## Project Structure

```text
ApexKV/
├── include/
│   └── hash_table.hpp      # HashTable class definition
├── src/
│   └── hash_table.cpp      # HashTable implementation
├── tests/
│   └── test_apexkv.cpp     # Unit tests and edge case coverage
├── main.cpp                # Demo application
├── CMakeLists.txt          # CMake build configuration
├── .gitignore
└── README.md
```

## Build & Run

### Direct Compilation (g++)

```bash
# Build & run demo
g++ -std=c++17 -I include src/hash_table.cpp main.cpp -o apexkv.exe
./apexkv.exe

# Build & run test suite
g++ -std=c++17 -I include src/hash_table.cpp tests/test_apexkv.cpp -o test_apexkv.exe
./test_apexkv.exe
```

### Using CMake

```bash
cmake -S . -B build
cmake --build build

# Run tests
./build/test_apexkv.exe
```

## Test Suite

The test runner in `tests/test_apexkv.cpp` verifies:

- **Basic CRUD:** Insertion and lookups for single and multiple keys.
- **Key Updates:** Overwriting existing keys without creating duplicate entries or inflating `size`.
- **Deletions:** Removing keys from the head/middle of a chain, and handling non-existent key deletions gracefully.
- **Rehashing Integrity:** Inserting 50+ elements to trigger dynamic resizing and verifying that all existing data remains intact after table capacity doubles.

## Roadmap

- [x] Separate chaining hash table with `std::forward_list`
- [x] Dynamic load factor tracking and $2\times$ table rehashing
- [x] Delete operation with iterator tracking (`erase_after`)
- [x] Unit test harness and CMake build configuration
- [ ] Generic templates for arbitrary key/value types (`HashTable<K, V>`)
- [ ] Disk persistence (append-only WAL log or snapshots)
- [ ] Simple CLI / REPL interface