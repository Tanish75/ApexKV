#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include "persistent_kv.hpp"

// Tiny test helper
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

// Clean up test files
static void cleanup(const std::string& path) {
    std::remove(path.c_str());
    std::remove((path + ".snapshot").c_str());
    std::remove((path + ".snapshot.tmp").c_str());
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_basic_put_get() {
    TEST("basic put and get");
    const std::string wal = "test_basic.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("name", "42");
        kv.put("score", "100");

        assert(kv.get("name").has_value());
        assert(kv.get("name").value() == "42");
        assert(kv.get("score").value() == "100");
        assert(!kv.get("missing").has_value());
        assert(kv.size() == 2);
    }

    cleanup(wal);
    PASS();
}

void test_put_with_int() {
    TEST("put with int overload");
    const std::string wal = "test_int.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("age", 25);
        assert(kv.get("age").value() == "25");
    }

    cleanup(wal);
    PASS();
}

void test_delete() {
    TEST("delete key");
    const std::string wal = "test_del.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("a", "1");
        kv.put("b", "2");
        assert(kv.remove("a") == true);
        assert(kv.remove("a") == false);  // already gone
        assert(!kv.get("a").has_value());
        assert(kv.get("b").value() == "2");
        assert(kv.size() == 1);
    }

    cleanup(wal);
    PASS();
}

void test_update_key() {
    TEST("update existing key");
    const std::string wal = "test_update.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("x", "1");
        kv.put("x", "999");
        assert(kv.get("x").value() == "999");
        assert(kv.size() == 1);
    }

    cleanup(wal);
    PASS();
}

void test_restart_recovery() {
    TEST("restart recovery from WAL");
    const std::string wal = "test_restart.wal";
    cleanup(wal);

    // Write some data and close
    {
        PersistentKV kv(wal);
        kv.put("city", "101");
        kv.put("temp", "37");
        kv.remove("temp");
        kv.put("city", "202");  // update
    }

    // Reopen — should recover the final state
    {
        PersistentKV kv(wal);
        assert(kv.size() == 1);
        assert(kv.get("city").value() == "202");
        assert(!kv.get("temp").has_value());
    }

    cleanup(wal);
    PASS();
}

void test_torn_last_record() {
    TEST("torn (truncated) last record");
    const std::string wal = "test_torn.wal";
    cleanup(wal);

    // Write valid records
    {
        PersistentKV kv(wal);
        kv.put("safe", "1");
        kv.put("also_safe", "2");
    }

    // Append garbage (simulating a torn write)
    {
        std::ofstream out(wal, std::ios::binary | std::ios::app);
        // Write a total_len that promises 100 bytes but only write 5
        uint32_t fake_len = 100;
        out.write(reinterpret_cast<const char*>(&fake_len), 4);
        out.write("XXXXX", 5);
        out.close();
    }

    // Recovery should keep the two good records and discard the torn one
    {
        PersistentKV kv(wal);
        assert(kv.size() == 2);
        assert(kv.get("safe").value() == "1");
        assert(kv.get("also_safe").value() == "2");
    }

    cleanup(wal);
    PASS();
}

void test_bad_checksum() {
    TEST("bad CRC32 checksum");
    const std::string wal = "test_crc.wal";
    cleanup(wal);

    // Write valid records
    {
        PersistentKV kv(wal);
        kv.put("good", "10");
        kv.put("bad_after", "20");
    }

    // Read the WAL, corrupt the CRC of the second record
    {
        std::fstream f(wal, std::ios::binary | std::ios::in | std::ios::out);
        assert(f.is_open());

        // Skip the first record: read its total_len, skip total_len bytes
        uint32_t first_len = 0;
        f.read(reinterpret_cast<char*>(&first_len), 4);
        f.seekg(first_len, std::ios::cur);

        // Now at start of second record. Read its total_len.
        auto second_start = f.tellg();
        uint32_t second_len = 0;
        f.read(reinterpret_cast<char*>(&second_len), 4);

        // The CRC is the last 4 bytes of the record body
        // Seek to the CRC position and flip a byte
        auto crc_pos = static_cast<std::streamoff>(second_start) + 4 +
                       static_cast<std::streamoff>(second_len) - 4;
        f.seekp(crc_pos);
        uint8_t garbage = 0xFF;
        f.write(reinterpret_cast<const char*>(&garbage), 1);
        f.close();
    }

    // Recovery should keep only the first record
    {
        PersistentKV kv(wal);
        assert(kv.size() == 1);
        assert(kv.get("good").value() == "10");
        assert(!kv.get("bad_after").has_value());
    }

    cleanup(wal);
    PASS();
}

void test_snapshot() {
    TEST("snapshot save and existence");
    const std::string wal = "test_snap.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("alpha", "1");
        kv.put("beta", "2");
        assert(kv.save_snapshot() == true);

        // Verify snapshot file exists
        std::ifstream snap(kv.snapshot_path(), std::ios::binary);
        assert(snap.is_open());
        snap.close();
    }

    cleanup(wal);
    PASS();
}

void test_keys_with_spaces_and_newlines() {
    TEST("keys and values with spaces and newlines");
    const std::string wal = "test_special.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("hello world", "42");
        kv.put("line\nbreak", "99");
        assert(kv.get("hello world").value() == "42");
        assert(kv.get("line\nbreak").value() == "99");
    }

    // Verify recovery preserves special chars
    {
        PersistentKV kv(wal);
        assert(kv.get("hello world").value() == "42");
        assert(kv.get("line\nbreak").value() == "99");
    }

    cleanup(wal);
    PASS();
}

void test_empty_wal_recovery() {
    TEST("recovery from empty/nonexistent WAL");
    const std::string wal = "test_empty.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        assert(kv.size() == 0);
        assert(!kv.get("anything").has_value());
    }

    cleanup(wal);
    PASS();
}

void test_many_operations_recovery() {
    TEST("many operations survive restart");
    const std::string wal = "test_many.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        for (int i = 0; i < 200; ++i) {
            kv.put("k" + std::to_string(i), std::to_string(i * 10));
        }
        // Delete even-numbered keys
        for (int i = 0; i < 200; i += 2) {
            kv.remove("k" + std::to_string(i));
        }
    }

    {
        PersistentKV kv(wal);
        assert(kv.size() == 100);
        for (int i = 1; i < 200; i += 2) {
            auto val = kv.get("k" + std::to_string(i));
            assert(val.has_value());
            assert(val.value() == std::to_string(i * 10));
        }
        // Even keys should be gone
        for (int i = 0; i < 200; i += 2) {
            assert(!kv.get("k" + std::to_string(i)).has_value());
        }
    }

    cleanup(wal);
    PASS();
}

// ---------------------------------------------------------------------------

int main() {
    std::cout << "=== WAL Tests ===\n";
    test_basic_put_get();
    test_put_with_int();
    test_delete();
    test_update_key();
    test_restart_recovery();
    test_torn_last_record();
    test_bad_checksum();
    test_snapshot();
    test_keys_with_spaces_and_newlines();
    test_empty_wal_recovery();
    test_many_operations_recovery();

    std::cout << "\n=== WAL Tests: " << tests_passed << "/" << tests_total
              << " PASSED ===\n";

    return (tests_passed == tests_total) ? 0 : 1;
}
