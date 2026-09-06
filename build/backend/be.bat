@echo off

cd /d %CD%

set ESC=
echo %ESC%[2J%ESC%%[H"

g++ -std=c++23 -O2 be.cpp -o be.exe
.\be.exe