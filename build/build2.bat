@echo off

cd /d D:\Projects\TalkProject\Talk++\build

cls
set ESC=

echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[1;4;38;5;39;49mStarting build.
echo %ESC%[0;38;5;229;49m[INFO] first argument: %1%ESC%[0m
echo %ESC%[0;38;5;229;49m[INFO] batch version: v1.0

echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0mbuild.cpp compile start
g++ -std=c++23 -Wall -O2 build2.cpp -o build2.exe
if %ERRORLEVEL% EQU 0 (
    echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0mInitial Build Success, running build.cpp
    echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0mInitial Build Code: %ESC%[1;38;5;10;49m%ERRORLEVEL%
    .\build.exe %1
) else (
    echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0mInitial Build Failed 
    echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0mInitial Build Code: %ESC%[1;38;5;10;49m%ERRORLEVEL%
    echo %ESC%[0;38;5;1;49m[buildtpp] %ESC%[0;38;5;196;49mfatal error: build.cpp returned an error!
)