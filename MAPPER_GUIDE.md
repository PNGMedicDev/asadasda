# Manual Mapper Testing Guide

## Overview

The **Advanced Manual Mapper** is a sophisticated tool that tests your EAC-level anti-cheat's ability to detect driver manual mapping - one of the most advanced cheating techniques.

## 🎯 What is Manual Mapping?

**Manual Mapping** bypasses Windows driver signature verification by:
1. Exploiting a signed vulnerable driver
2. Gaining kernel read/write access
3. Manually loading unsigned driver into kernel memory
4. Hiding from normal driver enumeration

**Your anti-cheat MUST detect this!**

---

## 🚀 Quick Start

### Step 1: Generate Mapper
```bash
python3 mapper_generator.py ./mapper
```

### Step 2: Build (Windows Required)
```cmd
cd mapper
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Step 3: Test
```cmd
# Generate a test driver first
cd ../../
python3 driver_generator_advanced.py TestDriver ./test_driver
cd test_driver/driver
# Build driver...

# Test with mapper
cd ../../mapper/build
mapper.exe ../../test_driver/driver/driver.sys
```

---

## 🛠️ How It Works

### Phase 1: Vulnerable Driver Exploitation
```
1. Load signed vulnerable driver (iqvw64e.sys)
2. Exploit IOCTL interface
3. Gain kernel read/write access
4. Can now access ALL memory
```

**Detection Point:**
- Your anti-cheat should have a vulnerable driver blacklist
- Block iqvw64e.sys and 200+ other vulnerable drivers
- Detect before exploitation occurs

### Phase 2: PE Parsing
```
1. Parse target driver's PE structure
2. Read sections, headers, imports
3. Calculate relocations
4. Prepare for kernel mapping
```

**Detection Point:**
- Monitor for PE parsing patterns
- Detect suspicious usermode activity

### Phase 3: Manual Mapping
```
1. Allocate kernel memory (non-module)
2. Map PE headers
3. Map sections (.text, .data, etc.)
4. Process relocations for new base
5. Resolve kernel imports
6. Write everything to kernel memory
```

**Detection Point:**
- Scan for code in non-module memory
- Detect executable pages not from loaded modules
- Monitor kernel memory allocations from usermode

### Phase 4: Entry Point Call
```
1. Call DriverEntry from mapped location
2. Driver initializes in kernel
3. Driver operates with full kernel privileges
```

**Detection Point:**
- Monitor for DriverEntry calls from unknown locations
- Validate all driver initializations

### Phase 5: Stealth Techniques
```
1. Clear PiDDBCacheTable entries
2. Remove from MmUnloadedDrivers
3. Hide from module enumeration
4. Operate invisibly
```

**Detection Point:**
- Monitor PiDDBCacheTable for tampering
- Detect MmUnloadedDrivers manipulation
- Cross-check multiple enumeration methods

---

## 🔍 Detection Methods

### 1. Vulnerable Driver Blacklist

**Implementation:**
```cpp
const char* vulnerableDrivers[] = {
    "iqvw64e.sys",      // Intel
    "capcom.sys",       // Capcom
    "dbutil_2_3.sys",   // Dell
    "msio64.sys",       // MSI
    "AsUpIO.sys",       // ASUS
    // ... 200+ more
};

