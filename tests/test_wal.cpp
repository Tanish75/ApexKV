#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include "persistent_kv.hpp"

// Tiny test helper
static int tests_passed = 0;
static int tests_total  = 0;

#define TEST(name)                                         \
    do {                                                   \
        tests_total++;                                     \
        std::cout << "  TEST: " << name << "... " << std::flush; \
    } while(0)

#define PASS()                                             \
    do {                                                   \
        tests_passed++;                                    \
        std::cout << "PASS\n";                             \
    } while(0)

// Clean up test files
static void cleanup(const std::string& path) {
    std::remove(path.c_str());
    std::remove((path + ".compact.tmp").c_str());
}

static size_t get_file_size(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open()) return 0;
    return static_cast<size_t>(in.tellg());
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

void test_compaction_file_shrinks_and_restarts() {
    TEST("compaction shrinks file and state is identical after restart");
    const std::string wal = "test_compact_shrink.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        // Put 100 keys
        for (int i = 0; i < 100; ++i) {
            kv.put("k" + std::to_string(i), "init_" + std::to_string(i));
        }
        // Overwrite them 3 times to build up a large WAL log
        for (int round = 0; round < 3; ++round) {
            for (int i = 0; i < 100; ++i) {
                kv.put("k" + std::to_string(i), "v" + std::to_string(round) + "_" + std::to_string(i));
            }
        }
        // Delete 50 keys
        for (int i = 0; i < 50; ++i) {
            kv.remove("k" + std::to_string(i));
        }
        assert(kv.size() == 50);

        size_t before_compact = get_file_size(wal);
        assert(before_compact > 0);

        bool compacted = kv.compact();
        assert(compacted == true);

        size_t after_compact = get_file_size(wal);
        assert(after_compact < before_compact);
        assert(kv.size() == 50);

        for (int i = 50; i < 100; ++i) {
            auto val = kv.get("k" + std::to_string(i));
            assert(val.has_value() && val.value() == "v2_" + std::to_string(i));
        }
    }

    // Verify recovery after restart on compacted WAL
    {
        PersistentKV kv(wal);
        assert(kv.size() == 50);
        for (int i = 0; i < 50; ++i) {
            assert(!kv.contains("k" + std::to_string(i)));
        }
        for (int i = 50; i < 100; ++i) {
            auto val = kv.get("k" + std::to_string(i));
            assert(val.has_value() && val.value() == "v2_" + std::to_string(i));
        }
    }

    cleanup(wal);
    PASS();
}

void test_compaction_drops_expired_keys() {
    TEST("compaction drops expired keys from log and memory");
    const std::string wal = "test_compact_expired.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("live_key", "persistent_val");
        kv.put("short_lived", "to_be_expired", 1); // TTL = 1s

        assert(kv.size() == 2);

        // Sleep 1.2s to expire short_lived
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        bool compacted = kv.compact();
        assert(compacted == true);
        assert(kv.size() == 1);
        assert(kv.get("live_key").value() == "persistent_val");
        assert(!kv.get("short_lived").has_value());
    }

    // After restart, only live_key should be in the store
    {
        PersistentKV kv(wal);
        assert(kv.size() == 1);
        assert(kv.contains("live_key"));
        assert(!kv.contains("short_lived"));
    }

    cleanup(wal);
    PASS();
}

