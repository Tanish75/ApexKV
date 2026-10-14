#ifndef PERSISTENT_KV_HPP
#define PERSISTENT_KV_HPP

#include "hash_table.hpp"
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cstdio>    // std::rename, std::remove

// ---------------------------------------------------------------------------
// CRC32 (standard polynomial 0xEDB88320, lookup-table based)
// ---------------------------------------------------------------------------
namespace crc32_detail {

inline uint32_t crc32_table[256] = {};
inline bool crc32_table_ready = false;

inline void build_crc32_table() {
    if (crc32_table_ready) return;
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320u;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
    crc32_table_ready = true;
}

inline uint32_t compute_crc32(const void* data, size_t len) {
    build_crc32_table();
    auto* bytes = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFFu;
}

} // namespace crc32_detail

// ---------------------------------------------------------------------------
// WAL record format (binary, length-prefixed):
//   [total_len: 4 bytes] [op: 1 byte] [key_len: 4 bytes] [val_len: 4 bytes]
//   [key: key_len bytes] [value: val_len bytes] [crc32: 4 bytes]
//
// op values: 1 = PUT, 2 = DELETE
// CRC32 covers everything from op through value (excludes total_len and crc32).
// ---------------------------------------------------------------------------

enum class WalOp : uint8_t {
    PUT    = 1,
    DELETE = 2
};

// ---------------------------------------------------------------------------
// PersistentKV — hash table backed by a write-ahead log
// ---------------------------------------------------------------------------
class PersistentKV {
private:
    HashTable table_;
    std::string wal_path_;
    std::string snapshot_path_;
    std::ofstream wal_stream_;     // kept open for appending

    // ----- Low-level binary I/O helpers -----

    static void write_u32(std::ostream& out, uint32_t val) {
        out.write(reinterpret_cast<const char*>(&val), 4);
    }

    static void write_u8(std::ostream& out, uint8_t val) {
        out.write(reinterpret_cast<const char*>(&val), 1);
    }

    static bool read_u32(std::istream& in, uint32_t& val) {
        return !!in.read(reinterpret_cast<char*>(&val), 4);
    }

    static bool read_u8(std::istream& in, uint8_t& val) {
        return !!in.read(reinterpret_cast<char*>(&val), 1);
    }

    // ----- WAL writing -----

    void append_put(const std::string& key, const std::string& value) {
        // Build payload: op(1) + key_len(4) + val_len(4) + key + value
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = static_cast<uint32_t>(value.size());
        size_t payload_size = 1 + 4 + 4 + key.size() + value.size();

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::PUT);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &val_len, 4); pos += 4;
        std::memcpy(&payload[pos], key.data(), key.size()); pos += key.size();
        std::memcpy(&payload[pos], value.data(), value.size());

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4); // payload + crc

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    void append_delete(const std::string& key) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = 0;
        size_t payload_size = 1 + 4 + 4 + key.size();

        std::vector<uint8_t> payload(payload_size);
        size_t pos = 0;
        payload[pos++] = static_cast<uint8_t>(WalOp::DELETE);
        std::memcpy(&payload[pos], &key_len, 4); pos += 4;
        std::memcpy(&payload[pos], &val_len, 4); pos += 4;
        std::memcpy(&payload[pos], key.data(), key.size());

        uint32_t crc = crc32_detail::compute_crc32(payload.data(), payload.size());
        uint32_t total_len = static_cast<uint32_t>(payload_size + 4);

        write_u32(wal_stream_, total_len);
        wal_stream_.write(reinterpret_cast<const char*>(payload.data()),
                          static_cast<std::streamsize>(payload.size()));
        write_u32(wal_stream_, crc);
        wal_stream_.flush();
    }

    // ----- Recovery -----

    // Replays the WAL from disk, stops at first corrupt/truncated record.
    // Returns the byte offset of the last known-good position.
    size_t replay_wal() {
        std::ifstream in(wal_path_, std::ios::binary);
        if (!in.is_open()) return 0;

        size_t good_pos = 0;

        while (true) {
            // Remember position before reading next record
            auto start = in.tellg();
            if (start == -1) break;

            // Read total_len
            uint32_t total_len = 0;
            if (!read_u32(in, total_len)) break;      // EOF — clean end
            if (total_len < 10) break;                 // impossibly small

            // Read the entire record body (payload + crc)
            std::vector<uint8_t> body(total_len);
            if (!in.read(reinterpret_cast<char*>(body.data()),
                         static_cast<std::streamsize>(total_len))) {
                break;  // truncated record
            }

            // Verify CRC (last 4 bytes of body are the stored CRC)
            size_t payload_size = total_len - 4;
            uint32_t stored_crc = 0;
            std::memcpy(&stored_crc, &body[payload_size], 4);
            uint32_t computed_crc =
                crc32_detail::compute_crc32(body.data(), payload_size);

            if (stored_crc != computed_crc) {
                break;  // corrupt record — stop here
            }

            // Parse the validated payload
            size_t pos = 0;
            auto op = static_cast<WalOp>(body[pos++]);
            uint32_t key_len = 0, val_len = 0;
            std::memcpy(&key_len, &body[pos], 4); pos += 4;
            std::memcpy(&val_len, &body[pos], 4); pos += 4;

            if (pos + key_len + val_len > payload_size) break; // malformed

            std::string key(reinterpret_cast<char*>(&body[pos]), key_len);
            pos += key_len;
            std::string value(reinterpret_cast<char*>(&body[pos]), val_len);

            // Apply to in-memory table
            if (op == WalOp::PUT) {
                table_.put(key, std::stoi(value));
            } else if (op == WalOp::DELETE) {
                table_.remove(key);
            }

            good_pos = static_cast<size_t>(in.tellg());
        }

        in.close();

        // Truncate the WAL at the last good position to discard corruption
        truncate_file(wal_path_, good_pos);

        return good_pos;
    }

    // Truncate a file to `size` bytes by rewriting it.
    static void truncate_file(const std::string& path, size_t size) {
        // Read the good portion
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return;

        std::vector<char> buf(size);
        if (size > 0) {
            in.read(buf.data(), static_cast<std::streamsize>(size));
        }
        in.close();

        // Rewrite the file with only the good bytes
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (size > 0) {
            out.write(buf.data(), static_cast<std::streamsize>(size));
        }
        out.close();
    }

