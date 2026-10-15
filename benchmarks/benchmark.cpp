#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <iomanip>
#include "hash_table.hpp"

// ---------------------------------------------------------------------------
// ApexKV vs std::unordered_map Comparative Benchmark Harness
// ---------------------------------------------------------------------------
int main() {
    std::cout << "========================================================\n"
              << "       ApexKV vs std::unordered_map Benchmark Suite      \n"
              << "========================================================\n";

    const int num_ops = 50000;
    const int repetitions = 5;

    std::cout << "Pre-generating " << num_ops << " unique string keys...\n";
    std::vector<std::string> keys;
    keys.reserve(num_ops);
    for (int i = 0; i < num_ops; ++i) {
        keys.push_back("user_key_prefix_" + std::to_string(i));
    }

    std::vector<double> apexkv_put_ops, apexkv_get_ops;
    std::vector<double> stdmap_put_ops, stdmap_get_ops;

    for (int r = 1; r <= repetitions; ++r) {
        std::cout << "\n--- Repetition " << r << "/" << repetitions << " ---\n";

        // 1. ApexKV HashTable
        {
            HashTable<std::string, int> ht;
            auto start_put = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_ops; ++i) {
                ht.put(keys[i], i);
            }
            auto end_put = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> put_ms = end_put - start_put;
            double put_ops = (num_ops / put_ms.count()) * 1000.0;
            apexkv_put_ops.push_back(put_ops);

            auto start_get = std::chrono::high_resolution_clock::now();
            int hits = 0;
            for (int i = 0; i < num_ops; ++i) {
                if (ht.contains(keys[i])) hits++;
            }
            auto end_get = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> get_ms = end_get - start_get;
            double get_ops = (num_ops / get_ms.count()) * 1000.0;
            apexkv_get_ops.push_back(get_ops);

            std::cout << "  ApexKV:        PUT " << std::fixed << std::setprecision(2) << put_ms.count() << " ms ("
                      << static_cast<long long>(put_ops) << " ops/sec) | GET "
                      << get_ms.count() << " ms (" << static_cast<long long>(get_ops) << " ops/sec, hits=" << hits << ")\n";
        }

        // 2. std::unordered_map
        {
            std::unordered_map<std::string, int> map;
            auto start_put = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_ops; ++i) {
                map[keys[i]] = i;
            }
            auto end_put = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> put_ms = end_put - start_put;
            double put_ops = (num_ops / put_ms.count()) * 1000.0;
            stdmap_put_ops.push_back(put_ops);

            auto start_get = std::chrono::high_resolution_clock::now();
            int hits = 0;
            for (int i = 0; i < num_ops; ++i) {
                if (map.find(keys[i]) != map.end()) hits++;
            }
            auto end_get = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> get_ms = end_get - start_get;
            double get_ops = (num_ops / get_ms.count()) * 1000.0;
            stdmap_get_ops.push_back(get_ops);

            std::cout << "  std::unordered_map: PUT " << std::fixed << std::setprecision(2) << put_ms.count() << " ms ("
                      << static_cast<long long>(put_ops) << " ops/sec) | GET "
                      << get_ms.count() << " ms (" << static_cast<long long>(get_ops) << " ops/sec, hits=" << hits << ")\n";
        }
    }

    auto avg = [](const std::vector<double>& v) {
        return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };

    double avg_apex_put = avg(apexkv_put_ops);
    double avg_apex_get = avg(apexkv_get_ops);
    double avg_std_put  = avg(stdmap_put_ops);
    double avg_std_get  = avg(stdmap_get_ops);

    std::cout << "\n========================================================\n"
              << "                      FINAL RESULTS                     \n"
              << "========================================================\n"
              << "Operation | ApexKV (avg ops/sec) | std::map (avg ops/sec) | Ratio (Apex / std)\n"
              << "----------|----------------------|------------------------|-------------------\n"
              << "PUT       | " << std::setw(20) << static_cast<long long>(avg_apex_put)
              << " | " << std::setw(22) << static_cast<long long>(avg_std_put)
              << " | " << std::setw(15) << std::setprecision(2) << (avg_apex_put / avg_std_put) << "x\n"
              << "GET       | " << std::setw(20) << static_cast<long long>(avg_apex_get)
              << " | " << std::setw(22) << static_cast<long long>(avg_std_get)
              << " | " << std::setw(15) << std::setprecision(2) << (avg_apex_get / avg_std_get) << "x\n"
              << "========================================================\n";

    return 0;
}
