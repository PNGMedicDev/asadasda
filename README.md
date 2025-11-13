# Driver Generator Tool - Anti-Cheat Testing Framework

A comprehensive framework for generating Windows kernel drivers to test anti-cheat detection capabilities. This tool is designed exclusively for testing anti-cheat systems you own and develop.

## ⚠️ CRITICAL LEGAL NOTICE

**AUTHORIZED USE ONLY**

This tool is designed for:
- ✅ Testing anti-cheat systems **you own and develop**
- ✅ Educational purposes in **controlled environments**
- ✅ Security research with **proper authorization**
- ✅ Personal testing on **systems you own**

This tool is **NOT** for:
- ❌ Cheating in online games
- ❌ Bypassing security measures you don't own
- ❌ Unauthorized access to any system
- ❌ Any illegal or unethical activity

**By using this tool, you agree that you have authorization and are complying with all applicable laws and regulations.**

## Overview

The Driver Generator Tool automatically creates a complete Windows kernel driver project with:

### 🔧 Kernel Driver Features
- Custom read/write memory operations with CR0 bypass
- Process base address retrieval
- Function hooking/unhooking capabilities
- IOCTL-based communication interface
- String encryption/decryption system
- Randomized identifiers to test detection

### 🖥️ Usermode Interface Features
- ImGui-based graphical user interface
- Process enumeration and attachment
- Real-time memory manipulation
- Hook management system
- Direct driver communication

### 🏗️ Build System
- Automated build scripts
- CMake configuration
- Visual Studio project templates
- Cross-component integration

## Quick Start

### 1. Generate a Driver Project

```bash
python driver_generator.py MyTestDriver ./MyDriver
```

### 2. Navigate to Project

```bash
cd MyDriver
```

### 3. Review Documentation

- `README.md` - Full project documentation
- `QUICKSTART.md` - Fast-track guide (if generated)

### 4. Build (Windows + WDK Required)

```bash
python build.py
```

### 5. Test Your Anti-Cheat

Follow the testing workflow in the generated documentation.

## System Requirements

### Development Environment
- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2019/2022
- **SDK**: Windows 11 SDK (latest)
- **WDK**: Windows Driver Kit (matching SDK version)
- **Build Tools**: CMake 3.15+, Python 3.7+

### Runtime Environment
- **OS**: Windows 10/11 (64-bit)
- **Privileges**: Administrator rights
- **Mode**: Test signing enabled
- **Architecture**: x64

## Project Structure

```
driver_generator.py          # Main generator script
├── QUICKSTART.md            # Quick start guide
├── example_test.py          # Example usage and workflow
├── config_template.json     # Configuration reference
└── README.md                # This file

Generated Project Structure:
MyDriver/
├── driver/                  # Kernel driver source
│   ├── driver.h            # Driver header
│   ├── driver.c            # Driver implementation
│   └── driver.vcxproj      # Visual Studio project
├── usermode/               # Usermode application
│   ├── main.cpp           # GUI application
│   ├── driver_interface.h # Interface header
│   └── driver_interface.cpp # Interface implementation
├── common/                 # Shared definitions
├── build/                  # Build output
├── CMakeLists.txt         # Build configuration
├── build.py               # Auto-build script
├── config.json            # Project configuration
└── README.md              # Project documentation
```

## Usage Examples

### Basic Generation

```bash
# Generate with default options
python driver_generator.py TestDriver

# Generate to specific directory
python driver_generator.py TestDriver ./output

# Interactive generation
python driver_generator.py MyDriver
```

### View Example Workflow

```bash
python example_test.py
```

### Test Generation (Dry Run)

```bash
# Generate a test project to verify everything works
python driver_generator.py QuickTest ./test_output
cd test_output
cat README.md
```

## Features in Detail

### 🎯 Memory Operations

**Read Memory**
- Kernel-mode memory reading
- Process context switching
- Exception handling
- Multi-architecture support

