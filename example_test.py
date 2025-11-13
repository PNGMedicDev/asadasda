#!/usr/bin/env python3
"""
Example test script demonstrating driver generator usage
Creates a test driver project and shows the testing workflow
"""

import sys
import os
from pathlib import Path

def print_banner(text):
    print("\n" + "=" * 70)
    print(f"  {text}")
    print("=" * 70 + "\n")

def run_example():
    print_banner("Driver Generator - Example Test Workflow")

    print("This example demonstrates how to:")
    print("  1. Generate a driver project")
    print("  2. Understand the project structure")
    print("  3. Plan your anti-cheat testing")
    print()

    # Example 1: Generate a simple test driver
    print_banner("Example 1: Generate Test Driver")

    print("Command:")
    print("  python driver_generator.py TestDriver ./TestProject")
    print()
    print("This creates:")
    print("  ./TestProject/")
    print("    ├── driver/          # Kernel driver with memory read/write")
    print("    ├── usermode/        # GUI application")
    print("    ├── build.py         # Auto-build script")
    print("    └── README.md        # Full documentation")
    print()

    # Example 2: Testing workflow
    print_banner("Example 2: Testing Workflow")

    workflow = """
    Step 1: BUILD THE TOOL
    ─────────────────────────────────────────────
    cd TestProject
    python build.py

    Step 2: ENABLE TEST SIGNING (Windows, as Admin)
    ─────────────────────────────────────────────
    bcdedit /set testsigning on
    # Reboot required

    Step 3: LOAD THE DRIVER (as Admin)
    ─────────────────────────────────────────────
    sc create TestDriver type= kernel binPath= C:\\Path\\To\\driver.sys
    sc start TestDriver
    sc query TestDriver   # Verify it's running

    Step 4: RUN YOUR ANTI-CHEAT
    ─────────────────────────────────────────────
    # Start your game/application with anti-cheat enabled

    Step 5: RUN THE TESTING TOOL
    ─────────────────────────────────────────────
    cd build/Release
    AntiCheatTester.exe TestDriver

    Step 6: ATTEMPT OPERATIONS
    ─────────────────────────────────────────────
    In the GUI:
      - Attach to your game process
      - Try to read memory
      - Try to write memory
      - Try to install a hook

    Step 7: VERIFY DETECTION
    ─────────────────────────────────────────────
    Your anti-cheat should:
      ✓ Detect the driver loading
      ✓ Block memory operations
      ✓ Log the violation
      ✓ Take action (flag/ban)

    Step 8: ITERATE
    ─────────────────────────────────────────────
    If NOT detected:
      - Improve your anti-cheat
      - Add driver detection
      - Implement memory protection
      - Test again
    """

    print(workflow)

    # Example 3: What to detect
    print_banner("Example 3: What Your Anti-Cheat Should Detect")

    detections = """
    ┌────────────────────────────────────────────────────────────────┐
    │ DETECTION CHECKLIST                                            │
    ├────────────────────────────────────────────────────────────────┤
    │                                                                │
    │ [ ] Kernel Driver Loading                                     │
    │     - Check for unsigned drivers                              │
    │     - Monitor device object creation                          │
    │     - Enumerate loaded modules                                │
    │                                                                │
    │ [ ] Memory Access Violations                                  │
    │     - Detect kernel-mode memory access                        │
    │     - Monitor CR0 register changes                            │
    │     - Check for suspicious memory patterns                    │
    │                                                                │
    │ [ ] Function Hooking                                          │
    │     - Scan for JMP instructions in code                       │
    │     - Verify IAT integrity                                    │
    │     - Check SSDT for hooks                                    │
    │                                                                │
    │ [ ] Process Manipulation                                      │
    │     - Monitor handle creation                                 │
    │     - Detect process attach/detach                            │
    │     - Check for suspicious IRP requests                       │
    │                                                                │
    │ [ ] Communication Channels                                    │
    │     - Monitor IOCTL calls                                     │
    │     - Detect unauthorized DeviceIoControl                     │
    │     - Log suspicious IRP traffic                              │
    │                                                                │
    └────────────────────────────────────────────────────────────────┘
    """

    print(detections)

    # Example 4: Sample code
    print_banner("Example 4: Using the Generated API")

    sample_code = '''
    // Example C++ code using the generated driver interface

    #include "driver_interface.h"
    #include <iostream>

    int main() {
        // Create driver interface
        DriverInterface driver("TestDriver");

        // Connect to driver
        if (!driver.Connect()) {
            std::cerr << "Failed to connect to driver!" << std::endl;
            return 1;
        }

        std::cout << "Connected to driver successfully!" << std::endl;

        // Find target process
        DWORD pid = driver.GetProcessIdByName("targetapp.exe");
        if (!pid) {
            std::cerr << "Process not found!" << std::endl;
            return 1;
        }

        std::cout << "Found process: " << pid << std::endl;

        // Get process base
        PVOID base = driver.GetProcessBase(pid);
        std::cout << "Process base: 0x" << std::hex << base << std::endl;

        // Read memory (should be detected by anti-cheat!)
        DWORD value = driver.Read<DWORD>(pid, (PVOID)0x12345678);
        std::cout << "Read value: " << value << std::endl;

        // Write memory (should be blocked by anti-cheat!)
        if (driver.Write<int>(pid, (PVOID)0x12345678, 999)) {
            std::cout << "Write successful (anti-cheat failed!)" << std::endl;
        } else {
            std::cout << "Write blocked (anti-cheat working!)" << std::endl;
        }

        // Install hook (should trigger anti-cheat!)
        BYTE originalBytes[16];
        PVOID target = (PVOID)0x12345678;
        PVOID hook = (PVOID)0x87654321;

        if (driver.HookFunction(target, hook, originalBytes)) {
            std::cout << "Hook installed (anti-cheat failed!)" << std::endl;

            // Remove hook
            driver.UnhookFunction(target, originalBytes);
        } else {
            std::cout << "Hook blocked (anti-cheat working!)" << std::endl;
        }

        driver.Disconnect();
        return 0;
    }
    '''

    print(sample_code)

    # Example 5: Anti-cheat implementation hints
    print_banner("Example 5: Implementing Detection in Your Anti-Cheat")

    detection_code = '''
    // Pseudo-code for anti-cheat detection

    class AntiCheatDetection {
    public:
        // Detect kernel drivers
        bool DetectKernelDrivers() {
            // Enumerate all loaded drivers
            for (auto driver : EnumerateDrivers()) {
                // Check if driver is signed
                if (!IsDriverSigned(driver)) {
                    LogViolation("Unsigned driver detected", driver);
                    return true;
                }

                // Check for suspicious device names
                if (IsSuspiciousDeviceName(driver.deviceName)) {
                    LogViolation("Suspicious device detected", driver);
                    return true;
                }
            }
            return false;
        }

        // Monitor memory access
        bool DetectMemoryViolations() {
            // Set memory guard pages
            SetMemoryGuards(criticalRegions);

            // Monitor for access violations
            if (AccessViolationDetected()) {
                LogViolation("Memory access violation");
                return true;
            }

            // Check CR0 register for WP bit changes
            if (IsWriteProtectionDisabled()) {
                LogViolation("Write protection disabled");
                return true;
            }

            return false;
        }

        // Detect function hooks
        bool DetectHooks() {
            // Scan critical functions
            for (auto func : criticalFunctions) {
                // Check for JMP instructions
                if (StartsWithJMP(func)) {
                    LogViolation("Inline hook detected", func);
                    return true;
                }

                // Verify function hash
                if (GetHash(func) != originalHash) {
                    LogViolation("Function modified", func);
                    return true;
                }
            }

            return false;
        }

        // Monitor IOCTL calls
        bool DetectIOCTL() {
            // Hook NtDeviceIoControlFile
            auto original = HookSystemCall(NtDeviceIoControlFile);

            // Monitor all IOCTL calls
            if (SuspiciousIOCTL(deviceHandle, controlCode)) {
                LogViolation("Suspicious IOCTL", controlCode);
                return true;
            }

            return false;
        }

        void LogViolation(const char* reason, void* context = nullptr) {
            // Log to server
            SendToServer({
                timestamp: GetTimestamp(),
                reason: reason,
                context: context,
                processId: GetCurrentProcessId()
            });

            // Take action
            TakeAction(VIOLATION_BAN);
        }
    };
    '''

    print(detection_code)

    # Final notes
    print_banner("Important Notes")

    notes = """
    ✓ This tool is for TESTING your anti-cheat, not bypassing others
    ✓ Always test on systems you own
    ✓ If your anti-cheat can't detect this, it needs work
    ✓ Modern anti-cheats use multiple detection layers
    ✓ Kernel-level protection is just one aspect

    Testing Methodology:
    ──────────────────────────────────────────────────────────────
    1. BASELINE: Test on unprotected process (verify tool works)
    2. PROTECTED: Test against your anti-cheat
    3. COMPARE: Tool should be detected/blocked
    4. IMPROVE: Enhance anti-cheat detection
    5. REPEAT: Continuous improvement cycle

    Good Testing Practices:
    ──────────────────────────────────────────────────────────────
    • Test in isolated environment first
    • Document what you're testing
    • Keep logs of detection attempts
    • Compare with known anti-cheat solutions
    • Share findings with your security team
    """

    print(notes)

    print_banner("Ready to Start Testing!")

    print("To generate your first test driver:")
    print()
    print("  python driver_generator.py MyTestDriver ./output")
    print()
    print("Then follow the QUICKSTART.md guide!")
    print()

if __name__ == "__main__":
    run_example()
