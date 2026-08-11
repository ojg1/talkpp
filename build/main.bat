@echo off

cd /d "%~dp0"
cd ..

g++ -std=c++23 -Wall -O2 main.cpp -o build\Talk++

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    .\build\Talk++.exe
) else (
    echo Compilation failed
)

echo "compiler finished"