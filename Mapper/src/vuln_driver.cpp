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

// Supported vulnerable drivers (tries in order - gdrv first for AMD)
static DriverInfo g_supportedDrivers[] = {
    {"gdrv", "gdrv.sys", "\\\\.\\GDrv", "Gigabyte - works on AMD/Intel (recommended)"},
    {"iqvw64e", "iqvw64e.sys", "\\\\.\\Nal", "Intel - works on AMD/Intel"},
    {"RTCore64", "RTCore64.sys", "\\\\.\\RTCore64", "MSI Afterburner - works on AMD/Intel"}
};

static HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
static DriverInfo* g_ActiveDriver = nullptr;

// IOCTL codes for vulnerable driver
#define IOCTL_READ_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ALLOC_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FREE_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(push, 1)
struct MEMORY_OPERATION {
    UINT64 address;
    PVOID buffer;
    SIZE_T size;
    UINT64 result;
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
        std::string driverPath = "vulnerable_drivers\\" + std::string(driver.filename);
        std::ifstream test(driverPath);
        if (!test.good()) {
            LOG_WARNING("  Driver file not found, skipping...");
            std::cout << std::endl;
            continue;
        }
        test.close();

        // Try to open existing driver device
        g_DriverHandle = CreateFileA(
            driver.deviceName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (g_DriverHandle == INVALID_HANDLE_VALUE) {
            LOG_DEBUG("  Device not found, attempting to load...");

            // Load the vulnerable driver
            SC_HANDLE scManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
            if (!scManager) {
                LOG_ERROR("  Failed to open SC Manager (run as Administrator!)");
                std::cout << std::endl;
                continue;
            }

            SC_HANDLE scService = CreateServiceA(
                scManager,
                driver.name,
                driver.name,
                SERVICE_ALL_ACCESS,
                SERVICE_KERNEL_DRIVER,
                SERVICE_DEMAND_START,
                SERVICE_ERROR_IGNORE,
                driverPath.c_str(),
                NULL, NULL, NULL, NULL, NULL
            );

            if (!scService) {
                scService = OpenServiceA(scManager, driver.name, SERVICE_ALL_ACCESS);
            }

            if (scService) {
                SERVICE_STATUS status;
                StartServiceA(scService, 0, NULL);
                Sleep(500);

                g_DriverHandle = CreateFileA(
                    driver.deviceName,
                    GENERIC_READ | GENERIC_WRITE,
                    0, NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                CloseServiceHandle(scService);
            }

            CloseServiceHandle(scManager);
        }

        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            g_ActiveDriver = &driver;
            std::cout << std::endl;
            LOG_SUCCESS("✓ Loaded: " << driver.filename);
            LOG_SUCCESS("Kernel R/W access acquired!");
            LOG_WARNING("Your anti-cheat should have detected this!");
            std::cout << std::endl;
            return true;
        }

        LOG_WARNING("  Failed to load, trying next...");
        std::cout << std::endl;
    }

    LOG_ERROR("Failed to load any vulnerable driver");
    LOG_INFO("Available drivers: gdrv.sys, iqvw64e.sys, RTCore64.sys");
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
    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    MEMORY_OPERATION op = {};
    op.address = address;
    op.buffer = buffer;
    op.size = size;

    DWORD bytesReturned;
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
    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    MEMORY_OPERATION op = {};
    op.address = address;
    op.buffer = buffer;
    op.size = size;

    DWORD bytesReturned;
    return DeviceIoControl(
        g_DriverHandle,
        IOCTL_WRITE_MEMORY,
        &op,
        sizeof(op),
        &op,
        sizeof(op),
        &bytesReturned,
        NULL
    );
}

UINT64 AllocateKernelMemory(SIZE_T size) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MEMORY_OPERATION op = {};
    op.size = size;

    DWORD bytesReturned;
    if (DeviceIoControl(
        g_DriverHandle,
        IOCTL_ALLOC_MEMORY,
        &op,
        sizeof(op),
        &op,
        sizeof(op),
        &bytesReturned,
        NULL
    )) {
        return op.result;
    }

    return 0;
}

bool FreeKernelMemory(UINT64 address) {
    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    MEMORY_OPERATION op = {};
    op.address = address;

    DWORD bytesReturned;
    return DeviceIoControl(
        g_DriverHandle,
        IOCTL_FREE_MEMORY,
        &op,
        sizeof(op),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
}

} // namespace VulnDriver
