#include <iostream>
#include "persistent_kv.hpp"

int main() 
{
    PersistentKV store("apexkv.log");

    store.put("name", "Alice");
    store.put("city", "Mumbai");
    store.put("language", "C++");

    std::cout << "name: " << store.get("name") << "\n";
    std::cout << "city: " << store.get("city") << "\n";
    std::cout << "size: " << store.getSize() << "\n";

    return 0;
}