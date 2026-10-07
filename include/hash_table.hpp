#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <string>
#include <forward_list>
#include <utility>
#include <functional>

class HashTable {
private:
    std::forward_list<std::pair<std::string, int>>* table;
    size_t cap;
    size_t sz;
    double maxLoad;

    // hash and mod to get bucket index
    size_t hashFunc(const std::string& key) const {
        return std::hash<std::string>{}(key) % cap;
    }

public:
    HashTable();
    ~HashTable();

    void put(const std::string& key, int value);
    int get(std::string key) const;
    void remove(std::string key);
    size_t getSize() const;

    double loadFactor() const;
    void rehash();
};

#endif