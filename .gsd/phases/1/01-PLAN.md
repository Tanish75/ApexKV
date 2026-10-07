---
phase: 1
plan: 1
wave: 1
depends_on: []
files_modified:
  - include/hash_table.hpp
  - src/hash_table.cpp
  - tests/test_apexkv.cpp
autonomous: true
gap_closure: false

must_haves:
  truths:
    - "HashTable safely handles copy construction, copy assignment, move construction, and move assignment without leaks or double frees"
    - "HashTable provides const-correct get(), contains(), remove(), put(), loadFactor(), and getSize() methods"
    - "get() cleanly distinguishes between key found with value and key missing"
  artifacts:
    - "include/hash_table.hpp defines complete Rule of 5 and enhanced API"
    - "src/hash_table.cpp implements deep copy, move transfer, and memory cleanup"
---

# Plan 1.1: Engine Hardening & Memory Lifecycle

<objective>
Harden the core C++17 `HashTable` implementation by implementing the Rule of 5 (copy constructor, copy assignment, move constructor, move assignment, destructor), adding const-correctness, robust method return types (`std::optional<int>` / `contains` / `bool remove`), and clean collision chaining memory safety.
</objective>

<context>
Load for context:
- .gsd/SPEC.md
- .gsd/REQUIREMENTS.md
- include/hash_table.hpp
- src/hash_table.cpp
- tests/test_apexkv.cpp
</context>

<tasks>

<task type="auto">
  <name>Implement Rule of 5 and Memory Lifecycle Management</name>
  <files>
    include/hash_table.hpp
    src/hash_table.cpp
  </files>
  <action>
    1. In `include/hash_table.hpp`, declare:
       - Copy constructor: `HashTable(const HashTable& other)`
       - Copy assignment: `HashTable& operator=(const HashTable& other)`
       - Move constructor: `HashTable(HashTable&& other) noexcept`
       - Move assignment: `HashTable& operator=(HashTable&& other) noexcept`
       - Destructor: `~HashTable()`
       - `void clear()` method to deallocate buckets and reset state.
       - Helper method `void copyFrom(const HashTable& other)`.
    2. In `src/hash_table.cpp`, implement deep copying of all buckets and nodes for copy operations.
    3. In `src/hash_table.cpp`, implement zero-copy pointer stealing and nullification for move operations.
    4. Ensure proper self-assignment guards (`if (this == &other) return *this;`) on both copy and move assignment operators.
    
    AVOID: Shallow pointer copies (`table = other.table`) which cause catastrophic double-free bugs.
    USE: Explicit deep copy iteration over `std::forward_list` buckets in `copyFrom`.
  </action>
  <verify>
    g++ -std=c++17 -I include src/hash_table.cpp tests/test_apexkv.cpp -o test_apexkv.exe && ./test_apexkv.exe
  </verify>
  <done>
    HashTable can be copied, moved, assigned, and cleared with zero memory errors or dangling pointers.
  </done>
</task>

<task type="auto">
  <name>Enhance Method Interfaces, Const-Correctness & Return Status</name>
  <files>
    include/hash_table.hpp
    src/hash_table.cpp
    tests/test_apexkv.cpp
  </files>
  <action>
    1. Include `<optional>` in `include/hash_table.hpp`.
    2. Enhance methods:
       - `bool put(const std::string& key, int value)`: Returns `true` if a new key was inserted, `false` if existing key was updated.
       - `std::optional<int> get(const std::string& key) const`: Returns `std::nullopt` when key not found, or `std::make_optional(value)` when found. Also keep backward-compatible overload or helper `int getOrDefault(const std::string& key, int defaultVal) const`.
       - `bool contains(const std::string& key) const`: Returns `true` if key exists in the table.
       - `bool remove(const std::string& key)`: Returns `true` if key was found and removed, `false` if key did not exist.
       - `size_t getCapacity() const`: Returns current bucket count `cap`.
       - `double getLoadFactor() const`: Returns current `loadFactor()`.
       - `void setMaxLoadFactor(double load)`: Allows configuring max load factor threshold (default 0.75).
    3. Ensure key arguments are passed as `const std::string&` across all methods.
    4. Update `tests/test_apexkv.cpp` to validate new signatures and return values.

    AVOID: Passing `std::string` by value in `get()` and `remove()`.
    USE: `const std::string&` to avoid unnecessary string allocations.
  </action>
  <verify>
    g++ -std=c++17 -I include src/hash_table.cpp tests/test_apexkv.cpp -o test_apexkv.exe && ./test_apexkv.exe
  </verify>
  <done>
    All methods compile cleanly, return explicit boolean/optional indicators, and pass updated test validations.
  </done>
</task>

</tasks>

<verification>
After all tasks, verify:
- [ ] Copy constructor and copy assignment create independent deep copies
- [ ] Move constructor and move assignment transfer ownership cleanly without allocating new memory
- [ ] `get()` returns `std::optional<int>` correctly identifying present vs missing keys
- [ ] `remove()` returns `true` on success and `false` on nonexistent key
</verification>

<success_criteria>
- [ ] All tasks verified passing with g++ compiler
- [ ] Zero compile warnings or errors under `-std=c++17 -Wall -Wextra`
- [ ] Backward compatibility with existing tests maintained
</success_criteria>
