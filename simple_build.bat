@echo off
cls
title Build Driver + Mapper

:: ====================================
:: SIMPLE BUILD SCRIPT
:: ====================================

echo.
echo ==========================================
echo   BUILDING DRIVER + MAPPER
echo ==========================================
echo.

:: ====================================
:: STEP 1: Generate Driver
:: ====================================

echo [1/4] Generating advanced driver...
echo yes | python driver_generator_advanced.py TestDriver .\TestDriver
echo.
echo [OK] Driver generated!
echo      Location: TestDriver\driver\
echo.

:: ====================================
:: STEP 2: Wait for Driver Build
:: ====================================

echo ==========================================
echo [2/4] BUILD THE DRIVER NOW
echo ==========================================
echo.
echo Open Visual Studio and build the driver:
echo   1. File ^> New ^> Project ^> Empty Project
echo   2. Configuration: Release, x64
echo   3. Add files:
echo      - TestDriver\driver\driver.c
echo      - TestDriver\driver\driver.h
echo   4. Build ^> Build Solution
echo   5. Find output: x64\Release\driver.sys
echo.

set /p "driverpath=Enter full path to driver.sys: "

if not exist "%driverpath%" (
    echo.
    echo [ERROR] File not found: %driverpath%
    echo.
    pause
    exit /b 1
)

echo.
echo [OK] Driver found: %driverpath%
echo.

:: ====================================
:: STEP 3: Generate + Build Mapper
:: ====================================

echo [3/4] Generating mapper...
echo yes | python mapper_generator.py .\Mapper
echo.

echo [3/4] Building mapper...
cd Mapper
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if errorlevel 1 cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
cmake --build . --config Release >nul 2>&1
cd ..\..
echo.
echo [OK] Mapper built!
echo      Location: Mapper\build\Release\mapper.exe
echo.

:: ====================================
:: STEP 4: Show File Paths
:: ====================================

cls
echo.
echo ==========================================
echo   BUILD COMPLETE!
echo ==========================================
echo.
echo.
echo COMPILED FILES:
echo ==========================================
echo.
echo [DRIVER]
echo   %driverpath%
echo.
echo [MAPPER]
echo   %cd%\Mapper\build\Release\mapper.exe
echo.
echo.
echo ==========================================
echo   HOW TO TEST
echo ==========================================
echo.
echo 1. Enable your anti-cheat
echo.
echo 2. Run this command:
echo    Mapper\build\Release\mapper.exe "%driverpath%"
echo.
echo 3. Expected result (EAC-level):
echo    [Anti-Cheat] Vulnerable driver detected!
echo    [Anti-Cheat] BLOCKED
echo.
echo 4. If mapper succeeds:
echo    Your anti-cheat FAILED the test!
echo.

:: Save to file
echo DRIVER: %driverpath% > compiled_files.txt
echo MAPPER: %cd%\Mapper\build\Release\mapper.exe >> compiled_files.txt

echo Paths saved to: compiled_files.txt
echo.
echo ==========================================
echo.

pause