**Write Memory**
- CR0 write protection bypass
- Safe memory writing
- Atomic operations
- Protected memory handling

### 🪝 Function Hooking

**Hook Installation**
- Inline hook creation
- Original byte preservation
- Jump instruction generation (x64/x86)
- Thread-safe modifications

**Hook Removal**
- Clean restoration
- Original code verification
- Memory protection handling

### 🔐 Security Features

**String Encryption**
- XOR-based obfuscation
- Runtime decryption
- Random key generation
- Static analysis evasion

**Randomization**
- Random pool tags
- Unique device names
- Variable naming obfuscation

### 📡 Communication

**IOCTL Interface**
- Buffered communication
- Request/response handling
- Error propagation
- Type-safe structures

## Anti-Cheat Testing Methodology

### Phase 1: Baseline Testing
1. Generate test driver
2. Build and load driver
3. Run on unprotected process (e.g., Notepad)
4. Verify all features work

### Phase 2: Protection Testing
1. Enable your anti-cheat
2. Attempt driver loading
3. Attempt memory operations
4. Attempt hook installation
5. Monitor anti-cheat response

### Phase 3: Detection Verification
Your anti-cheat should detect:
- ✓ Kernel driver loading
- ✓ Device object creation
- ✓ Unauthorized memory access
- ✓ CR0 manipulation
- ✓ Function hooks
- ✓ IOCTL communication

### Phase 4: Iteration
1. Analyze detection gaps
2. Improve anti-cheat
3. Re-test with driver
4. Document improvements
5. Repeat cycle

## What Your Anti-Cheat Should Detect

### Driver Level
- [ ] Unsigned driver loading
- [ ] Test signing mode detection
- [ ] Device object enumeration
- [ ] Driver signature verification
- [ ] Suspicious driver names
- [ ] Kernel module enumeration

### Memory Protection
- [ ] Unauthorized memory reads
- [ ] Unauthorized memory writes
- [ ] CR0 register manipulation
- [ ] Page table modifications
- [ ] Memory guard violations
- [ ] Process context switching

### Code Integrity
- [ ] Inline hooks (JMP instructions)
- [ ] IAT modifications
- [ ] Code section modifications
- [ ] Function prologue changes
- [ ] SSDT hooks
- [ ] Critical function tampering

### Communication Monitoring
- [ ] IOCTL requests to unknown drivers
- [ ] DeviceIoControl calls
- [ ] IRP request patterns
- [ ] Suspicious handle operations
- [ ] Cross-process communication

## Advanced Configuration

### Custom IOCTL Codes

Edit generated `driver.h`:
```c
#define IOCTL_CUSTOM_OPERATION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

### Additional Features

Modify `driver.c` to add:
- Module enumeration
- Registry operations
- File system operations
- Network communication
- Custom protection bypass techniques

### GUI Customization

The generated ImGui interface can be extended:
- Additional tabs/windows
- Memory scanning features
- Pattern searching
- Signature scanning
- Custom visualization

## Building the Generated Project

### Prerequisites Installation

1. **Visual Studio 2022**
   - Download from: https://visualstudio.microsoft.com/
   - Install: "Desktop development with C++"

2. **Windows 11 SDK**
   - Included with Visual Studio
   - Or download separately from Windows Dev Center

3. **Windows Driver Kit (WDK)**
   - Download from: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
   - Must match SDK version

4. **CMake**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

### Build Steps

```bash
# Navigate to generated project
cd MyDriver

# Run automated build
python build.py

# Or build manually:
# Usermode:
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Driver (requires WDK command prompt):
cd ../driver
msbuild driver.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Installation

```cmd
# Enable test signing (one-time, requires reboot)
bcdedit /set testsigning on
# Reboot now

# Create and start driver service
sc create MyTestDriver type= kernel binPath= C:\full\path\to\driver.sys
sc start MyTestDriver

# Verify driver loaded
sc query MyTestDriver
```

