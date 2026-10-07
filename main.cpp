#include <iostream>
#include "hash_table.hpp"

int main() 
{
    HashTable ht;

    // basic insert
    ht.put("name", 1);
    ht.put("age", 25);
    ht.put("city", 101);

    // update check
    ht.put("name", 2);

    // print results
    std::cout << "name: " << ht.get("name") << "\n";
    std::cout << "age: " << ht.get("age") << "\n";
    std::cout << "city: " << ht.get("city") << "\n";
    std::cout << "country: " << ht.get("country") << "\n"; // should be -1

    std::cout << "load factor: " << ht.loadFactor() << "\n";

    // insert 50 keys to trigger resize
    for (int i = 0; i < 50; i++) {
        ht.put("key" + std::to_string(i), i);
    }

    std::cout << "after 50 inserts, load factor: " << ht.loadFactor() << "\n";
    std::cout << "size: " << ht.getSize() << "\n";

    // remove test
    ht.remove("age");
    std::cout << "age after remove: " << ht.get("age") << "\n"; // -1

    return 0;
}