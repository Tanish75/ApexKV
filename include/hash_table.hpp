#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <string>
#include <forward_list>
#include <utility>
#include <functional>
#include <optional>
#include <cstddef>

class HashTable {
private:
    std::forward_list<std::pair<std::string, int>>* table;
    size_t cap;
    size_t sz;
    double maxLoad;

    // Hash and modulo to compute bucket index
    size_t hashFunc(const std::string& key) const {
        if (cap == 0) return 0;
        return std::hash<std::string>{}(key) % cap;
    }

    void freeTable() noexcept {
        delete[] table;
        table = nullptr;
        cap = 0;
        sz = 0;
    }

    void copyFrom(const HashTable& other) {
        cap = other.cap;
        sz = other.sz;
        maxLoad = other.maxLoad;
        if (cap > 0) {
            table = new std::forward_list<std::pair<std::string, int>>[cap];
            for (size_t i = 0; i < cap; ++i) {
                table[i] = other.table[i];
            }
        } else {
            table = nullptr;
        }
    }

public:
    explicit HashTable(size_t initialCap = 16, double maxLoadFactor = 0.75)
        : table(nullptr), cap(initialCap > 0 ? initialCap : 16), sz(0), maxLoad(maxLoadFactor > 0.0 ? maxLoadFactor : 0.75) {
        table = new std::forward_list<std::pair<std::string, int>>[cap];
    }

    // Rule of 5: Memory Lifecycle Management
    ~HashTable() noexcept {
        freeTable();
    }

    HashTable(const HashTable& other) {
        copyFrom(other);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            freeTable();
            copyFrom(other);
        }
        return *this;
    }

    HashTable(HashTable&& other) noexcept
        : table(other.table), cap(other.cap), sz(other.sz), maxLoad(other.maxLoad) {
        other.table = nullptr;
        other.cap = 0;
        other.sz = 0;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            freeTable();
            table = other.table;
            cap = other.cap;
            sz = other.sz;
            maxLoad = other.maxLoad;

            other.table = nullptr;
            other.cap = 0;
            other.sz = 0;
        }
        return *this;
    }

    // Core Key-Value Operations
    bool put(const std::string& key, int value) {
        if (cap == 0) {
            cap = 16;
            table = new std::forward_list<std::pair<std::string, int>>[cap];
        }

        size_t idx = hashFunc(key);

        // Check if key already exists, update value
        for (auto& kv : table[idx]) {
            if (kv.first == key) {
                kv.second = value;
                return false; // Existing key updated
            }
        }

        // Insert new key-value pair
        table[idx].push_front({key, value});
        sz++;

        // Check load factor and trigger rehash if needed
        if (getLoadFactor() > maxLoad) {
            rehash();
        }

        return true; // New key inserted
    }

    std::optional<int> get(const std::string& key) const {
        if (cap == 0 || sz == 0) {
            return std::nullopt;
        }

        size_t idx = hashFunc(key);
        for (const auto& kv : table[idx]) {
            if (kv.first == key) {
                return kv.second;
            }
        }

        return std::nullopt;
    }

    int getOrDefault(const std::string& key, int defaultValue) const {
        auto res = get(key);
        return res.has_value() ? *res : defaultValue;
    }

    bool contains(const std::string& key) const {
        return get(key).has_value();
    }

    bool remove(const std::string& key) {
        if (cap == 0 || sz == 0) {
            return false;
        }

        size_t idx = hashFunc(key);
        auto& bucket = table[idx];
        auto prev = bucket.before_begin();

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase_after(prev);
                sz--;
                return true;
            }
            prev = it;
        }

        return false;
    }

    void clear() noexcept {
        if (table != nullptr && cap > 0) {
            for (size_t i = 0; i < cap; ++i) {
                table[i].clear();
            }
        }
        sz = 0;
    }

    // State & Capacity Inspection
    size_t getSize() const noexcept {
        return sz;
    }

    size_t getCapacity() const noexcept {
        return cap;
    }

    double getLoadFactor() const noexcept {
        if (cap == 0) return 0.0;
        return static_cast<double>(sz) / static_cast<double>(cap);
    }

    double getMaxLoadFactor() const noexcept {
        return maxLoad;
    }

    void setMaxLoadFactor(double load) {
        if (load > 0.05) {
            maxLoad = load;
            if (getLoadFactor() > maxLoad) {
                rehash();
            }
        }
    }

    bool isEmpty() const noexcept {
        return sz == 0;
    }

    // Dynamic Resizing
    void rehash() {
        size_t newCap = (cap > 0) ? (cap * 2) : 16;
        rehash(newCap);
    }

    void rehash(size_t newCap) {
        if (newCap == 0) newCap = 16;

        auto* newTable = new std::forward_list<std::pair<std::string, int>>[newCap];

        // Rehash all existing elements into new table
        if (table != nullptr && cap > 0) {
            for (size_t i = 0; i < cap; ++i) {
                for (auto& kv : table[i]) {
                    size_t idx = std::hash<std::string>{}(kv.first) % newCap;
                    newTable[idx].push_front(std::move(kv));
                }
            }
        }

        delete[] table;
        table = newTable;
        cap = newCap;
    }
};

#endif // HASH_TABLE_HPP