public:
    // Constructor: recovers state from WAL, then opens for appending.
    explicit PersistentKV(const std::string& wal_path,
                          const std::string& snapshot_path = "")
        : wal_path_(wal_path),
          snapshot_path_(snapshot_path.empty() ? wal_path + ".snapshot" : snapshot_path)
    {
        replay_wal();
        wal_stream_.open(wal_path_, std::ios::binary | std::ios::app);
    }

    ~PersistentKV() {
        if (wal_stream_.is_open()) {
            wal_stream_.close();
        }
    }

    // Not copyable (owns a file stream)
    PersistentKV(const PersistentKV&) = delete;
    PersistentKV& operator=(const PersistentKV&) = delete;

    // ----- Public API -----

    bool put(const std::string& key, const std::string& value) {
        int int_val = std::stoi(value);
        append_put(key, value);
        return table_.put(key, int_val);
    }

    // Overload for int values
    bool put(const std::string& key, int value) {
        std::string val_str = std::to_string(value);
        append_put(key, val_str);
        return table_.put(key, value);
    }

    std::optional<std::string> get(const std::string& key) const {
        auto result = table_.get(key);
        if (!result.has_value()) return std::nullopt;
        return std::to_string(*result);
    }

    bool contains(const std::string& key) const {
        return table_.contains(key);
    }

    bool remove(const std::string& key) {
        if (!table_.contains(key)) return false;
        append_delete(key);
        return table_.remove(key);
    }

    size_t size() const {
        return table_.getSize();
    }

    // ----- Snapshot -----

    // Write current state to a temp file, then atomically rename.
    bool save_snapshot() {
        std::string tmp_path = snapshot_path_ + ".tmp";

        {
            std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
            if (!out.is_open()) return false;

            // Snapshot format: series of PUT records (same as WAL)
            // We cannot iterate HashTable yet, so we replay the WAL
            // to collect the final state.
            // For now, snapshot = copy of the current WAL.
            std::ifstream wal_in(wal_path_, std::ios::binary);
            if (wal_in.is_open()) {
                out << wal_in.rdbuf();
            }
        }

        // Atomic rename (POSIX rename is atomic; on Windows, best-effort)
        std::remove(snapshot_path_.c_str());
        return std::rename(tmp_path.c_str(), snapshot_path_.c_str()) == 0;
    }

    const std::string& wal_path() const { return wal_path_; }
    const std::string& snapshot_path() const { return snapshot_path_; }
};

#endif // PERSISTENT_KV_HPP