void test_compaction_overwrites_leftover_tmp() {
    TEST("leftover tmp file from crash is overwritten on compact");
    const std::string wal = "test_compact_tmp.wal";
    cleanup(wal);

    // Create a leftover .compact.tmp file simulating crash
    const std::string tmp_file = wal + ".compact.tmp";
    {
        std::ofstream fake_tmp(tmp_file, std::ios::binary);
        fake_tmp << "corrupted remnant from previous crash";
    }
    assert(get_file_size(tmp_file) > 0);

    {
        PersistentKV kv(wal);
        kv.put("data_a", "val_a");
        kv.put("data_b", "val_b");

        bool ok = kv.compact();
        assert(ok == true);
        assert(kv.size() == 2);
        assert(kv.get("data_a").value() == "val_a");
        assert(kv.get("data_b").value() == "val_b");
    }

    // Verify restart works cleanly
    {
        PersistentKV kv(wal);
        assert(kv.size() == 2);
        assert(kv.get("data_a").value() == "val_a");
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

void test_string_values_with_spaces_and_newlines() {
    TEST("string values with spaces and newlines survive restart");
    const std::string wal = "test_str_spaces_newlines.wal";
    cleanup(wal);

    const std::string multi_word = "hello brave new world of c++17";
    const std::string multi_line = "first line\nsecond line with spaces\r\nthird line\ttabbed";

    {
        PersistentKV kv(wal);
        kv.put("k_words", multi_word);
        kv.put("k_lines", multi_line);
        assert(kv.get("k_words").value() == multi_word);
        assert(kv.get("k_lines").value() == multi_line);
    }

    {
        PersistentKV kv(wal);
        assert(kv.size() == 2);
        auto w = kv.get("k_words");
        assert(w.has_value() && w.value() == multi_word);
        auto l = kv.get("k_lines");
        assert(l.has_value() && l.value() == multi_line);
    }

    cleanup(wal);
    PASS();
}

void test_empty_value() {
    TEST("empty string value works and survives restart");
    const std::string wal = "test_empty_val.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        assert(kv.put("empty_key", "") == true);
        assert(kv.contains("empty_key") == true);
        auto res = kv.get("empty_key");
        assert(res.has_value() && res.value().empty());
        assert(kv.size() == 1);
    }

    {
        PersistentKV kv(wal);
        assert(kv.size() == 1);
        assert(kv.contains("empty_key") == true);
        auto res = kv.get("empty_key");
        assert(res.has_value() && res.value() == "");
    }

    cleanup(wal);
    PASS();
}

void test_overwritten_key_keeps_latest_value() {
    TEST("overwritten key keeps latest value after restart");
    const std::string wal = "test_overwrite_latest.wal";
    cleanup(wal);

    {
        PersistentKV kv(wal);
        kv.put("status", "pending");
        kv.put("status", "in_progress");
        kv.put("status", "completed");
        assert(kv.size() == 1);
        assert(kv.get("status").value() == "completed");
    }

    {
        PersistentKV kv(wal);
        assert(kv.size() == 1);
        auto res = kv.get("status");
        assert(res.has_value() && res.value() == "completed");
    }

    cleanup(wal);
    PASS();
}

void test_every_byte_offset_truncation() {
    TEST("every byte offset truncation recovers valid prefix state");
    const std::string seed_wal = "test_trunc_seed.wal";
    cleanup(seed_wal);

    // Create 4 distinct historical records
    {
        PersistentKV kv(seed_wal);
        kv.put("k1", "v1");            // State 1: {k1: v1}
        kv.put("k2", "v2");            // State 2: {k1: v1, k2: v2}
        kv.put("k3", "v3");            // State 3: {k1: v1, k2: v2, k3: v3}
        kv.put("k1", "v1_updated");    // State 4: {k1: v1_updated, k2: v2, k3: v3}
    }

    // Read entire valid log bytes
    std::ifstream in(seed_wal, std::ios::binary);
    std::vector<char> log_bytes((std::istreambuf_iterator<char>(in)),
                                 std::istreambuf_iterator<char>());
    in.close();
    assert(!log_bytes.empty());

    // Define the valid prefix states:
    // Prefix 0: empty
    // Prefix 1: k1=v1
    // Prefix 2: k1=v1, k2=v2
    // Prefix 3: k1=v1, k2=v2, k3=v3
    // Prefix 4: k1=v1_updated, k2=v2, k3=v3
    struct State {
        std::vector<std::pair<std::string, std::string>> pairs;
    };
    std::vector<State> valid_states = {
        {{}},
        {{{"k1", "v1"}}},
        {{{"k1", "v1"}, {"k2", "v2"}}},
        {{{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}},
        {{{"k1", "v1_updated"}, {"k2", "v2"}, {"k3", "v3"}}}
    };

    const std::string test_wal = "test_trunc_offset.wal";

    for (size_t cut = 0; cut <= log_bytes.size(); ++cut) {
        cleanup(test_wal);
        {
            std::ofstream out(test_wal, std::ios::binary | std::ios::trunc);
            if (cut > 0) {
                out.write(log_bytes.data(), static_cast<std::streamsize>(cut));
            }
        }

        PersistentKV recovered(test_wal);

        // Assert that the recovered state matches one of the valid historical prefix states
        bool matches_any_state = false;
        for (const auto& st : valid_states) {
            if (recovered.size() == st.pairs.size()) {
                bool all_match = true;
                for (const auto& [k, v] : st.pairs) {
                    auto val = recovered.get(k);
                    if (!val.has_value() || *val != v) {
                        all_match = false;
                        break;
                    }
                }
                if (all_match) {
                    matches_any_state = true;
                    break;
                }
            }
        }
        assert(matches_any_state);
    }

    cleanup(seed_wal);
    cleanup(test_wal);
    PASS();
}

void test_bit_flip_stops_recovery_before_corrupted_record() {
    TEST("flipping one byte inside each record stops recovery before that record");
    const std::string seed_wal = "test_flip_seed.wal";
    cleanup(seed_wal);

    {
        PersistentKV kv(seed_wal);
        kv.put("rec0", "val0");
        kv.put("rec1", "val1");
        kv.put("rec2", "val2");
        kv.put("rec3", "val3");
    }

    std::ifstream in(seed_wal, std::ios::binary);
    std::vector<uint8_t> log_bytes((std::istreambuf_iterator<char>(in)),
                                   std::istreambuf_iterator<char>());
    in.close();

    // Parse record boundaries from the valid log
    std::vector<size_t> record_offsets;
    size_t cur = 0;
    while (cur + 4 <= log_bytes.size()) {
        uint32_t total_len = 0;
        std::memcpy(&total_len, &log_bytes[cur], 4);
        record_offsets.push_back(cur);
        cur += 4 + total_len;
    }
    assert(record_offsets.size() == 4);

    const std::string test_wal = "test_flip.wal";

    for (size_t r = 0; r < record_offsets.size(); ++r) {
        cleanup(test_wal);

        std::vector<uint8_t> corrupted = log_bytes;
        // Flip a byte inside the payload/crc of record r (e.g. offset + 8)
        size_t flip_pos = record_offsets[r] + 8;
        corrupted[flip_pos] ^= 0xFF;

        {
            std::ofstream out(test_wal, std::ios::binary | std::ios::trunc);
            out.write(reinterpret_cast<const char*>(corrupted.data()),
                      static_cast<std::streamsize>(corrupted.size()));
        }

        PersistentKV kv(test_wal);

        // Recovery MUST halt before record r
        assert(kv.size() == r);
        for (size_t i = 0; i < r; ++i) {
            std::string key = "rec" + std::to_string(i);
            std::string exp = "val" + std::to_string(i);
            auto v = kv.get(key);
            assert(v.has_value() && *v == exp);
        }
        // Records from r onwards must NOT be present
        for (size_t i = r; i < 4; ++i) {
            std::string key = "rec" + std::to_string(i);
            assert(!kv.contains(key));
        }
    }

    cleanup(seed_wal);
    cleanup(test_wal);
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
    test_compaction_file_shrinks_and_restarts();
    test_compaction_drops_expired_keys();
    test_compaction_overwrites_leftover_tmp();
    test_keys_with_spaces_and_newlines();
    test_empty_wal_recovery();
    test_many_operations_recovery();
    test_string_values_with_spaces_and_newlines();
    test_empty_value();
    test_overwritten_key_keeps_latest_value();
    test_every_byte_offset_truncation();
    test_bit_flip_stops_recovery_before_corrupted_record();

    std::cout << "\n=== WAL Tests: " << tests_passed << "/" << tests_total
              << " PASSED ===\n";

    return (tests_passed == tests_total) ? 0 : 1;
}
