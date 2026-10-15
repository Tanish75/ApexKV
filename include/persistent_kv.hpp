#ifndef PERSISTENT_KV_HPP
#define PERSISTENT_KV_HPP

#include "hash_table.hpp"
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <optional>
#include <array>

// ---------------------------------------------------------------------------
// CRC32 (standard polynomial 0xEDB88320, lookup-table based)
// Thread-safe function-local static initialization
// ---------------------------------------------------------------------------
namespace crc32_detail {

inline const uint32_t* get_crc32_table() {
    static const auto table = []() {
        std::array<uint32_t, 256> tbl{};
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xEDB88320u;
                else
                    crc >>= 1;
            }
            tbl[i] = crc;
        }
        return tbl;
    }();
    return table.data();
}

inline uint32_t compute_crc32(const void* data, size_t len) {
    const uint32_t* table = get_crc32_table();
    auto* bytes = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc = (crc >> 8) ^ table[(crc ^ bytes[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFFu;
}

} // namespace crc32_detail

// ---------------------------------------------------------------------------
// WAL record format (binary, length-prefixed):
//   PUT:     [total_len: 4B] [op: 1B] [key_len: 4B] [val_len: 4B] [key] [val] [crc: 4B]
//   DELETE:  [total_len: 4B] [op: 1B] [key_len: 4B] [val_len: 4B] [key] [crc: 4B]
//   PUT_TTL: [total_len: 4B] [op: 1B] [key_len: 4B] [val_len: 4B] [expire_at: 8B] [key] [val] [crc: 4B]
//   EXPIRE:  [total_len: 4B] [op: 1B] [key_len: 4B] [expire_at: 8B] [key] [crc: 4B]
// ---------------------------------------------------------------------------

enum class WalOp : uint8_t {
    PUT     = 1,
    DELETE  = 2,
    PUT_TTL = 3,
    EXPIRE  = 4
};

// ---------------------------------------------------------------------------
// PersistentKV — thread-safe hash table backed by binary WAL with TTL support
// ---------------------------------------------------------------------------
class PersistentKV {
private:
    HashTable<std::string, std::string> table_;
    std::unordered_map<std::string, int64_t> expiries_; // key -> unix timestamp seconds
    std::string wal_path_;
    std::ofstream wal_stream_;
    mutable std::shared_mutex mutex_; // Coarse reader-writer lock

    // Helper: current epoch seconds
    static int64_t now_seconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // Helper: low-level binary I/O
    static void write_u32(std::ostream& out, uint32_t val) {
        out.write(reinterpret_cast<const char*>(&val), 4);
    }

    static void write_u64(std::ostream& out, uint64_t val) {
        out.write(reinterpret_cast<const char*>(&val), 8);
    }

    static bool read_u32(std::istream& in, uint32_t& val) {
        return !!in.read(reinterpret_cast<char*>(&val), 4);
    }

    static bool read_u64(std::istream& in, uint64_t& val) {
        return !!in.read(reinterpret_cast<char*>(&val), 8);
    }

    // Check expiration without lock (must be called with lock held)
    bool is_expired_unlocked(const std::string& key) const {
        auto it = expiries_.find(key);
        if (it == expiries_.end() || it->second <= 0) return false;
        return now_seconds() >= it->second;
    }

    // WAL append methods
    void append_put(const std::string& key, const std::string& value) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = static_cast<uint32_t>(value.size());
        size_t payload_size = 1 + 4 + 4 + key_len + val_len;

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::PUT);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &val_len, 4); pos += 4;
        if (key_len > 0) {
            std::memcpy(&payload[pos], key.data(), key_len);
            pos += key_len;
        }
        if (val_len > 0) {
            std::memcpy(&payload[pos], value.data(), val_len);
            pos += val_len;
        }

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    void append_put_ttl(const std::string& key, const std::string& value, int64_t expire_at) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = static_cast<uint32_t>(value.size());
        size_t payload_size = 1 + 4 + 4 + 8 + key_len + val_len;

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::PUT_TTL);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &val_len, 4); pos += 4;
        std::memcpy(&payload[pos], &expire_at, 8); pos += 8;
        if (key_len > 0) {
            std::memcpy(&payload[pos], key.data(), key_len);
            pos += key_len;
        }
        if (val_len > 0) {
            std::memcpy(&payload[pos], value.data(), val_len);
            pos += val_len;
        }

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    void append_delete(const std::string& key) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = 0;
        size_t payload_size = 1 + 4 + 4 + key_len;

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::DELETE);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &val_len, 4); pos += 4;
        if (key_len > 0) {
            std::memcpy(&payload[pos], key.data(), key_len);
            pos += key_len;
        }

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    void append_expire(const std::string& key, int64_t expire_at) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        size_t payload_size = 1 + 4 + 8 + key_len;

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::EXPIRE);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &expire_at, 8); pos += 8;
        if (key_len > 0) {
            std::memcpy(&payload[pos], key.data(), key_len);
            pos += key_len;
        }

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    // Unlocked operations
    bool remove_unlocked(const std::string& key) {
        if (!table_.contains(key)) return false;
        append_delete(key);
        expiries_.erase(key);
        return table_.remove(key);
    }

    bool put_unlocked(const std::string& key, const std::string& value, int64_t ttl_seconds) {
        if (ttl_seconds > 0) {
            int64_t expire_at = now_seconds() + ttl_seconds;
            append_put_ttl(key, value, expire_at);
            expiries_[key] = expire_at;
        } else {
            append_put(key, value);
            expiries_.erase(key);
        }
        return table_.put(key, value);
    }

    // Recovery from WAL on startup
    size_t replay_wal() {
        std::ifstream in(wal_path_, std::ios::binary);
        if (!in.is_open()) return 0;

        size_t good_pos = 0;

        while (true) {
            auto start = in.tellg();
            if (start == -1) break;

            uint32_t total_len = 0;
            if (!read_u32(in, total_len)) break;

            constexpr uint32_t MIN_RECORD_LEN = 13;             // 1B op + 4B key_len + 4B val_len + 4B crc
            constexpr uint32_t MAX_RECORD_LEN = 64 * 1024 * 1024; // 64 MB cap
            if (total_len < MIN_RECORD_LEN || total_len > MAX_RECORD_LEN) {
                break;
            }

            std::vector<uint8_t> body(total_len);
            if (!in.read(reinterpret_cast<char*>(body.data()),
                         static_cast<std::streamsize>(total_len))) {
                break;
            }

            size_t payload_size = total_len - 4;
            uint32_t stored_crc = 0;
            std::memcpy(&stored_crc, body.data() + payload_size, 4);
            uint32_t computed_crc = crc32_detail::compute_crc32(body.data(), payload_size);

            if (stored_crc != computed_crc) {
                break;
            }

            if (payload_size < 1) break;

            size_t pos = 0;
            auto op = static_cast<WalOp>(body[pos++]);

            if (op == WalOp::PUT) {
                if (static_cast<uint64_t>(pos) + 8 > static_cast<uint64_t>(payload_size)) break;
                uint32_t key_len = 0, val_len = 0;
                std::memcpy(&key_len, body.data() + pos, 4); pos += 4;
                std::memcpy(&val_len, body.data() + pos, 4); pos += 4;

                uint64_t needed = static_cast<uint64_t>(pos) + static_cast<uint64_t>(key_len) + static_cast<uint64_t>(val_len);
                if (needed > static_cast<uint64_t>(payload_size)) break;

                std::string key(reinterpret_cast<const char*>(body.data() + pos), key_len); pos += key_len;
                std::string value(reinterpret_cast<const char*>(body.data() + pos), val_len);
                table_.put(key, value);
                expiries_.erase(key);

            } else if (op == WalOp::DELETE) {
                if (static_cast<uint64_t>(pos) + 8 > static_cast<uint64_t>(payload_size)) break;
                uint32_t key_len = 0, val_len = 0;
                std::memcpy(&key_len, body.data() + pos, 4); pos += 4;
                std::memcpy(&val_len, body.data() + pos, 4); pos += 4;

                uint64_t needed = static_cast<uint64_t>(pos) + static_cast<uint64_t>(key_len);
                if (needed > static_cast<uint64_t>(payload_size)) break;

                std::string key(reinterpret_cast<const char*>(body.data() + pos), key_len);
                table_.remove(key);
                expiries_.erase(key);

            } else if (op == WalOp::PUT_TTL) {
                if (static_cast<uint64_t>(pos) + 16 > static_cast<uint64_t>(payload_size)) break;
                uint32_t key_len = 0, val_len = 0;
                int64_t expire_at = 0;
                std::memcpy(&key_len, body.data() + pos, 4); pos += 4;
                std::memcpy(&val_len, body.data() + pos, 4); pos += 4;
                std::memcpy(&expire_at, body.data() + pos, 8); pos += 8;

                uint64_t needed = static_cast<uint64_t>(pos) + static_cast<uint64_t>(key_len) + static_cast<uint64_t>(val_len);
                if (needed > static_cast<uint64_t>(payload_size)) break;

                std::string key(reinterpret_cast<const char*>(body.data() + pos), key_len); pos += key_len;
                std::string value(reinterpret_cast<const char*>(body.data() + pos), val_len);
                table_.put(key, value);
                expiries_[key] = expire_at;

            } else if (op == WalOp::EXPIRE) {
                if (static_cast<uint64_t>(pos) + 12 > static_cast<uint64_t>(payload_size)) break;
                uint32_t key_len = 0;
                int64_t expire_at = 0;
                std::memcpy(&key_len, body.data() + pos, 4); pos += 4;
                std::memcpy(&expire_at, body.data() + pos, 8); pos += 8;

                uint64_t needed = static_cast<uint64_t>(pos) + static_cast<uint64_t>(key_len);
                if (needed > static_cast<uint64_t>(payload_size)) break;

                std::string key(reinterpret_cast<const char*>(body.data() + pos), key_len);
                if (table_.contains(key)) {
                    expiries_[key] = expire_at;
                }
            } else {
                // Unknown opcode: stop replay immediately
                break;
            }

            good_pos = static_cast<size_t>(in.tellg());
        }

        in.close();
        truncate_file(wal_path_, good_pos);
        return good_pos;
    }

    static void truncate_file(const std::string& path, size_t size) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return;

        std::vector<char> buf(size);
        if (size > 0) {
            in.read(buf.data(), static_cast<std::streamsize>(size));
        }
        in.close();

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (size > 0) {
            out.write(buf.data(), static_cast<std::streamsize>(size));
        }
        out.close();
    }