bool IsVulnerableDriver(const char* driverName) {
    for (auto vuln : vulnerableDrivers) {
        if (strcmp(driverName, vuln) == 0) {
            LOG_VIOLATION("Vulnerable driver detected!");
            return true;
        }
    }
    return false;
}
```

### 2. Memory Integrity Scanning

**Implementation:**
```cpp
void ScanKernelMemory() {
    // Get all loaded modules
    auto modules = EnumerateKernelModules();

    // Scan kernel memory
    for (UINT64 addr = kernelBase; addr < kernelEnd; addr += PAGE_SIZE) {
        if (IsExecutable(addr)) {
            bool fromModule = false;

            // Check if from loaded module
            for (auto& mod : modules) {
                if (addr >= mod.base && addr < mod.base + mod.size) {
                    fromModule = true;
                    break;
                }
            }

            if (!fromModule) {
                LOG_VIOLATION("Code in non-module memory!");
                // Found manually mapped driver
            }
        }
    }
}
```

### 3. Hidden Module Detection

**Implementation:**
```cpp
void DetectHiddenModules() {
    // Method 1: PsLoadedModuleList
    auto list1 = EnumerateViaPsLoadedModuleList();

    // Method 2: AuxKlibQueryModuleInformation
    auto list2 = EnumerateViaAuxKlib();

    // Method 3: ZwQuerySystemInformation
    auto list3 = EnumerateViaZwQuery();

    // Compare
    if (list1 != list2 || list2 != list3) {
        LOG_VIOLATION("Hidden driver detected!");
        // Driver is hiding from enumeration
    }
}
```

### 4. Callback Validation

**Implementation:**
```cpp
void ValidateCallbacks() {
    // Get all notify callbacks
    auto callbacks = EnumerateNotifyCallbacks();

    for (auto& callback : callbacks) {
        // Check if callback is from known module
        bool fromKnownModule = false;

        for (auto& mod : loadedModules) {
            if (callback.address >= mod.base &&
                callback.address < mod.base + mod.size) {
                fromKnownModule = true;
                break;
            }
        }

        if (!fromKnownModule) {
            LOG_VIOLATION("Callback from unknown module!");
            // Manually mapped driver registered callback
        }
    }
}
```

### 5. PiDDBCache Monitoring

**Implementation:**
```cpp
void MonitorPiDDBCache() {
    // Get PiDDBCacheTable
    auto table = GetPiDDBCacheTable();
    auto originalHash = CalculateHash(table);

    // Periodically check
    while (running) {
        auto currentHash = CalculateHash(table);

        if (currentHash != originalHash) {
            LOG_VIOLATION("PiDDBCache tampered!");
            // Someone cleared traces
        }

        Sleep(1000);
    }
}
```

---

## 📊 Testing Workflow

### 1. Baseline Test
```bash
# Without anti-cheat
mapper.exe driver.sys
# Should succeed (no protection)
```

### 2. Anti-Cheat Test
```bash
# With your anti-cheat running
mapper.exe driver.sys
# Should be BLOCKED
```

### 3. Verify Detection
Check your anti-cheat logs for:
```
[Anti-Cheat] Vulnerable driver detected: iqvw64e.sys
[Anti-Cheat] Driver loading blocked
[Anti-Cheat] Manual mapping attempt detected
[Anti-Cheat] User flagged
```

### 4. Results

**✅ Success (EAC-Level):**
```
Mapper: ERROR - Driver loading blocked
Mapper: ERROR - Access denied
Anti-Cheat: Detection logged
Anti-Cheat: User banned
```

**❌ Failure (Needs Improvement):**
```
Mapper: Driver mapped successfully
Mapper: Operating in kernel
Anti-Cheat: No detection (BAD!)
```

---

## 🎯 Detection Checklist

Your anti-cheat should detect **ALL** of these:

| Detection Point | Status | Method |
|----------------|--------|--------|
| Vulnerable driver loading | [ ] | Blacklist check |
| Kernel R/W access | [ ] | Monitor IOCTLs |
| Non-module memory alloc | [ ] | Memory scanning |
| Manual PE mapping | [ ] | Pattern detection |
| Import resolution | [ ] | Behavioral analysis |
| Driver entry call | [ ] | Callback validation |
| PiDDBCache clearing | [ ] | Integrity monitoring |
| MmUnloadedDrivers tamper | [ ] | List validation |
| Hidden module | [ ] | Cross-enumeration |

**All must be ✅ for EAC-level protection!**

---

## 🔧 Building the Mapper

### Requirements
- Windows 10/11
- Visual Studio 2019/2022
- CMake 3.15+
- Windows SDK

### Build Commands
```cmd
cd mapper
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Output
```
build/mapper.exe
```

---

## 💻 Usage Examples

### Drag and Drop
1. Open File Explorer
2. Find mapper.exe
3. Drag driver.sys onto mapper.exe
4. Follow prompts

### Command Line
```cmd
# Basic usage
mapper.exe driver.sys

# With full path
mapper.exe C:\path\to\driver.sys

# Test mapping
mapper.exe test_driver.sys
```

