@echo off
REM ApexKV Windows Build Script
REM Builds CLI, All Test Suites, and Benchmark using g++ (C++17)

if not exist build mkdir build

echo [1/6] Compiling interactive CLI (apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include main.cpp -o build\apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\apexkv.exe

echo [2/6] Compiling HashTable Tests (test_apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include tests\test_apexkv.cpp -o build\test_apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\test_apexkv.exe

echo [3/6] Compiling WAL Tests (test_wal.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include tests\test_wal.cpp -o build\test_wal.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_wal.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\test_wal.exe

echo [4/6] Compiling TTL Tests (test_ttl.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include tests\test_ttl.cpp -o build\test_ttl.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_ttl.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\test_ttl.exe

echo [5/6] Compiling Thread Safety Tests (test_thread_safety.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include tests\test_thread_safety.cpp -o build\test_thread_safety.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_thread_safety.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\test_thread_safety.exe

echo [6/6] Compiling Benchmark Runner (benchmark_apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include benchmarks\benchmark.cpp -o build\benchmark_apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile benchmark_apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\benchmark_apexkv.exe

echo.
echo [SUCCESS] Build complete!
echo   - Run CLI:           .\build\apexkv.exe
echo   - Run Table Tests:   .\build\test_apexkv.exe
echo   - Run WAL Tests:     .\build\test_wal.exe
echo   - Run TTL Tests:     .\build\test_ttl.exe
echo   - Run Thread Tests:  .\build\test_thread_safety.exe
echo   - Run Benchmark:     .\build\benchmark_apexkv.exe
