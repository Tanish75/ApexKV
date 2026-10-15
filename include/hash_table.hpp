#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <string>
#include <vector>
#include <forward_list>
#include <utility>
#include <functional>
#include <optional>
#include <cstddef>

// ---------------------------------------------------------------------------
// HashTable — separate chaining hash table backed by std::vector of std::forward_list
// ---------------------------------------------------------------------------
class HashTable {
public:
    using Pair = std::pair<std::string, int>;

private:
    std::vector<std::forward_list<Pair>> table_;
    size_t cap_;
    size_t sz_;
    double max_load_;

    // Computes bucket index for key
    size_t hash_func(const std::string& key) const {
        if (cap_ == 0) return 0;
        return std::hash<std::string>{}(key) % cap_;
    }

public:
    explicit HashTable(size_t initial_cap = 16, double max_load_factor = 0.75)
        : table_(initial_cap > 0 ? initial_cap : 16),
          cap_(initial_cap > 0 ? initial_cap : 16),
          sz_(0),
          max_load_(max_load_factor > 0.0 ? max_load_factor : 0.75) {}

    // Rule of 5: std::vector and std::forward_list handle all memory cleanly
    ~HashTable() = default;
    HashTable(const HashTable&) = default;
    HashTable& operator=(const HashTable&) = default;

    HashTable(HashTable&& other) noexcept
        : table_(std::move(other.table_)),
          cap_(other.cap_),
          sz_(other.sz_),
          max_load_(other.max_load_) {
        other.cap_ = 0;
        other.sz_ = 0;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            table_ = std::move(other.table_);
            cap_ = other.cap_;
            sz_ = other.sz_;
            max_load_ = other.max_load_;
            other.cap_ = 0;
            other.sz_ = 0;
        }
        return *this;
    }

    // Direct pointer lookup (nullptr if key is not found)
    Pair* find(const std::string& key) {
        if (cap_ == 0 || sz_ == 0) return nullptr;
        size_t idx = hash_func(key);
        for (auto& pair : table_[idx]) {
            if (pair.first == key) {
                return &pair;
            }
        }
        return nullptr;
    }

    const Pair* find(const std::string& key) const {
        if (cap_ == 0 || sz_ == 0) return nullptr;
        size_t idx = hash_func(key);
        for (const auto& pair : table_[idx]) {
            if (pair.first == key) {
                return &pair;
            }
        }
        return nullptr;
    }

    // Insert or update. Returns true if new key was inserted, false if updated.
    bool put(const std::string& key, int value) {
        if (Pair* existing = find(key)) {
            existing->second = value;
            return false;
        }

        if (cap_ == 0) {
            cap_ = 16;
            table_.resize(cap_);
        }

        size_t idx = hash_func(key);
        table_[idx].push_front({key, value});
        sz_++;

        if (getLoadFactor() > max_load_) {
            rehash();
        }

        return true;
    }

    // Get optional value
    std::optional<int> get(const std::string& key) const {
        const Pair* pair = find(key);
        if (pair != nullptr) {
            return pair->second;
        }
        return std::nullopt;
    }

    // Lookup with fallback default
    int getOrDefault(const std::string& key, int default_value) const {
        auto res = get(key);
        return res.has_value() ? *res : default_value;
    }

    // Key existence check
    bool contains(const std::string& key) const {
        return find(key) != nullptr;
    }

    // Remove key from bucket chain
    bool remove(const std::string& key) {
        if (cap_ == 0 || sz_ == 0) {
            return false;
        }

        size_t idx = hash_func(key);
        auto& bucket = table_[idx];
        auto prev = bucket.before_begin();

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase_after(prev);
                sz_--;
                return true;
            }
            prev = it;
        }

        return false;
    }

    // Clear all elements
    void clear() noexcept {
        for (auto& bucket : table_) {
            bucket.clear();
        }
        sz_ = 0;
    }

    // State & Capacity Inspection
    size_t getSize() const noexcept {
        return sz_;
    }

    size_t getCapacity() const noexcept {
        return cap_;
    }

    double getLoadFactor() const noexcept {
        if (cap_ == 0) return 0.0;
        return static_cast<double>(sz_) / static_cast<double>(cap_);
    }

    double getMaxLoadFactor() const noexcept {
        return max_load_;
    }

    void setMaxLoadFactor(double load) {
        if (load > 0.05) {
            max_load_ = load;
            if (getLoadFactor() > max_load_) {
                rehash();
            }
        }
    }

    bool isEmpty() const noexcept {
        return sz_ == 0;
    }

    // Dynamic Resizing
    void rehash() {
        size_t new_cap = (cap_ > 0) ? (cap_ * 2) : 16;
        rehash(new_cap);
    }

    void rehash(size_t new_cap) {
        if (new_cap == 0) new_cap = 16;

        std::vector<std::forward_list<Pair>> new_table(new_cap);

        for (auto& bucket : table_) {
            for (auto& pair : bucket) {
                size_t idx = std::hash<std::string>{}(pair.first) % new_cap;
                new_table[idx].push_front(std::move(pair));
            }
        }

        table_ = std::move(new_table);
        cap_ = new_cap;
    }
};

#endif // HASH_TABLE_HPP