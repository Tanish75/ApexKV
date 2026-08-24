#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <string>
#include <forward_list>
#include <utility>
#include <functional>

class HashTable {
private:
    std::forward_list<std::pair<std::string, int>>* buckets_;
    size_t capacity_;
    size_t size_;
    double max_load_factor_;

    size_t hash_func(const std::string& key) const {
        return std::hash<std::string>{}(key) % capacity_;
    }

public:
    HashTable();
    ~HashTable();

    void put(const std::string& key, int value);
    int get(const std::string& key) const;
    void remove(const std::string& key);
    size_t getSize() const;

    double load_factor() const;
    void rehash();
};

#endif // HASH_TABLE_HPP