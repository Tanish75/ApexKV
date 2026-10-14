#!/usr/bin/env bash
set -e

# ApexKV Linux/macOS Build Script
# Builds CLI, Unit Tests, and Benchmark using g++ or clang++ (C++17)

mkdir -p build

CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -Wall -Wextra -O2 -I include"

echo "[1/3] Compiling interactive CLI (apexkv)..."
$CXX $CXXFLAGS main.cpp -o build/apexkv
echo "  - Built build/apexkv"

echo "[2/3] Compiling Unit Test Runner (test_apexkv)..."
$CXX $CXXFLAGS tests/test_apexkv.cpp -o build/test_apexkv
echo "  - Built build/test_apexkv"

echo "[3/3] Compiling Benchmark Runner (benchmark_apexkv)..."
$CXX $CXXFLAGS benchmarks/benchmark.cpp -o build/benchmark_apexkv
echo "  - Built build/benchmark_apexkv"

echo ""
echo "[SUCCESS] Build complete!"
echo "  - Run CLI:       ./build/apexkv"
echo "  - Run Tests:     ./build/test_apexkv"
echo "  - Run Benchmark: ./build/benchmark_apexkv"
