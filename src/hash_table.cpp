#include "hash_table.hpp"

HashTable::HashTable() {
    cap = 16;
    sz = 0;
    maxLoad = 0.75;
    table = new std::forward_list<std::pair<std::string, int>>[cap];
}

HashTable::~HashTable() {
    delete[] table;
}

void HashTable::put(const std::string& key, int value) 
{
    size_t idx = hashFunc(key);

    // check if already there, update it
    for (auto& kv : table[idx]) {
        if (kv.first == key) {
            kv.second = value;
            return;
        }
    }

    // naya key insert
    table[idx].push_front({key, value});
    sz++;

    // zyada ho gaya to resize
    if (loadFactor() > maxLoad) {
        rehash();
    }
}

int HashTable::get(std::string key) const 
{
    size_t idx = hashFunc(key);

    for (const auto& kv : table[idx]) {
        if (kv.first == key)
            return kv.second;
    }

    return -1; // not found
}

void HashTable::remove(std::string key) 
{
    size_t idx = hashFunc(key);
    auto& bucket = table[idx];
    auto prev = bucket.before_begin();

    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if (it->first == key) {
            bucket.erase_after(prev);
            sz--;
            return;
        }
        prev = it;
    }
}

size_t HashTable::getSize() const {
    return sz;
}

double HashTable::loadFactor() const {
    if (cap == 0) return 0.0;
    return (double)sz / (double)cap;
}

void HashTable::rehash() 
{
    size_t newCap = cap * 2;
    auto* newTable = new std::forward_list<std::pair<std::string, int>>[newCap];

    // move everything to new table
    for (size_t i = 0; i < cap; i++) {
        for (auto& kv : table[i]) {
            size_t idx = std::hash<std::string>{}(kv.first) % newCap;
            newTable[idx].push_front(std::move(kv));
        }
    }

    delete[] table;
    table = newTable;
    cap = newCap;
}