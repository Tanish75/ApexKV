#include "include/hash_table.hpp"
#include <iostream>
using namespace std;


int main() {
    HashTable ht(5);

    // Test PUT
    ht.put("name", "Alice");
    ht.put("age", "25");
    ht.put("city", "Mumbai");
    ht.put("name", "Bob");  // update

    // Test GET
    cout << "name: " << ht.get("name") << endl;
    cout << "age: " << ht.get("age") << endl;
    cout << "city: " << ht.get("city") << endl;

    // Test REMOVE
    ht.remove("age");

    // Final
    cout << "Final size: " << ht.getSize() << endl;

    return 0;
}
