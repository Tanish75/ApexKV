#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include "persistent_kv.hpp"

// ---------------------------------------------------------------------------
// Tiny assert-based test helper
// ---------------------------------------------------------------------------
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
    std::remove((path + ".compact.tmp").c_str());
}

void test_basic_put_ttl_and_expiry() {
    TEST("basic put with TTL and lazy expiration");
    const std::string wal = "test_ttl_basic.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        // Put with 1 second TTL
        kv.put("temp_key", "123", 1);
        kv.put("perm_key", "456", 0); // No TTL

        // Immediate lookup: both should exist
        assert(kv.get("temp_key").value() == "123");
        assert(kv.contains("temp_key") == true);
        assert(kv.get("perm_key").value() == "456");
        assert(kv.size() == 2);

        // Sleep for 1.2 seconds to allow TTL to expire
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        // temp_key must now be lazily removed on get/contains
        assert(!kv.get("temp_key").has_value());
        assert(!kv.contains("temp_key"));
        assert(kv.size() == 1);

        // perm_key remains intact
        assert(kv.get("perm_key").value() == "456");
        assert(kv.contains("perm_key"));
    }

    cleanup(wal);
    PASS();
}

void test_ttl_query_and_manual_expire() {
    TEST("ttl() query and manual expire() command");
    const std::string wal = "test_ttl_query.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("session", "999"); // No TTL initially

        // Initial ttl query: persistent key should return -1
        assert(kv.ttl("session") == -1);

        // Query non-existent key: returns -2
        assert(kv.ttl("non_existent") == -2);

        // Assign 1-second TTL via expire()
        assert(kv.expire("session", 1) == true);
        int64_t rem = kv.ttl("session");
        assert(rem >= 0 && rem <= 1);

        // Setting expire on missing key returns false
        assert(kv.expire("missing", 5) == false);

        // Sleep past expiration
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));

        // After expiration, ttl returns -2 and key is removed
        assert(kv.ttl("session") == -2);
        assert(!kv.get("session").has_value());
    }

    cleanup(wal);
    PASS();
}

void test_ttl_update_and_override() {
    TEST("updating key resets or modifies TTL");
    const std::string wal = "test_ttl_update.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        // Insert with 1s TTL
        kv.put("key", "1", 1);
        assert(kv.ttl("key") >= 0);

        // Update with no TTL: resets key to persistent (no expiry)
        kv.put("key", "2", 0);
        assert(kv.ttl("key") == -1);

        std::this_thread::sleep_for(std::chrono::milliseconds(1100));

        // Must still exist
        assert(kv.get("key").value() == "2");
        assert(kv.size() == 1);
    }

    cleanup(wal);
    PASS();
}

void test_ttl_survives_wal_restart() {
    TEST("TTL expiration timestamps survive WAL restart");
    const std::string wal = "test_ttl_restart.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        // Key 1: Short TTL (1 second)
        kv.put("short_lived", "10", 1);
        // Key 2: Long TTL (10 seconds)
        kv.put("long_lived", "20", 10);
        // Key 3: Persistent
        kv.put("permanent", "30");
        assert(kv.size() == 3);
    }

    // Sleep 1.2 seconds so short_lived expires while store is shut down
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    // Restart store from same WAL file
    {
        PersistentKV kv(wal);

        // short_lived should now be expired upon restart replay and lookup
        assert(!kv.get("short_lived").has_value());
        assert(!kv.contains("short_lived"));

        // long_lived should still be alive with remaining TTL
        assert(kv.get("long_lived").value() == "20");
        int64_t rem = kv.ttl("long_lived");
        assert(rem >= 5 && rem <= 10);

        // permanent should still exist
        assert(kv.get("permanent").value() == "30");
        assert(kv.ttl("permanent") == -1);

        assert(kv.size() == 2);
    }

    cleanup(wal);
    PASS();
}

int main() {
    std::cout << "=== Running ApexKV TTL Tests ===\n";
    test_basic_put_ttl_and_expiry();
    test_ttl_query_and_manual_expire();
    test_ttl_update_and_override();
    test_ttl_survives_wal_restart();

    std::cout << "\n=== TTL Tests: " << tests_passed << "/" << tests_total << " PASSED ===\n";
    return (tests_passed == tests_total) ? 0 : 1;
}
