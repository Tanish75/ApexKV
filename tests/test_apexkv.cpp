#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include "hash_table.hpp"
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

// ---------------------------------------------------------------------------
// Unit Tests
// ---------------------------------------------------------------------------

void test_insert_and_search() {
    TEST("insert and search");
    HashTable<std::string, int> ht;
    assert(ht.put("a", 10) == true);
    assert(ht.put("b", 20) == true);
    assert(ht.put("c", 30) == true);

    auto resA = ht.get("a");
    assert(resA.has_value() && *resA == 10);

    auto resB = ht.get("b");
    assert(resB.has_value() && *resB == 20);

    auto resC = ht.get("c");
    assert(resC.has_value() && *resC == 30);

    auto resX = ht.get("x");
    assert(!resX.has_value());
    assert(ht.getOrDefault("x", -1) == -1);
    assert(ht.contains("a") == true);
    assert(ht.contains("x") == false);

    PASS();
}

void test_update() {
    TEST("update existing key");
    HashTable<std::string, int> ht;
    assert(ht.put("x", 1) == true);
    assert(ht.put("x", 100) == false);  // update returns false

    auto resX = ht.get("x");
    assert(resX.has_value() && *resX == 100);
    assert(ht.getSize() == 1);

    PASS();
}

void test_remove() {
    TEST("basic remove");
    HashTable<std::string, int> ht;
    ht.put("a", 1);
    ht.put("b", 2);
    ht.put("c", 3);

    assert(ht.remove("b") == true);
    assert(ht.remove("b") == false); // second remove should fail
    assert(!ht.get("b").has_value());
    assert(ht.getOrDefault("a", -1) == 1);
    assert(ht.getOrDefault("c", -1) == 3);
    assert(ht.getSize() == 2);

    PASS();
}

void test_size_and_capacity() {
    TEST("size and capacity tracking");
    HashTable<std::string, int> ht(8, 0.75);
    assert(ht.isEmpty() == true);
    assert(ht.getSize() == 0);
    assert(ht.getCapacity() == 8);

    ht.put("k1", 100);
    ht.put("k2", 200);
    ht.put("k3", 300);
    assert(ht.getSize() == 3);
    assert(!ht.isEmpty());

    ht.remove("k2");
    assert(ht.getSize() == 2);

    ht.clear();
    assert(ht.getSize() == 0);
    assert(ht.isEmpty());
    assert(!ht.get("k1").has_value());

    PASS();
}

void test_find_pointer_accessor() {
    TEST("find() pointer accessor and in-place mutation");
    HashTable<std::string, int> ht;
    ht.put("key1", 42);

    // Non-const pointer lookup
    HashTable<std::string, int>::Pair* p = ht.find("key1");
    assert(p != nullptr);
    assert(p->first == "key1");
    assert(p->second == 42);

    // In-place value modification through pointer
    p->second = 99;
    assert(ht.get("key1").value() == 99);

    // Const pointer lookup
    const HashTable<std::string, int>& const_ht = ht;
    const HashTable<std::string, int>::Pair* cp = const_ht.find("key1");
    assert(cp != nullptr);
    assert(cp->second == 99);

    // Missing key returns nullptr
    assert(ht.find("missing") == nullptr);
    assert(const_ht.find("missing") == nullptr);

    PASS();
}

void test_forced_collisions() {
    TEST("forced collisions in single bucket");
    // Initial capacity 1 with high max load prevents rehash so all elements collide in bucket 0
    HashTable<std::string, int> ht(1, 10.0);
    assert(ht.getCapacity() == 1);

    std::vector<std::string> keys = {"alpha", "beta", "gamma", "delta", "epsilon"};
    for (size_t i = 0; i < keys.size(); ++i) {
        assert(ht.put(keys[i], static_cast<int>(i * 10)) == true);
    }

    assert(ht.getSize() == 5);
    assert(ht.getCapacity() == 1);

    // Verify all keys can still be accessed and verified correctly
    for (size_t i = 0; i < keys.size(); ++i) {
        assert(ht.contains(keys[i]));
        auto val = ht.get(keys[i]);
        assert(val.has_value() && *val == static_cast<int>(i * 10));
    }

    PASS();
}

void test_remove_from_middle_of_chain() {
    TEST("removal from head, middle, and tail of collision chain");
    // Force bucket collision
    HashTable<std::string, int> ht(1, 10.0);
    // Keys pushed front: chain order in bucket is k5 -> k4 -> k3 -> k2 -> k1
    ht.put("k1", 1);
    ht.put("k2", 2);
    ht.put("k3", 3);
    ht.put("k4", 4);
    ht.put("k5", 5);
    assert(ht.getSize() == 5);

    // 1. Delete middle of chain (k3)
    assert(ht.remove("k3") == true);
    assert(!ht.contains("k3"));
    assert(ht.getSize() == 4);
    assert(ht.get("k5").value() == 5);
    assert(ht.get("k4").value() == 4);
    assert(ht.get("k2").value() == 2);
    assert(ht.get("k1").value() == 1);

    // 2. Delete head of chain (k5)
    assert(ht.remove("k5") == true);
    assert(!ht.contains("k5"));
    assert(ht.getSize() == 3);
    assert(ht.get("k4").value() == 4);
    assert(ht.get("k2").value() == 2);
    assert(ht.get("k1").value() == 1);

    // 3. Delete tail of chain (k1)
    assert(ht.remove("k1") == true);
    assert(!ht.contains("k1"));
    assert(ht.getSize() == 2);
    assert(ht.get("k4").value() == 4);
    assert(ht.get("k2").value() == 2);

    PASS();
}

