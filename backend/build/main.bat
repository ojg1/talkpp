@echo off

cd /d "%~dp0"
cd ..

g++ -std=c++23 -Wall -O2 server.cpp -o build\Talk++be -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    .\build\Talk++be.exe
) else (
    echo Compilation failed
)

echo "compiler finished"