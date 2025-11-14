# Windows Setup - Super Simple

## 🚀 One-Click Setup

### Step 1: Double-click this file
```
auto_build_everything.bat
```

### Step 2: Wait (30 seconds)
The script will:
- Generate advanced driver ✅
- Generate manual mapper ✅
- Build mapper.exe ✅
- Create run_mapper.bat ✅
- Show you everything ✅

### Step 3: Build Driver in Visual Studio
```
1. Open Visual Studio 2022
2. File → New → Project → Empty Project
3. Add files:
   - TestDriver\driver\driver.c
   - TestDriver\driver\driver.h
4. Configuration: Release, x64
5. Build → Build Solution (Ctrl+Shift+B)
6. Output: x64\Release\driver.sys
```

### Step 4: Test Your Anti-Cheat
```
1. Double-click: run_mapper.bat
2. Drag driver.sys when prompted
3. Watch the results!
```

---

## 📁 What You Get

```
After running auto_build_everything.bat:

TestDriver\
  └─ driver\
      ├─ driver.c          ← Build this in VS
      └─ driver.h

Mapper\
  └─ build\
      └─ Release\
          └─ mapper.exe    ← Ready to use!

run_mapper.bat              ← Helper script
BUILD_INFO.txt              ← All file paths
```

---

## ✅ Expected Results

### With EAC-Level Anti-Cheat (GOOD):
```
[Anti-Cheat] Vulnerable driver detected!
[Anti-Cheat] BLOCKED
[Mapper] ERROR: Access denied
```

### Without Anti-Cheat (or if it fails):
```
[Mapper] Driver mapped successfully
^^ Your anti-cheat FAILED! ❌
```

---

## 🎯 Quick Commands

### Full Auto Build:
```cmd
auto_build_everything.bat
```

### Just Test (after building driver):
```cmd
run_mapper.bat
```

### Manual Test:
```cmd
Mapper\build\Release\mapper.exe path\to\driver.sys
```

---

## 🔧 Prerequisites

Install these first (one-time):
1. **Python 3.11+** - Download from python.org
2. **Visual Studio 2022** - Community edition (free)
3. **Windows SDK** - Comes with VS
4. **WDK** - Windows Driver Kit
5. **CMake** - Download from cmake.org

---

## 📝 File Paths Reference

After successful build:

| File | Path | Description |
|------|------|-------------|
| **Driver Source** | `TestDriver\driver\driver.c` | Build in Visual Studio |
| **Driver Header** | `TestDriver\driver\driver.h` | Build in Visual Studio |
| **Mapper EXE** | `Mapper\build\Release\mapper.exe` | Ready to use |
| **Config** | `TestDriver\config.json` | Obfuscation map |
| **Helper** | `run_mapper.bat` | Auto-generated |
| **Info** | `BUILD_INFO.txt` | All paths |

---

## 💡 Pro Tips

### Rebuild Everything:
```cmd
del /s /q TestDriver
del /s /q Mapper
auto_build_everything.bat
```

### Check Files Exist:
```cmd
dir TestDriver\driver\driver.c
dir Mapper\build\Release\mapper.exe
```

### Quick Driver Build (if you have MSBuild):
```cmd
msbuild TestDriver\driver.vcxproj /p:Configuration=Release /p:Platform=x64
```

---

## 🎓 Testing Workflow

```
1. Run: auto_build_everything.bat
   ↓
2. Build driver in Visual Studio
   ↓
3. Enable YOUR anti-cheat
   ↓
4. Run: run_mapper.bat
   ↓
5. Check results:
   - Blocked? ✅ EAC-level!
   - Success? ❌ Needs work!
```

---

## ❓ Troubleshooting

### "Python not found"
```cmd
winget install Python.Python.3.11
```

### "CMake failed"
```cmd
winget install Kitware.CMake
```

### "Visual Studio not found"
Install VS 2022 Community from:
https://visualstudio.microsoft.com/downloads/

### "Driver won't build"
Make sure you installed:
- Visual Studio with C++
- Windows SDK
- Windows Driver Kit (WDK)

---

## 🏆 Success Criteria

Your anti-cheat is EAC-level when:

```
✅ Blocks vulnerable driver (iqvw64e.sys)
✅ Detects kernel memory operations
✅ Finds manually mapped driver
✅ Prevents driver loading
✅ Logs the violation
✅ Bans the user
```

If mapper succeeds → Not EAC-level yet!

---

## 🎯 Summary

**Easiest way:**
1. Double-click `auto_build_everything.bat`
2. Build driver in Visual Studio
3. Double-click `run_mapper.bat`
4. Verify anti-cheat blocks it

**Takes:** ~5 minutes total

**Goal:** Mapper should be BLOCKED by your anti-cheat!

---

That's it! Super simple. 🚀
