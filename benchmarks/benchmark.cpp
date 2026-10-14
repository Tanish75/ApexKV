#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "hash_table.hpp"

int main() {
    std::cout << "========================================\n";
    std::cout << "       ApexKV Baseline Benchmark       \n";
    std::cout << "========================================\n";

    const int num_ops = 50000;
    HashTable ht;

    std::cout << "Pre-generating " << num_ops << " keys...\n";
    std::vector<std::string> keys;
    keys.reserve(num_ops);
    for (int i = 0; i < num_ops; ++i) {
        keys.push_back("key_" + std::to_string(i));
    }

    // Benchmark PUT operations
    std::cout << "Benchmarking " << num_ops << " PUT operations...\n";
    auto start_put = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_ops; ++i) {
        ht.put(keys[i], i);
    }
    auto end_put = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> put_duration = end_put - start_put;

    double put_ops_per_sec = (num_ops / put_duration.count()) * 1000.0;
    std::cout << "  - PUT time: " << put_duration.count() << " ms (" 
              << static_cast<long long>(put_ops_per_sec) << " ops/sec)\n";

    // Benchmark GET operations
    std::cout << "Benchmarking " << num_ops << " GET operations...\n";
    auto start_get = std::chrono::high_resolution_clock::now();
    int hits = 0;
    for (int i = 0; i < num_ops; ++i) {
        if (ht.contains(keys[i])) {
            hits++;
        }
    }
    auto end_get = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> get_duration = end_get - start_get;

    double get_ops_per_sec = (num_ops / get_duration.count()) * 1000.0;
    std::cout << "  - GET time: " << get_duration.count() << " ms (" 
              << static_cast<long long>(get_ops_per_sec) << " ops/sec, " 
              << hits << "/" << num_ops << " hits)\n";

    std::cout << "========================================\n";
    return 0;
}
