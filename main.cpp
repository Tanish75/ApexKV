#include <iostream>
#include "hash_table.hpp"

int main() 
{
    HashTable ht;

    // Insert some keys (values as int)
    ht.put("name_code", 1);      // 1 = code for some name
    ht.put("age", 25);
    ht.put("city_code", 101);    // 101 = code for some city

    // Update existing key
    ht.put("name_code", 2);      // update value

    // Search
    std::cout << "name_code: " << ht.get("name_code") << "\n";
    std::cout << "age: " << ht.get("age") << "\n";
    std::cout << "city_code: " << ht.get("city_code") << "\n";
    std::cout << "country: " << ht.get("country") << "\n";  // not found -> -1

    // Load factor test
    std::cout << "Initial load factor: " << ht.load_factor() << "\n";

    // Insert enough keys to trigger resize
    for (int i = 0; i < 50; ++i) 
    {
        ht.put("key" + std::to_string(i), i);
    }

    std::cout << "After 50 inserts, load factor: " << ht.load_factor() << "\n";

    return 0;
}