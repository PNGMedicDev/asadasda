@echo off
setlocal

:: ========================================
:: AUTOMATIC BUILD - Everything
:: ========================================

cls
echo.
echo ================================================
echo    AUTOMATIC ANTI-CHEAT TESTING TOOL BUILDER
echo ================================================
echo.

:: ========================================
:: Generate Driver
:: ========================================

echo [STEP 1/3] Generating Advanced Driver...
echo.
echo yes | python driver_generator_advanced.py TestDriver .\TestDriver >nul 2>&1
echo [OK] Driver generated: TestDriver\driver\
echo.

:: ========================================
:: Generate Mapper
:: ========================================

echo [STEP 2/3] Generating Manual Mapper...
echo.
echo yes | python mapper_generator.py .\Mapper >nul 2>&1
echo [OK] Mapper generated: Mapper\
echo.

:: ========================================
:: Build Mapper
:: ========================================

echo [STEP 3/3] Building Mapper...
echo.
cd Mapper
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if errorlevel 1 cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
cmake --build . --config Release >nul 2>&1
cd ..\..
echo [OK] Mapper built: Mapper\build\Release\mapper.exe
echo.

:: ========================================
:: Create easy run script
:: ========================================

echo @echo off > run_mapper.bat
echo echo Drag your driver.sys file here and press Enter: >> run_mapper.bat
echo set /p driverpath= >> run_mapper.bat
echo Mapper\build\Release\mapper.exe "%%driverpath%%" >> run_mapper.bat
echo pause >> run_mapper.bat

:: ========================================
:: Output Summary
:: ========================================

cls
echo.
echo ================================================
echo              BUILD COMPLETE!
echo ================================================
echo.
echo GENERATED FILES:
echo ================================================
echo.

echo [DRIVER - C Source Code]
echo   Location: TestDriver\driver\
echo   Files:
echo     - driver.h (header)
echo     - driver.c (source)
echo   Status: NEEDS COMPILATION
echo.

echo [MAPPER - Ready to Use]
echo   Location: Mapper\build\Release\
echo   File: mapper.exe
echo   Status: READY
echo.

echo ================================================
echo              NEXT STEPS
echo ================================================
echo.

echo TO BUILD THE DRIVER:
echo -------------------
echo.
echo Method 1 - Visual Studio (Easiest):
echo   1. Open Visual Studio 2022
echo   2. Create new "Empty Project"
echo   3. Add TestDriver\driver\driver.c and driver.h
echo   4. Set to Release x64
echo   5. Build (Ctrl+Shift+B)
echo   6. Output: x64\Release\driver.sys
echo.
echo Method 2 - MSBuild (Command Line):
echo   msbuild TestDriver\driver.vcxproj /p:Configuration=Release /p:Platform=x64
echo.

echo ================================================
echo              TESTING
echo ================================================
echo.

echo After building driver.sys:
echo.
echo OPTION 1 - Drag and Drop:
echo   1. Run: run_mapper.bat
echo   2. Drag driver.sys when prompted
echo.
echo OPTION 2 - Command Line:
echo   Mapper\build\Release\mapper.exe path\to\driver.sys
echo.

echo ================================================
echo         EXPECTED RESULTS (EAC-LEVEL)
echo ================================================
echo.
echo With anti-cheat enabled:
echo   [Anti-Cheat] Vulnerable driver detected!
echo   [Anti-Cheat] BLOCKED
echo   [Mapper] ERROR: Access denied
echo.
echo Without anti-cheat (or if it fails):
echo   [Mapper] Driver mapped successfully
echo   ^^ This means your anti-cheat needs work!
echo.

echo ================================================
echo.

:: Create helpful readme
echo BUILD COMPLETE > BUILD_INFO.txt
echo. >> BUILD_INFO.txt
echo DRIVER SOURCE: >> BUILD_INFO.txt
echo   %cd%\TestDriver\driver\driver.c >> BUILD_INFO.txt
echo   %cd%\TestDriver\driver\driver.h >> BUILD_INFO.txt
echo. >> BUILD_INFO.txt
echo MAPPER EXECUTABLE: >> BUILD_INFO.txt
echo   %cd%\Mapper\build\Release\mapper.exe >> BUILD_INFO.txt
echo. >> BUILD_INFO.txt
echo BUILD THE DRIVER IN VISUAL STUDIO >> BUILD_INFO.txt
echo THEN RUN: run_mapper.bat >> BUILD_INFO.txt

echo File locations saved to: BUILD_INFO.txt
echo.

pause
