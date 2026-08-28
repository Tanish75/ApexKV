#include <iostream>
#include <string>
#include "hash_table.hpp"

void test_put_and_get() {
    HashTable ht;

    ht.put("name", 10);
    ht.put("age", 25);
    ht.put("city", 50);

    if (ht.get("name") != 10) {
        std::cout << "FAIL: get name\n";
        return;
    }

    if (ht.get("age") != 25) {
        std::cout << "FAIL: get age\n";
        return;
    }

    if (ht.get("city") != 50) {
        std::cout << "FAIL: get city\n";
        return;
    }

    if (ht.get("missing") != -1) {
        std::cout << "FAIL: missing key should return -1\n";
        return;
    }

    std::cout << "PASS: put and get\n";
}

void test_update() {
    HashTable ht;

    ht.put("name", 10);
    ht.put("name", 20);
    ht.put("name", 30);

    if (ht.get("name") != 30) {
        std::cout << "FAIL: update value\n";
        return;
    }

    if (ht.getSize() != 1) {
        std::cout << "FAIL: update should not increase size\n";
        return;
    }

    std::cout << "PASS: update\n";
}

void test_remove() {
    HashTable ht;

    ht.put("a", 1);
    ht.put("b", 2);
    ht.put("c", 3);

    ht.remove("b");

    if (ht.get("b") != -1) {
        std::cout << "FAIL: remove key\n";
        return;
    }

    if (ht.getSize() != 2) {
        std::cout << "FAIL: size after remove\n";
        return;
    }

    if (ht.get("a") != 1 || ht.get("c") != 3) {
        std::cout << "FAIL: other keys changed after remove\n";
        return;
    }

    std::cout << "PASS: remove\n";
}

void test_remove_missing_key() {
    HashTable ht;

    ht.put("a", 1);
    ht.put("b", 2);

    ht.remove("missing");

    if (ht.getSize() != 2) {
        std::cout << "FAIL: missing remove changed size\n";
        return;
    }

    if (ht.get("a") != 1 || ht.get("b") != 2) {
        std::cout << "FAIL: missing remove changed data\n";
        return;
    }

    std::cout << "PASS: remove missing key\n";
}

void test_rehash_keeps_data() {
    HashTable ht;

    for (int i = 0; i < 50; ++i) {
        ht.put("key" + std::to_string(i), i * 10);
    }

    if (ht.getSize() != 50) {
        std::cout << "FAIL: size after rehash\n";
        return;
    }

    for (int i = 0; i < 50; ++i) {
        std::string key = "key" + std::to_string(i);

        if (ht.get(key) != i * 10) {
            std::cout << "FAIL: rehash lost " << key << "\n";
            return;
        }
    }

    std::cout << "PASS: rehash keeps data\n";
}

int main() {
    test_put_and_get();
    test_update();
    test_remove();
    test_remove_missing_key();
    test_rehash_keeps_data();

    return 0;
}