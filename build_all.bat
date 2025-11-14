@echo off
setlocal enabledelayedexpansion

:: ========================================
:: Advanced Driver + Mapper Builder
:: ========================================

color 0A
title Anti-Cheat Testing Tool Builder

echo.
echo ========================================
echo   ANTI-CHEAT TESTING TOOL BUILDER
echo ========================================
echo.

:: Check Python
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found! Install Python first.
    pause
    exit /b 1
)

:: ========================================
:: STEP 1: Generate Advanced Driver
:: ========================================

echo [1/6] Generating Advanced Driver...
echo.

python driver_generator_advanced.py TestDriver .\TestDriver < nul

if errorlevel 1 (
    echo [ERROR] Driver generation failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Driver generated in: TestDriver\driver\
echo.

:: ========================================
:: STEP 2: Build Driver (Manual)
:: ========================================

echo ========================================
echo [2/6] Building Driver (MANUAL STEP)
echo ========================================
echo.
echo The driver needs to be built with Visual Studio + WDK.
echo.
echo OPTION 1 - Visual Studio GUI:
echo   1. Open Visual Studio 2022
echo   2. File ^> New ^> Project ^> Empty Project
echo   3. Set Configuration to: Release, x64
echo   4. Project Properties:
echo      - Configuration Type: Driver
echo      - Platform Toolset: WindowsKernelModeDriver10.0
echo   5. Add files:
echo      - TestDriver\driver\driver.h
echo      - TestDriver\driver\driver.c
echo   6. Build ^> Build Solution (Ctrl+Shift+B)
echo   7. Output: x64\Release\driver.sys
echo.
echo OPTION 2 - WDK Command Line:
echo   1. Open "x64 Free Build Environment" from Start Menu
echo   2. cd to TestDriver\driver\
echo   3. Run: build
echo   4. Output: objfre_win7_amd64\amd64\driver.sys
echo.
echo ========================================
echo.

set /p built="Have you built the driver? (Y/N): "
if /i not "%built%"=="Y" (
    echo.
    echo [CANCELLED] Build the driver first, then run this script again.
    pause
    exit /b 0
)

set /p driverpath="Enter the full path to driver.sys: "

if not exist "%driverpath%" (
    echo [ERROR] Driver file not found: %driverpath%
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Driver located: %driverpath%
echo.

:: ========================================
:: STEP 3: Generate Mapper
:: ========================================

echo ========================================
echo [3/6] Generating Manual Mapper...
echo ========================================
echo.

python mapper_generator.py .\Mapper < nul

if errorlevel 1 (
    echo [ERROR] Mapper generation failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Mapper generated in: Mapper\
echo.

:: ========================================
:: STEP 4: Build Mapper
:: ========================================

echo ========================================
echo [4/6] Building Mapper...
echo ========================================
echo.

cd Mapper

if not exist build mkdir build
cd build

echo Running CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1

if errorlevel 1 (
    echo [WARNING] Visual Studio 2022 not found, trying 2019...
    cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1

    if errorlevel 1 (
        echo [ERROR] CMake configuration failed!
        echo Make sure Visual Studio 2019/2022 is installed.
        cd ..\..
        pause
        exit /b 1
    )
)

echo Building mapper...
cmake --build . --config Release

if errorlevel 1 (
    echo [ERROR] Mapper build failed!
    cd ..\..
    pause
    exit /b 1
)

cd ..\..

echo.
echo [SUCCESS] Mapper built successfully!
echo.

:: ========================================
:: STEP 5: Show File Paths
:: ========================================

echo ========================================
echo [5/6] BUILD COMPLETE!
echo ========================================
echo.

echo FILE LOCATIONS:
echo ---------------
echo.
echo [DRIVER]
echo   Path: %driverpath%
echo   Type: Kernel driver (.sys)
echo   Use: Load with mapper or test signing
echo.
echo [MAPPER]
for %%F in (Mapper\build\Release\mapper.exe) do (
    if exist "%%F" (
        echo   Path: %cd%\%%F
        echo   Type: Executable (.exe)
        echo   Use: Drag driver.sys onto mapper.exe
    ) else (
        echo   Path: NOT FOUND
        echo   Status: Build may have failed
    )
)
echo.

:: ========================================
:: STEP 6: Testing Instructions
:: ========================================

echo ========================================
echo [6/6] TESTING INSTRUCTIONS
echo ========================================
echo.
echo To test your anti-cheat:
echo.
echo 1. Enable your anti-cheat
echo.
echo 2. Run the mapper:
echo    Mapper\build\Release\mapper.exe %driverpath%
echo.
echo 3. Expected result (EAC-level):
echo    [Anti-Cheat] Vulnerable driver detected!
echo    [Anti-Cheat] Blocking...
echo    [Mapper] ERROR: Access denied
echo.
echo 4. If mapper succeeds:
echo    Your anti-cheat FAILED the test!
echo    Add detection for:
echo      - Vulnerable drivers (iqvw64e.sys)
echo      - Kernel memory operations
echo      - Manual mapping techniques
echo.

:: Save paths to file
echo [DRIVER] > build_output.txt
echo %driverpath% >> build_output.txt
echo. >> build_output.txt
echo [MAPPER] >> build_output.txt
echo %cd%\Mapper\build\Release\mapper.exe >> build_output.txt

echo Paths saved to: build_output.txt
echo.

echo ========================================
echo.

pause
