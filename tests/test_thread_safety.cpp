#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include <cstdio>
#include "persistent_kv.hpp"

static int tests_passed = 0;
static int tests_total  = 0;

#define TEST(name)                                         \
    do {                                                   \
        tests_total++;                                     \
        std::cout << "  TEST: " << name << "... ";         \
    } while(0)

#define PASS()                                             \
    do {                                                   \
        tests_passed++;                                    \
        std::cout << "PASS\n";                             \
    } while(0)

static void cleanup(const std::string& path) {
    std::remove(path.c_str());
    std::remove((path + ".snapshot").c_str());
    std::remove((path + ".snapshot.tmp").c_str());
}

void test_single_thread_basic() {
    TEST("single-thread basic ops");
    const std::string wal = "test_ts_basic.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("a", "1");
        kv.put("b", "2");
        assert(kv.get("a").value() == "1");
        assert(kv.get("b").value() == "2");
        assert(kv.size() == 2);
        kv.remove("a");
        assert(!kv.get("a").has_value());
        assert(kv.size() == 1);
    }

    cleanup(wal);
    PASS();
}

void test_concurrent_writers() {
    TEST("concurrent writers (no data loss)");
    const std::string wal = "test_ts_writers.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        const int num_threads = 4;
        const int ops_per_thread = 250;

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&kv, t, ops_per_thread]() {
                for (int i = 0; i < ops_per_thread; ++i) {
                    std::string key = "t" + std::to_string(t) + "_k" + std::to_string(i);
                    kv.put(key, std::to_string(t * 1000 + i));
                }
            });
        }

        for (auto& th : threads) th.join();

        // All keys from all threads should be present
        assert(kv.size() == static_cast<size_t>(num_threads * ops_per_thread));

        for (int t = 0; t < num_threads; ++t) {
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "t" + std::to_string(t) + "_k" + std::to_string(i);
                assert(kv.contains(key));
            }
        }
    }

    cleanup(wal);
    PASS();
}

void test_concurrent_readers_and_writers() {
    TEST("concurrent readers and writers");
    const std::string wal = "test_ts_rw.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);

        // Pre-populate some data
        for (int i = 0; i < 100; ++i) {
            kv.put("pre_" + std::to_string(i), std::to_string(i));
        }

        std::atomic<int> read_count{0};
        std::atomic<bool> done{false};

        // Writer threads: add more keys
        std::vector<std::thread> writers;
        for (int t = 0; t < 2; ++t) {
            writers.emplace_back([&kv, t]() {
                for (int i = 0; i < 200; ++i) {
                    kv.put("w" + std::to_string(t) + "_" + std::to_string(i),
                           std::to_string(i));
                }
            });
        }

        // Reader threads: continuously read pre-populated keys
        std::vector<std::thread> readers;
        for (int t = 0; t < 4; ++t) {
            readers.emplace_back([&kv, &read_count, &done]() {
                while (!done.load()) {
                    for (int i = 0; i < 100; ++i) {
                        kv.get("pre_" + std::to_string(i));
                        read_count.fetch_add(1);
                    }
                }
            });
        }

        // Wait for writers to finish
        for (auto& th : writers) th.join();
        done.store(true);
        for (auto& th : readers) th.join();

        // Verify all written data is intact
        for (int t = 0; t < 2; ++t) {
            for (int i = 0; i < 200; ++i) {
                std::string key = "w" + std::to_string(t) + "_" + std::to_string(i);
                assert(kv.contains(key));
            }
        }

        std::cout << "(reads=" << read_count.load() << ") ";
    }

    cleanup(wal);
    PASS();
}

void test_concurrent_delete() {
    TEST("concurrent deletes");
    const std::string wal = "test_ts_del.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);

        // Pre-populate
        for (int i = 0; i < 200; ++i) {
            kv.put("d_" + std::to_string(i), std::to_string(i));
        }
        assert(kv.size() == 200);

        // Two threads delete non-overlapping ranges
        std::thread t1([&kv]() {
            for (int i = 0; i < 100; ++i) {
                kv.remove("d_" + std::to_string(i));
            }
        });

        std::thread t2([&kv]() {
            for (int i = 100; i < 200; ++i) {
                kv.remove("d_" + std::to_string(i));
            }
        });

        t1.join();
        t2.join();
        assert(kv.size() == 0);
    }

    cleanup(wal);
    PASS();
}

int main() {
    std::cout << "=== Thread Safety Tests ===\n";
    test_single_thread_basic();
    test_concurrent_writers();
    test_concurrent_readers_and_writers();
    test_concurrent_delete();

    std::cout << "\n=== Thread Safety Tests: " << tests_passed << "/"
              << tests_total << " PASSED ===\n";
    return (tests_passed == tests_total) ? 0 : 1;
}