### Console Output
```
╔═══════════════════════════════════════════════════════════════╗
║         ADVANCED MANUAL MAPPER - Anti-Cheat Tester           ║
╚═══════════════════════════════════════════════════════════════╝

[+] Driver file: test_driver.sys
[+] Loading vulnerable driver for kernel access...
[✓] Vulnerable driver loaded - kernel R/W access acquired!
[!] Your anti-cheat should have detected this!

[+] Parsing PE file...
[✓] Image base: 0x140000000
[✓] Image size: 0x5000
[✓] Entry point: 0x1000

[+] Allocating kernel memory for driver...
[✓] Driver allocated at kernel address: 0xFFFFF80012340000
[!] Your anti-cheat should detect non-module memory allocation!

[+] Processing relocations for new base 0xFFFFF80012340000
[✓] Processed 42 relocations

[+] Resolving kernel imports...
[✓] Resolved imports from 3 modules

[+] Mapping sections to kernel...
[✓] Driver successfully mapped to kernel memory!

[+] Calling driver entry point...
[✓] Driver entry point executed

[+] Applying stealth techniques...
[✓] PiDDBCache cleared
[✓] MmUnloadedDrivers cleared

╔═══════════════════════════════════════════════════════════════╗
║   DRIVER SUCCESSFULLY LOADED VIA MANUAL MAPPING!              ║
╚═══════════════════════════════════════════════════════════════╝

[!] If your EAC-level anti-cheat didn't detect this:
  1. Add vulnerable driver blacklist
  2. Implement memory integrity scanning
  3. Detect code in non-module memory
  4. Monitor for PiDDBCache manipulation
  5. Validate system callback origins
```

---

## ⚠️ Important Notes

### For Testers
- **Only use on systems you own**
- **Only test your own anti-cheat**
- **This is NOT for bypassing others' protection**
- **Legal testing only**

### For Anti-Cheat Developers
- **This shows what you must detect**
- **Study each technique carefully**
- **Implement all detection methods**
- **Test regularly**

### Expected Behavior
- **Mapper should be BLOCKED**
- **Driver should NOT load**
- **User should be flagged**
- **Logs should show detection**

---

## 🏆 EAC-Level Criteria

Your anti-cheat is **EAC-level** when:

1. ✅ **Blocks vulnerable driver loading**
2. ✅ **Detects kernel R/W attempts**
3. ✅ **Finds code in non-module memory**
4. ✅ **Catches manual mapping process**
5. ✅ **Detects PiDDBCache tampering**
6. ✅ **Finds hidden modules**
7. ✅ **Validates all callbacks**
8. ✅ **Logs comprehensive data**

If **ANY** check fails → Not EAC-level yet!

---

## 📚 Additional Resources

### Techniques Used
- **BYOVD** (Bring Your Own Vulnerable Driver)
- **Manual PE Mapping**
- **Kernel Memory Manipulation**
- **Driver List Hiding**
- **Trace Clearing**

### What to Study
- Windows driver loading process
- PE file format
- Kernel memory management
- Driver enumeration methods
- Anti-cheat detection techniques

### Related Tools
- KDMapper (public mapper)
- drvmap
- manual-map
- kdmap

**Your mapper is MORE sophisticated than these!**

---

## 🎓 Learning Path

### Phase 1: Understanding
1. Study how Windows loads drivers
2. Learn PE file format
3. Understand kernel memory

### Phase 2: Detection
1. Implement vulnerable driver blacklist
2. Add memory integrity scanning
3. Create hidden module detection

### Phase 3: Testing
1. Test with this mapper
2. Verify all detections work
3. Fix any gaps

### Phase 4: Maintenance
1. Update vulnerable driver list
2. Add new detection methods
3. Keep improving

---

## Summary

**Advanced Manual Mapper** = Ultimate test for your anti-cheat

**Features:**
- Vulnerable driver exploitation
- Manual PE mapping
- Stealth techniques
- Professional quality

**Goal:**
Your anti-cheat should **DETECT and BLOCK** this completely!

**If it doesn't:**
You're not EAC-level yet - keep improving! 🛡️

---

**Remember:** This tool is for testing YOUR anti-cheat. If it successfully loads drivers on your system, your anti-cheat failed the test!