### Running

```cmd
cd build\Release
AntiCheatTester.exe MyTestDriver
```

## Troubleshooting

### Common Issues

**"Driver failed to load"**
- Solution: Enable test signing and reboot
- Check: `bcdedit /enum {current}` should show "testsigning Yes"
- Verify: Run as Administrator

**"Can't connect to driver"**
- Solution: Verify driver is running with `sc query DriverName`
- Check: Device object exists in `\\Device\\` namespace
- Verify: Usermode app running as Administrator

**"Memory operations fail"**
- Note: This may be expected if testing protection!
- Verify: Try on simple process (Notepad.exe) first
- Check: Process ID is correct
- Check: Address is valid

**Build errors**
- Verify: All prerequisites installed
- Check: Matching SDK and WDK versions
- Solution: Clean build directory and retry

### Debug Tips

1. **Use DebugView** (Sysinternals)
   - Shows kernel DbgPrint output
   - Monitor driver loading
   - See real-time debug messages

2. **Check Event Viewer**
   - Windows Logs → System
   - Look for driver errors
   - Review failure codes

3. **Test Incrementally**
   - Test driver load first
   - Then test connection
   - Then test each operation
   - Isolate failures

## Security Considerations

### Test Signing

- Only enable on test systems
- Never use on production systems
- Disable after testing
- Understand security implications

### Driver Loading

- Drivers run in kernel mode
- Full system access
- Can crash system if buggy
- Test in VM recommended

### Anti-Virus Warnings

- AV may flag kernel drivers
- This is expected behavior
- Add exceptions if needed
- Only for your test systems

## Contributing

This tool is for defensive security testing. Contributions should focus on:
- ✅ Improving testing capabilities
- ✅ Better detection techniques
- ✅ Documentation improvements
- ✅ Build system enhancements
- ✅ Additional testing features

Not accepted:
- ❌ Detection evasion techniques
- ❌ Anti-debugging features
- ❌ Rootkit capabilities
- ❌ Offensive-only features

## Legal & Ethical Use

### Acceptable Use
- Testing anti-cheat systems you develop
- Educational research in controlled environments
- Security research with authorization
- Personal learning on owned systems

### Unacceptable Use
- Cheating in any online game
- Bypassing any security system without authorization
- Distributing for malicious purposes
- Any illegal activity

### Disclaimer

This software is provided for educational and defensive security testing purposes only. The authors and contributors:
- Make no warranties about this software
- Are not responsible for any misuse
- Do not condone illegal or unethical use
- Provide this tool "as-is"

Users are solely responsible for ensuring their use complies with all applicable laws and regulations.

## Resources

### Documentation
- [Windows Driver Kit Documentation](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [Kernel-Mode Driver Architecture](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/)
- [ImGui Documentation](https://github.com/ocornut/imgui)

### Anti-Cheat Development
- Research kernel-mode protection techniques
- Study existing anti-cheat systems (authorized only)
- Join security research communities
- Attend security conferences

### Tools
- WinDbg (Kernel debugging)
- Sysinternals Suite (System monitoring)
- IDA Pro / Ghidra (Reverse engineering)
- OSR Driver Loader (Driver testing)

## License

MIT License

Copyright (c) 2025 Anti-Cheat Testing Framework

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Support

For issues related to:
- **Building**: Check prerequisites and build logs
- **Testing**: Review testing methodology documentation
- **Detection**: This is the goal! Improve your anti-cheat
- **Legal/Ethical**: Ensure you have proper authorization

## Acknowledgments

This tool was created to help developers build better anti-cheat systems by understanding what they need to detect. It serves as a reference implementation of techniques that should be blocked by robust anti-cheat solutions.

---

**Remember: The goal is to make anti-cheat systems better, not to bypass them!**

If you're using this tool correctly, your anti-cheat should detect and block it. If it doesn't, you have work to do! 🛡️
