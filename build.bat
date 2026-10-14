@echo off
REM ApexKV Windows Build Script
REM Builds CLI, Unit Tests, and Benchmark using g++ (C++17)

if not exist build mkdir build

echo [1/3] Compiling interactive CLI (apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include main.cpp -o build\apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\apexkv.exe

echo [2/3] Compiling Unit Test Runner (test_apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include tests\test_apexkv.cpp -o build\test_apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\test_apexkv.exe

echo [3/3] Compiling Benchmark Runner (benchmark_apexkv.exe)...
g++ -std=c++17 -Wall -Wextra -O2 -I include benchmarks\benchmark.cpp -o build\benchmark_apexkv.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile benchmark_apexkv.exe
    exit /b %ERRORLEVEL%
)
echo   - Built build\benchmark_apexkv.exe

echo.
echo [SUCCESS] Build complete!
echo   - Run CLI:       .\build\apexkv.exe
echo   - Run Tests:     .\build\test_apexkv.exe
echo   - Run Benchmark: .\build\benchmark_apexkv.exe
