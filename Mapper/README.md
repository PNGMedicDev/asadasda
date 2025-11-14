# Advanced Manual Mapper - Anti-Cheat Testing Tool

## Purpose

This manual mapper tests if your EAC-level anti-cheat can detect advanced driver loading techniques that bypass Windows signature verification.

## ⚠️ AUTHORIZED USE ONLY

Test ONLY on your own anti-cheat systems!

## Features

### Exploitation Techniques:
- Vulnerable driver exploitation (Intel iqvw64e.sys style)
- Kernel memory read/write access
- Memory allocation in kernel space

### Manual Mapping:
- Full PE parsing and processing
- Relocation fixing
- Import resolution
- Section mapping
- Entry point calling

### Stealth Techniques:
- PiDDBCacheTable clearing
- MmUnloadedDrivers manipulation
- Hidden from module enumeration

## Building

### Requirements:
- CMake 3.15+
- Visual Studio 2019/2022
- Windows SDK

### Build Steps:
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Output:
```
build/mapper.exe
```

## Usage

### Drag and Drop:
1. Build mapper.exe
2. Drag your .sys driver onto mapper.exe
3. Follow prompts

### Command Line:
```cmd
mapper.exe driver.sys
```

## What Your Anti-Cheat Should Detect

### ✅ Detection Points:

1. **Vulnerable Driver Loading**
   - Detect iqvw64e.sys or similar vulnerable drivers
   - Block before exploitation occurs

2. **Kernel Memory Operations**
   - Monitor for suspicious memory read/write
   - Detect non-standard kernel memory access

3. **Non-Module Memory Allocation**
   - Scan for executable code in non-module memory
   - Detect kernel allocations from user-mode

4. **Manual PE Mapping**
   - Identify PE mapping patterns
   - Detect relocation processing

5. **PiDDBCache Manipulation**
   - Monitor PiDDBCacheTable for tampering
   - Detect missing driver entries

6. **Hidden Module Detection**
   - Cross-check multiple enumeration methods
   - Find drivers hidden from lists

7. **Callback Validation**
   - Verify all system callbacks
   - Check callback origins

8. **Behavioral Analysis**
   - Pattern matching for mapping techniques
   - Heuristic detection

## Expected Results

### ✅ EAC-Level Anti-Cheat:
```
[Anti-Cheat] Vulnerable driver detected: iqvw64e.sys
[Anti-Cheat] Driver loading blocked
[Anti-Cheat] Manual mapping attempt detected
[Anti-Cheat] Violation logged
[Anti-Cheat] User banned
```

### ❌ Weak Anti-Cheat:
```
[Mapper] Driver mapped successfully
[Mapper] Operating in kernel
[Anti-Cheat] No detection (FAILED TEST!)
```

## Testing Workflow

1. **Enable your anti-cheat**
2. **Run mapper on test driver**
3. **Verify detection**
4. **Check logs**
5. **If not detected → Improve anti-cheat**

## Technical Details

### Vulnerable Driver:
- Uses Intel iqvw64e.sys style exploit
- Gains kernel R/W through signed driver
- Exploits IOCTL interface

### Mapping Process:
1. Load vulnerable driver
2. Get kernel R/W access
3. Allocate kernel memory
4. Parse target driver PE
5. Process relocations
6. Resolve imports
7. Map sections to kernel
8. Call entry point
9. Apply stealth

### Detection Difficulty:
- **Very High** without proper anti-cheat
- **Should be Easy** with EAC-level protection

## Files

```
mapper/
├── src/
│   ├── main.cpp          # Main program + UI
│   ├── vuln_driver.cpp   # Vulnerable driver exploit
│   ├── pe_parser.cpp     # PE parsing/processing
│   ├── mapper.cpp        # Mapping engine
│   └── utils.cpp         # Utilities
├── include/
│   └── mapper.h          # Main header
├── vulnerable_drivers/   # Place iqvw64e.sys here
├── build/                # Build output
└── CMakeLists.txt        # Build configuration
```

## Legal Notice

**FOR TESTING YOUR OWN ANTI-CHEAT ONLY!**

This tool demonstrates techniques used by real cheats. Your anti-cheat must detect these to be effective.

If your anti-cheat cannot detect this mapper, it is NOT EAC-level!

## Summary

This mapper tests your anti-cheat's ability to detect:
- Vulnerable driver exploitation (BYOVD)
- Manual driver mapping
- Kernel memory manipulation
- Stealth techniques

**Goal:** Your anti-cheat should BLOCK this completely!

If it doesn't → Time to improve your detection! 🛡️