public:
    explicit PersistentKV(const std::string& wal_path)
        : wal_path_(wal_path) {

        replay_wal();

        wal_stream_.open(wal_path_, std::ios::binary | std::ios::app);
        if (!wal_stream_.is_open()) {
            wal_stream_.clear();
            wal_stream_.open(wal_path_, std::ios::binary | std::ios::out | std::ios::trunc);
        }
    }

    ~PersistentKV() {
        if (wal_stream_.is_open()) {
            wal_stream_.flush();
            wal_stream_.close();
        }
    }

    PersistentKV(const PersistentKV&) = delete;
    PersistentKV& operator=(const PersistentKV&) = delete;

    // ----- Public API (Thread-Safe) -----

    // Put with optional TTL (0 = no expiry)
    bool put(const std::string& key, const std::string& value, int64_t ttl_seconds = 0) {
        std::unique_lock lock(mutex_);
        return put_unlocked(key, value, ttl_seconds);
    }

    bool put(const std::string& key, int value, int64_t ttl_seconds = 0) {
        std::unique_lock lock(mutex_);
        return put_unlocked(key, std::to_string(value), ttl_seconds);
    }

    // Set TTL on an existing key (returns false if key does not exist)
    bool expire(const std::string& key, int64_t ttl_seconds) {
        std::unique_lock lock(mutex_);
        if (!table_.contains(key)) return false;
        if (is_expired_unlocked(key)) {
            remove_unlocked(key);
            return false;
        }
        if (ttl_seconds > 0) {
            int64_t expire_at = now_seconds() + ttl_seconds;
            append_expire(key, expire_at);
            expiries_[key] = expire_at;
        } else {
            remove_unlocked(key);
        }
        return true;
    }

    // Query remaining TTL in seconds
    // -2: key not found or expired
    // -1: key exists with no TTL set
    // >= 0: seconds remaining until expiry
    int64_t ttl(const std::string& key) {
        {
            std::shared_lock lock(mutex_);
            if (!table_.contains(key)) return -2;
            if (!is_expired_unlocked(key)) {
                auto it = expiries_.find(key);
                if (it == expiries_.end() || it->second <= 0) return -1;
                return it->second - now_seconds();
            }
        }
        // Key is expired: clean up with write lock
        std::unique_lock lock(mutex_);
        if (is_expired_unlocked(key)) {
            remove_unlocked(key);
        }
        return -2;
    }

    // Get value (performs lazy removal if expired)
    std::optional<std::string> get(const std::string& key) {
        {
            std::shared_lock lock(mutex_);
            if (!is_expired_unlocked(key)) {
                return table_.get(key);
            }
        }
        std::unique_lock lock(mutex_);
        if (is_expired_unlocked(key)) {
            remove_unlocked(key);
        }
        return std::nullopt;
    }

    // Check existence (performs lazy removal if expired)
    bool contains(const std::string& key) {
        {
            std::shared_lock lock(mutex_);
            if (!is_expired_unlocked(key)) {
                return table_.contains(key);
            }
        }
        std::unique_lock lock(mutex_);
        if (is_expired_unlocked(key)) {
            remove_unlocked(key);
        }
        return false;
    }

    // Delete key
    bool remove(const std::string& key) {
        std::unique_lock lock(mutex_);
        return remove_unlocked(key);
    }

    // Count active non-expired elements
    size_t size() const {
        std::shared_lock lock(mutex_);
        size_t total = table_.getSize();
        size_t expired_count = 0;
        int64_t now = now_seconds();
        for (const auto& [k, exp] : expiries_) {
            if (exp > 0 && now >= exp && table_.contains(k)) {
                expired_count++;
            }
        }
        return (total >= expired_count) ? (total - expired_count) : 0;
    }

    // Compacts the WAL by writing only active, non-expired keys to a temp file,
    // then atomically replacing the WAL file.
    bool compact() {
        std::unique_lock lock(mutex_);
        std::string tmp_path = wal_path_ + ".compact.tmp";

        // Overwrite or create fresh temporary file
        std::remove(tmp_path.c_str());

        {
            std::ofstream tmp_stream(tmp_path, std::ios::binary | std::ios::trunc);
            if (!tmp_stream.is_open()) {
                return false;
            }

            int64_t now = now_seconds();
            table_.for_each([&](const std::string& key, const std::string& value) {
                auto it = expiries_.find(key);
                if (it != expiries_.end() && it->second > 0) {
                    if (now >= it->second) {
                        return; // Drop expired key
                    }
                    // Write PUT_TTL record
                    uint32_t key_len = static_cast<uint32_t>(key.size());
                    uint32_t val_len = static_cast<uint32_t>(value.size());
                    int64_t expire_at = it->second;
                    size_t payload_size = 1 + 4 + 4 + 8 + key_len + val_len;

                    std::vector<uint8_t> payload(payload_size);
                    size_t pos = 0;
                    payload[pos++] = static_cast<uint8_t>(WalOp::PUT_TTL);
                    std::memcpy(&payload[pos], &key_len, 4); pos += 4;
                    std::memcpy(&payload[pos], &val_len, 4); pos += 4;
                    std::memcpy(&payload[pos], &expire_at, 8); pos += 8;
                    if (key_len > 0) {
                        std::memcpy(&payload[pos], key.data(), key_len);
                        pos += key_len;
                    }
                    if (val_len > 0) {
                        std::memcpy(&payload[pos], value.data(), val_len);
                        pos += val_len;
                    }

                    uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
                    uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

                    write_u32(tmp_stream, total_len);
                    tmp_stream.write(reinterpret_cast<const char*>(payload.data()),
                                     static_cast<std::streamsize>(payload.size()));
                    write_u32(tmp_stream, crc);
                } else {
                    // Write PUT record
                    uint32_t key_len = static_cast<uint32_t>(key.size());
                    uint32_t val_len = static_cast<uint32_t>(value.size());
                    size_t payload_size = 1 + 4 + 4 + key_len + val_len;

                    std::vector<uint8_t> payload(payload_size);
                    size_t pos = 0;
                    payload[pos++] = static_cast<uint8_t>(WalOp::PUT);
                    std::memcpy(&payload[pos], &key_len, 4); pos += 4;
                    std::memcpy(&payload[pos], &val_len, 4); pos += 4;
                    if (key_len > 0) {
                        std::memcpy(&payload[pos], key.data(), key_len);
                        pos += key_len;
                    }
                    if (val_len > 0) {
                        std::memcpy(&payload[pos], value.data(), val_len);
                        pos += val_len;
                    }

                    uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
                    uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

                    write_u32(tmp_stream, total_len);
                    tmp_stream.write(reinterpret_cast<const char*>(payload.data()),
                                     static_cast<std::streamsize>(payload.size()));
                    write_u32(tmp_stream, crc);
                }
            });

            tmp_stream.flush();
            if (!tmp_stream.good()) {
                tmp_stream.close();
                std::remove(tmp_path.c_str());
                return false;
            }
            tmp_stream.close();
        }

        // Purge expired keys from memory
        int64_t now = now_seconds();
        std::vector<std::string> expired_keys;
        for (const auto& [k, exp] : expiries_) {
            if (exp > 0 && now >= exp) {
                expired_keys.push_back(k);
            }
        }
        for (const auto& k : expired_keys) {
            expiries_.erase(k);
            table_.remove(k);
        }

        if (wal_stream_.is_open()) {
            wal_stream_.close();
        }

        std::remove(wal_path_.c_str());
        if (std::rename(tmp_path.c_str(), wal_path_.c_str()) != 0) {
            wal_stream_.open(wal_path_, std::ios::binary | std::ios::app);
            return false;
        }

        wal_stream_.open(wal_path_, std::ios::binary | std::ios::app);
        return wal_stream_.is_open();
    }

    const std::string& wal_path() const { return wal_path_; }
};

#endif // PERSISTENT_KV_HPP
