#include <iostream>
#include <cassert>
#include <string>
#include "hash_table.hpp"

void test_insert_and_search() {
    HashTable ht;
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

    std::cout << "PASS: insert and search\n";
}

void test_update() {
    HashTable ht;
    assert(ht.put("x", 1) == true);
    assert(ht.put("x", 100) == false);  // update returns false

    auto resX = ht.get("x");
    assert(resX.has_value() && *resX == 100);
    assert(ht.getSize() == 1);

    std::cout << "PASS: update\n";
}

void test_remove() {
    HashTable ht;
    ht.put("a", 1);
    ht.put("b", 2);
    ht.put("c", 3);

    assert(ht.remove("b") == true);
    assert(ht.remove("b") == false); // second remove should fail
    assert(!ht.get("b").has_value());
    assert(ht.getOrDefault("a", -1) == 1);
    assert(ht.getOrDefault("c", -1) == 3);

    std::cout << "PASS: remove\n";
}

void test_size_and_capacity() {
    HashTable ht(8, 0.75);
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

    std::cout << "PASS: size and capacity\n";
}

void test_copy_semantics() {
    HashTable original;
    original.put("alpha", 1);
    original.put("beta", 2);
    original.put("gamma", 3);

    // Copy Constructor
    HashTable copied(original);
    assert(copied.getSize() == 3);
    assert(copied.getOrDefault("alpha", 0) == 1);
    assert(copied.getOrDefault("beta", 0) == 2);

    // Modify copy, ensure original is unmodified (deep copy check)
    copied.put("alpha", 999);
    copied.put("delta", 4);
    assert(original.getOrDefault("alpha", 0) == 1);
    assert(copied.getOrDefault("alpha", 0) == 999);
    assert(!original.contains("delta"));
    assert(copied.contains("delta"));

    // Copy Assignment
    HashTable assigned;
    assigned = original;
    assert(assigned.getSize() == 3);
    assert(assigned.getOrDefault("gamma", 0) == 3);

    // Self-assignment check
    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wself-assign-overload"
    #endif
    assigned = assigned;
    #if defined(__clang__)
    #pragma clang diagnostic pop
    #endif
    assert(assigned.getSize() == 3);

    std::cout << "PASS: copy semantics\n";
}

void test_move_semantics() {
    HashTable source;
    source.put("m1", 10);
    source.put("m2", 20);

    // Move Constructor
    HashTable moved(std::move(source));
    assert(moved.getSize() == 2);
    assert(moved.getOrDefault("m1", 0) == 10);
    assert(moved.getOrDefault("m2", 0) == 20);
    assert(source.getSize() == 0);

    // Move Assignment
    HashTable target;
    target.put("temp", 99);
    target = std::move(moved);
    assert(target.getSize() == 2);
    assert(target.getOrDefault("m1", 0) == 10);
    assert(moved.getSize() == 0);

    std::cout << "PASS: move semantics\n";
}

void test_rehash() {
    HashTable ht(4, 0.75); // initial cap 4, threshold is > 3 elements
    assert(ht.getCapacity() == 4);

    for (int i = 0; i < 20; ++i) {
        ht.put("key_" + std::to_string(i), i * 10);
    }

    assert(ht.getSize() == 20);
    assert(ht.getCapacity() >= 32);
    assert(ht.getLoadFactor() <= ht.getMaxLoadFactor());

    for (int i = 0; i < 20; ++i) {
        auto val = ht.get("key_" + std::to_string(i));
        assert(val.has_value() && *val == i * 10);
    }

    std::cout << "PASS: dynamic rehash\n";
}

int main() {
    std::cout << "=== Running ApexKV Phase 1 Tests ===\n";
    test_insert_and_search();
    test_update();
    test_remove();
    test_size_and_capacity();
    test_copy_semantics();
    test_move_semantics();
    test_rehash();
    std::cout << "=== ALL PHASE 1 TESTS PASSED ===\n";
    return 0;
}