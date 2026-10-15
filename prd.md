# ApexKV v2: harden and extend my existing C++17 key-value store

Existing code: include/hash_table.hpp, include/persistent_kv.hpp, tests/, main.cpp, CMakeLists.txt, README.md. Follow my existing style: snake_case, trailing underscore on members, plain C++17, standard library only, zero dependencies.

Hard rules for every phase:
- Keep code simple and readable, as a 2nd-year CS student would write it. No template metaprogramming, no over-abstraction. Short comments only where non-obvious.
- I must be able to explain every line in an interview.
- Never invent benchmark numbers or claim unimplemented features.
- One small atomic commit per task, plain commit messages. Never alter commit dates.

Phases:
1. Housekeeping and build: delete stale src/hash_table.cpp; remove committed .exe/.log/.snapshot files; rename gitignore to .gitignore (ignore *.exe, *.log, *.snapshot, build/); fix CMakeLists.txt to build apexkv, test_apexkv, benchmark_apexkv with -Wall -Wextra and register tests with ctest.
2. HashTable and tests: replace raw new[]/delete[] with std::vector<std::forward_list<Pair>>; add find() returning pointer (nullptr if missing). Make tests/test_apexkv.cpp compile (fix PersistentKV constructor calls) and use a tiny assert-based helper. Add tests: forced collisions, rehash at 0.75 boundary, update existing key, remove from middle of chain, restart recovery.
3. WAL v2: binary length-prefixed records (op, key len, value len, key, value, CRC32); keep the stream open as a member; support keys/values with spaces and newlines; on startup stop at first corrupt/truncated record and truncate the log; write snapshots to a temp file then rename. Tests for a torn last record and a bad checksum.
4. TTL: put with expiry; lazy removal on get/contains; expiry survives restart via the log. Tests.
5. Thread safety and CLI: coarse std::shared_mutex in PersistentKV (shared for reads, exclusive for writes) plus a multi-threaded test. Turn main.cpp into a small CLI: SET, GET, DEL, EXPIRE.
6. Quality and docs: benchmark with pre-generated keys, several repetitions, ops/sec vs std::unordered_map (report honestly). Run tests under -fsanitize=address,undefined and valgrind and fix issues. Add a GitHub Actions workflow to build and run tests. Update README to document only what exists. Write RESUME_NOTES.md (not committed) listing only verified facts: RAII/smart pointers actually used, sanitizer/valgrind results, test count and coverage, WAL features, TTL, thread safety, measured benchmark numbers.
