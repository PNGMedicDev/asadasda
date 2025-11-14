#include "mapper.h"
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

namespace VulnDriver {

// Intel iqvw64e.sys style vulnerable driver
static HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
static const char* DRIVER_NAME = "iqvw64e";
static const char* DEVICE_NAME = "\\\\.\\Nal";

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

    // Try to open existing driver device
    g_DriverHandle = CreateFileA(
        DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        LOG_WARNING("Driver device not found, attempting to load driver...");

        // Load the vulnerable driver
        SC_HANDLE scManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scManager) {
            LOG_ERROR("Failed to open SC Manager");
            return false;
        }

        SC_HANDLE scService = CreateServiceA(
            scManager,
            DRIVER_NAME,
            DRIVER_NAME,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            "vulnerable_drivers\\iqvw64e.sys",
            NULL, NULL, NULL, NULL, NULL
        );

        if (!scService) {
            scService = OpenServiceA(scManager, DRIVER_NAME, SERVICE_ALL_ACCESS);
        }

        if (scService) {
            SERVICE_STATUS status;
            StartServiceA(scService, 0, NULL);
            Sleep(500);

            g_DriverHandle = CreateFileA(
                DEVICE_NAME,
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
        LOG_SUCCESS("Vulnerable driver loaded - kernel R/W access acquired!");
        LOG_WARNING("Your anti-cheat should have detected this!");
        return true;
    }

    LOG_ERROR("Failed to load vulnerable driver");
    return false;
}

bool UnloadVulnerableDriver() {
    if (g_DriverHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(g_DriverHandle);
        g_DriverHandle = INVALID_HANDLE_VALUE;
    }

    SC_HANDLE scManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scManager) {
        SC_HANDLE scService = OpenServiceA(scManager, DRIVER_NAME, SERVICE_ALL_ACCESS);
        if (scService) {
            SERVICE_STATUS status;
            ControlService(scService, SERVICE_CONTROL_STOP, &status);
            DeleteService(scService);
            CloseServiceHandle(scService);
        }
        CloseServiceHandle(scManager);
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
        LOG_SUCCESS("Allocated " << size << " bytes in kernel at 0x" << std::hex << op.result);
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
