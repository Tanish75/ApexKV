#include <iostream>
#include "hash_table.hpp"

void test_insert_and_search() {
    HashTable ht;
    ht.put("a", 10);
    ht.put("b", 20);
    ht.put("c", 30);

    if (ht.get("a") != 10) {
        std::cout << "FAIL: get('a')\n";
        return;
    }
    if (ht.get("b") != 20) {
        std::cout << "FAIL: get('b')\n";
        return;
    }
    if (ht.get("c") != 30) {
        std::cout << "FAIL: get('c')\n";
        return;
    }
    if (ht.get("x") != -1) {
        std::cout << "FAIL: get('x') (not found)\n";
        return;
    }

    std::cout << "PASS: insert and search\n";
}

void test_update() {
    HashTable ht;
    ht.put("x", 1);
    ht.put("x", 100);  // update

    if (ht.get("x") != 100) {
        std::cout << "FAIL: update\n";
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
        std::cout << "FAIL: remove('b')\n";
        return;
    }
    if (ht.get("a") != 1 || ht.get("c") != 3) {
        std::cout << "FAIL: other keys after remove\n";
        return;
    }

    std::cout << "PASS: remove\n";
}

void test_size() {
    HashTable ht;
    ht.put("a", 1);
    ht.put("b", 2);
    ht.put("c", 3);

    if (ht.getSize() != 3) {
        std::cout << "FAIL: size after 3 inserts\n";
        return;
    }

    ht.remove("b");
    if (ht.getSize() != 2) {
        std::cout << "FAIL: size after remove\n";
        return;
    }

    std::cout << "PASS: size\n";
}

int main() 
{
    test_insert_and_search();
    test_update();
    test_remove();
    test_size();

    return 0;
}