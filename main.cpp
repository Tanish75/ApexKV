#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include "persistent_kv.hpp"

// ---------------------------------------------------------------------------
// ApexKV CLI — interactive command-line interface
//
// Commands:
//   SET <key> <value> [ttl]  — insert or update a key with optional TTL in seconds
//   GET <key>                — retrieve value for key
//   DEL <key>                — delete a key
//   EXPIRE <key> <secs>      — set expiration for existing key
//   TTL <key>                — check remaining TTL in seconds (-1: none, -2: absent)
//   SIZE                     — print number of active keys
//   HELP                     — print available commands
//   EXIT / QUIT              — exit the CLI
// ---------------------------------------------------------------------------

static void print_help() {
    std::cout << "Commands:\n"
              << "  SET <key> <value> [ttl] — store a key-value pair with optional TTL (seconds)\n"
              << "  GET <key>               — retrieve value for key\n"
              << "  DEL <key>               — delete a key\n"
              << "  EXPIRE <key> <secs>     — set expiration TTL in seconds\n"
              << "  TTL <key>               — get remaining TTL (-1: infinite, -2: not found)\n"
              << "  SIZE                    — show number of active keys\n"
              << "  COMPACT                 — compact WAL log to purge stale and expired keys\n"
              << "  HELP                    — display this help menu\n"
              << "  EXIT / QUIT             — exit\n";
}

int main() {
    std::cout << "========================================\n"
              << "         ApexKV v2 — Interactive CLI    \n"
              << "========================================\n";

    PersistentKV kv("apexkv.wal");

    std::cout << "Recovered " << kv.size() << " key(s) from WAL.\n";
    std::cout << "Type a command (or HELP):\n\n";

    std::string line;
    while (true) {
        std::cout << "apexkv> ";
        if (!std::getline(std::cin, line)) break;  // EOF

        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) continue;  // empty line

        // Convert command to uppercase
        for (auto& c : cmd) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        if (cmd == "SET") {
            std::string key, value;
            if (!(iss >> key >> value)) {
                std::cout << "Usage: SET <key> <value> [ttl_seconds]\n";
                continue;
            }
            int64_t ttl_secs = 0;
            iss >> ttl_secs;

            bool is_new = kv.put(key, value, ttl_secs);
            std::cout << (is_new ? "(new) " : "(updated) ") << key << " = " << value;
            if (ttl_secs > 0) {
                std::cout << " (TTL: " << ttl_secs << "s)";
            }
            std::cout << "\n";

        } else if (cmd == "GET") {
            std::string key;
            if (!(iss >> key)) {
                std::cout << "Usage: GET <key>\n";
                continue;
            }
            auto val = kv.get(key);
            if (val.has_value()) {
                std::cout << key << " = " << val.value() << "\n";
            } else {
                std::cout << "(nil)\n";
            }

        } else if (cmd == "DEL") {
            std::string key;
            if (!(iss >> key)) {
                std::cout << "Usage: DEL <key>\n";
                continue;
            }
            bool removed = kv.remove(key);
            std::cout << (removed ? "(deleted)" : "(not found)") << "\n";

        } else if (cmd == "EXPIRE") {
            std::string key;
            int64_t secs = 0;
            if (!(iss >> key >> secs) || secs <= 0) {
                std::cout << "Usage: EXPIRE <key> <positive_seconds>\n";
                continue;
            }
            bool ok = kv.expire(key, secs);
            std::cout << (ok ? "(ok)" : "(key not found)") << "\n";

        } else if (cmd == "TTL") {
            std::string key;
            if (!(iss >> key)) {
                std::cout << "Usage: TTL <key>\n";
                continue;
            }
            int64_t remaining = kv.ttl(key);
            if (remaining == -2) {
                std::cout << "(not found / expired)\n";
            } else if (remaining == -1) {
                std::cout << "(no TTL set - persistent)\n";
            } else {
                std::cout << remaining << " second(s) remaining\n";
            }

        } else if (cmd == "SIZE") {
            std::cout << kv.size() << " key(s)\n";

        } else if (cmd == "COMPACT") {
            bool ok = kv.compact();
            std::cout << (ok ? "(compacted)\n" : "(compaction failed)\n");

        } else if (cmd == "HELP") {
            print_help();

        } else if (cmd == "EXIT" || cmd == "QUIT") {
            std::cout << "Bye.\n";
            break;

        } else {
            std::cout << "Unknown command. Type HELP for usage.\n";
        }
    }

    return 0;
}