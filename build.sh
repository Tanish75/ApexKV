#!/usr/bin/env bash
set -e

# ApexKV Linux/macOS Build Script
# Builds CLI, All Test Suites, and Benchmark using g++ or clang++ (C++17)

mkdir -p build

CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -Wall -Wextra -O2 -I include -pthread"

echo "[1/6] Compiling interactive CLI (apexkv)..."
$CXX $CXXFLAGS main.cpp -o build/apexkv
echo "  - Built build/apexkv"

echo "[2/6] Compiling HashTable Tests (test_apexkv)..."
$CXX $CXXFLAGS tests/test_apexkv.cpp -o build/test_apexkv
echo "  - Built build/test_apexkv"

echo "[3/6] Compiling WAL Tests (test_wal)..."
$CXX $CXXFLAGS tests/test_wal.cpp -o build/test_wal
echo "  - Built build/test_wal"

echo "[4/6] Compiling TTL Tests (test_ttl)..."
$CXX $CXXFLAGS tests/test_ttl.cpp -o build/test_ttl
echo "  - Built build/test_ttl"

echo "[5/6] Compiling Thread Safety Tests (test_thread_safety)..."
$CXX $CXXFLAGS tests/test_thread_safety.cpp -o build/test_thread_safety
echo "  - Built build/test_thread_safety"

echo "[6/6] Compiling Benchmark Runner (benchmark_apexkv)..."
$CXX $CXXFLAGS benchmarks/benchmark.cpp -o build/benchmark_apexkv
echo "  - Built build/benchmark_apexkv"

echo ""
echo "[SUCCESS] Build complete!"
echo "  - Run CLI:           ./build/apexkv"
echo "  - Run Table Tests:   ./build/test_apexkv"
echo "  - Run WAL Tests:     ./build/test_wal"
echo "  - Run TTL Tests:     ./build/test_ttl"
echo "  - Run Thread Tests:  ./build/test_thread_safety"
echo "  - Run Benchmark:     ./build/benchmark_apexkv"