void test_rehash_boundary() {
    TEST("dynamic rehash at exact 0.75 threshold boundary");
    HashTable<std::string, int> ht(4, 0.75); // capacity = 4, threshold = 4 * 0.75 = 3
    assert(ht.getCapacity() == 4);

    ht.put("item1", 1);
    assert(ht.getCapacity() == 4);
    assert(ht.getLoadFactor() == 0.25);

    ht.put("item2", 2);
    assert(ht.getCapacity() == 4);
    assert(ht.getLoadFactor() == 0.50);

    ht.put("item3", 3);
    assert(ht.getCapacity() == 4);
    assert(ht.getLoadFactor() == 0.75);

    // 4th insert exceeds 0.75 load factor threshold -> triggers rehash doubling to 8
    ht.put("item4", 4);
    assert(ht.getSize() == 4);
    assert(ht.getCapacity() == 8);
    assert(ht.getLoadFactor() == 0.50);

    // Verify all 4 items remain retrievable after rehash
    assert(ht.get("item1").value() == 1);
    assert(ht.get("item2").value() == 2);
    assert(ht.get("item3").value() == 3);
    assert(ht.get("item4").value() == 4);

    PASS();
}

void test_copy_semantics() {
    TEST("copy constructor and assignment (deep copy)");
    HashTable<std::string, int> original;
    original.put("alpha", 1);
    original.put("beta", 2);
    original.put("gamma", 3);

    // Copy Constructor
    HashTable<std::string, int> copied(original);
    assert(copied.getSize() == 3);
    assert(copied.getOrDefault("alpha", 0) == 1);
    assert(copied.getOrDefault("beta", 0) == 2);

    // Modify copy, ensure original is unmodified
    copied.put("alpha", 999);
    copied.put("delta", 4);
    assert(original.getOrDefault("alpha", 0) == 1);
    assert(copied.getOrDefault("alpha", 0) == 999);
    assert(!original.contains("delta"));
    assert(copied.contains("delta"));

    // Copy Assignment
    HashTable<std::string, int> assigned;
    assigned = original;
    assert(assigned.getSize() == 3);
    assert(assigned.getOrDefault("gamma", 0) == 3);

    PASS();
}

void test_move_semantics() {
    TEST("move constructor and assignment");
    HashTable<std::string, int> source;
    source.put("m1", 10);
    source.put("m2", 20);

    // Move Constructor
    HashTable<std::string, int> moved(std::move(source));
    assert(moved.getSize() == 2);
    assert(moved.getOrDefault("m1", 0) == 10);
    assert(moved.getOrDefault("m2", 0) == 20);

    // Move Assignment
    HashTable<std::string, int> target;
    target.put("temp", 99);
    target = std::move(moved);
    assert(target.getSize() == 2);
    assert(target.getOrDefault("m1", 0) == 10);
    assert(target.getOrDefault("m2", 0) == 20);

    PASS();
}

void test_string_template_instantiation() {
    TEST("string key and string value template instantiation");
    HashTable<std::string, std::string> ht;
    assert(ht.put("greeting", "hello world") == true);
    assert(ht.put("farewell", "goodbye\nworld") == true);
    assert(ht.put("empty", "") == true);

    assert(ht.get("greeting").value() == "hello world");
    assert(ht.get("farewell").value() == "goodbye\nworld");
    assert(ht.get("empty").value() == "");
    assert(ht.getSize() == 3);

    PASS();
}

void test_restart_recovery() {
    TEST("restart recovery with PersistentKV WAL");
    const std::string wal_file = "test_phase2_restart.wal";
    std::remove(wal_file.c_str());
    std::remove((wal_file + ".snapshot").c_str());

    {
        PersistentKV kv(wal_file);
        kv.put("user:101", "100");
        kv.put("user:102", "200");
        kv.put("user:103", "300");
        kv.remove("user:102");
        kv.put("user:101", "150"); // update
        assert(kv.size() == 2);
    }

    // Simulate restart by opening a new instance from the existing WAL
    {
        PersistentKV kv(wal_file);
        assert(kv.size() == 2);
        assert(kv.get("user:101").value() == "150");
        assert(!kv.contains("user:102"));
        assert(kv.get("user:103").value() == "300");
    }

    std::remove(wal_file.c_str());
    std::remove((wal_file + ".snapshot").c_str());

    PASS();
}

int main() {
    std::cout << "=== ApexKV HashTable Unit Tests ===\n";
    test_insert_and_search();
    test_update();
    test_remove();
    test_size_and_capacity();
    test_find_pointer_accessor();
    test_forced_collisions();
    test_remove_from_middle_of_chain();
    test_rehash_boundary();
    test_copy_semantics();
    test_move_semantics();
    test_string_template_instantiation();
    test_restart_recovery();

    std::cout << "\n=== Test Results: " << tests_passed << "/" << tests_total << " PASSED ===\n";
    return (tests_passed == tests_total) ? 0 : 1;
}