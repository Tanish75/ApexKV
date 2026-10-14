#include <iostream>
#include <string>
#include "hash_table.hpp"

int main() {
    std::cout << "========================================\n";
    std::cout << "         ApexKV In-Memory Store         \n";
    std::cout << "========================================\n\n";

    HashTable ht;

    std::cout << "[1] Inserting initial key-value pairs...\n";
    ht.put("name", 1);
    ht.put("age", 25);
    ht.put("city", 101);

    std::cout << "  - name: " << ht.getOrDefault("name", -1) << "\n";
    std::cout << "  - age: " << ht.getOrDefault("age", -1) << "\n";
    std::cout << "  - city: " << ht.getOrDefault("city", -1) << "\n";
    std::cout << "  - country (non-existent): " << (ht.contains("country") ? "found" : "not found") << "\n";

    std::cout << "\n[2] Updating existing key ('name' -> 2)...\n";
    bool inserted = ht.put("name", 2);
    std::cout << "  - Was new insert? " << (inserted ? "yes" : "no (updated)") << "\n";
    std::cout << "  - Updated name: " << ht.getOrDefault("name", -1) << "\n";

    std::cout << "\n[3] Triggering dynamic rehashing (inserting 50 items)...\n";
    std::cout << "  - Initial capacity: " << ht.getCapacity() << ", load factor: " << ht.getLoadFactor() << "\n";
    for (int i = 0; i < 50; i++) {
        ht.put("key" + std::to_string(i), i);
    }
    std::cout << "  - After 50 inserts: size=" << ht.getSize() 
              << ", capacity=" << ht.getCapacity() 
              << ", load factor=" << ht.getLoadFactor() << "\n";

    std::cout << "\n[4] Deleting key 'age'...\n";
    bool removed = ht.remove("age");
    std::cout << "  - Removed successfully? " << (removed ? "yes" : "no") << "\n";
    std::cout << "  - Lookup 'age' after removal: " << (ht.contains("age") ? "found" : "not found") << "\n";

    std::cout << "\n[5] Final store status:\n";
    std::cout << "  - Total elements: " << ht.getSize() << "\n";
    std::cout << "  - Capacity buckets: " << ht.getCapacity() << "\n";
    std::cout << "  - Max load threshold: " << ht.getMaxLoadFactor() << "\n";
    std::cout << "========================================\n";

    return 0;
}