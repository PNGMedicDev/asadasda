# Quick Start Guide - Driver Generator Tool

## Fast Track to Testing Your Anti-Cheat

### Step 1: Generate Your Driver Project (2 minutes)

```bash
python driver_generator.py MyAntiCheatTest ./MyDriver
```

When prompted, type `yes` to confirm.

### Step 2: What You Get

```
MyDriver/
├── driver/           # Kernel driver (detects memory manipulation)
├── usermode/         # GUI app (performs the manipulation)
├── README.md         # Full documentation
└── build.py          # Automated build script
```

### Step 3: Build the Tool (Windows Only)

**Prerequisites:**
- Windows 10/11
- Visual Studio 2019/2022
- Windows Driver Kit (WDK)
- CMake

**Build:**
```bash
cd MyDriver
python build.py
```

### Step 4: Install & Run

**Enable Test Signing** (one-time, requires reboot):
```cmd
bcdedit /set testsigning on
```
Reboot your system.

**Load the Driver:**
```cmd
sc create MyAntiCheatTest type= kernel binPath= C:\full\path\to\driver.sys
sc start MyAntiCheatTest
```

**Run the GUI:**
```cmd
cd build\Release
AntiCheatTester.exe MyAntiCheatTest
```

### Step 5: Test Your Anti-Cheat

Now your anti-cheat should detect:

1. **The kernel driver** loading
2. **Memory manipulation** attempts
3. **Function hooks** being installed
4. **Unauthorized process access**

If your anti-cheat **doesn't detect this**, it needs improvement!

## What This Tool Tests

### ✅ Detection Categories

| Feature | Your Anti-Cheat Should Detect |
|---------|------------------------------|
| Kernel Driver | Unsigned driver loading, device object creation |
| Memory Read | Unauthorized memory access via kernel |
| Memory Write | Memory modifications, CR0 bypass |
| Function Hooks | Inline hooks, code modifications |
| Process Attach | Cross-process handle opening |

### Example Test Scenario

1. **Start your game** with anti-cheat enabled
2. **Run this tool** and attach to game process
3. **Attempt to read memory** (e.g., player health)
4. **Your anti-cheat should:**
   - Detect the driver
   - Block the memory access
   - Flag/ban the attempt
   - Log the violation

## GUI Usage

```
┌─────────────────────────────────────┐
│ Anti-Cheat Testing Tool             │
├─────────────────────────────────────┤
│ Driver Status: Connected            │
│                                     │
│ Target Process: game.exe            │
│ [Attach to Process] [Detach]        │
│                                     │
│ Process ID: 1234                    │
│ Base Address: 0x7FF600000000        │
├─────────────────────────────────────┤
│ Memory Operations                   │
│                                     │
│ Read Address: 0x12345678            │
│ [Read Memory]                       │
│ Result: Value: 0x00000064 (100)     │
│                                     │
│ Write Address: 0x12345678           │
│ Write Value: 999                    │
│ [Write Memory]                      │
├─────────────────────────────────────┤
│ Function Hooking                    │
│                                     │
│ Hook Target: 0x12345678             │
│ Hook Destination: 0x87654321        │
│ [Install Hook] [Remove Hook]        │
│                                     │
│ Hook Status: Not Installed          │
└─────────────────────────────────────┘
```

## Troubleshooting

### "Driver failed to load"
- Run as Administrator
- Check test signing: `bcdedit /enum {current}`
- Look for "testsigning Yes"

### "Can't connect to driver"
- Verify driver is running: `sc query MyAntiCheatTest`
- Check logs: Event Viewer → Windows Logs → System

### "Memory operations fail"
- Normal! This means protection is working
- Try on a simple Notepad.exe first
- Some processes have strong protection

## Important Reminders

### ⚠️ Legal Use Only

- Test ONLY your own games/software
- Use ONLY on systems you own
- This is for **defensive** security testing
- Misuse can result in legal consequences

### 🎯 Testing Methodology

1. **Baseline Test**: Run against unprotected process (Notepad)
2. **Protected Test**: Run against your anti-cheat
3. **Compare Results**: Protected should block/detect
4. **Iterate**: Improve anti-cheat, test again

## Next Steps

After basic testing, explore:

1. **Code Review**: Study the driver source to understand techniques
2. **Detection Logic**: Implement detection in your anti-cheat
3. **Logging**: Add logging to track detection events
4. **Response**: Decide action (block, flag, ban)

## Advanced Features

### Custom IOCTLs

Add your own detection vectors by modifying:
- `driver/driver.h` - Add IOCTL codes
- `driver/driver.c` - Implement handlers
- `usermode/driver_interface.cpp` - Add client functions

### Encryption Testing

The tool uses XOR encryption for strings. Your anti-cheat should:
- Detect encrypted strings in memory
- Identify XOR decryption routines
- Flag suspicious patterns

### Hook Detection

Test if your anti-cheat can detect:
- Inline hooks (JMP instructions)
- Import Address Table (IAT) hooks
- System Service Descriptor Table (SSDT) hooks

## Resources

- **WDK Documentation**: https://docs.microsoft.com/en-us/windows-hardware/drivers/
- **Anti-Cheat Development**: Research kernel-mode protection techniques
- **ImGui**: https://github.com/ocornut/imgui (for GUI modifications)

## Support

This is a testing tool. Issues should focus on:
- Build problems
- Documentation clarity
- Testing methodology

**Not supported**:
- Bypassing anti-cheats you don't own
- Cheating in games
- Unauthorized access

---

**Remember**: The goal is to **detect** this tool, not use it for cheating!
