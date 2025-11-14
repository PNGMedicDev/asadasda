@echo off
:: SUPER SIMPLE VERSION - Just generates everything
title Quick Builder

echo.
echo Generating driver...
echo yes | python driver_generator_advanced.py TestDriver .\TestDriver

echo.
echo Generating mapper...
echo yes | python mapper_generator.py .\Mapper

echo.
echo Building mapper...
cd Mapper
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cd ..\..

echo.
echo ========================================
echo DONE! Files generated:
echo ========================================
echo.
echo DRIVER SOURCE:
echo   TestDriver\driver\driver.c
echo   TestDriver\driver\driver.h
echo.
echo MAPPER EXECUTABLE:
echo   Mapper\build\Release\mapper.exe
echo.
echo NEXT STEPS:
echo   1. Build the driver in Visual Studio
echo   2. Run: Mapper\build\Release\mapper.exe path\to\driver.sys
echo.
pause
