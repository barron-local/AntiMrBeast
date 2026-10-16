@echo off
setlocal

cd /d "%~dp0"

where g++ >nul 2>&1
if %errorlevel% equ 0 (
    g++ -std=c++17 -O2 main.cpp -o AntiMrBeast.exe
    if %errorlevel% equ 0 (
        echo Built: AntiMrBeast.exe
        exit /b 0
    )
    exit /b 1
)

where cl >nul 2>&1
if %errorlevel% equ 0 (
    cl /nologo /EHsc /std:c++17 /O2 main.cpp /Fe:AntiMrBeast.exe /link /SUBSYSTEM:CONSOLE
    if %errorlevel% equ 0 (
        echo Built: AntiMrBeast.exe
        exit /b 0
    )
    exit /b 1
)

echo No suitable C++ compiler (g++ or cl) found.
exit /b 1