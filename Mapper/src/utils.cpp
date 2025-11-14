#include "mapper.h"
#include <TlHelp32.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

namespace Utils {

std::vector<BYTE> ReadFileToMemory(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: " << path);
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<BYTE> buffer(size);
    if (!file.read((char*)buffer.data(), size)) {
        LOG_ERROR("Failed to read file");
        return {};
    }

    LOG_SUCCESS("Read " << size << " bytes from file");
    return buffer;
}

UINT64 GetKernelModuleBase(const std::string& moduleName) {
    // Use NtQuerySystemInformation to get kernel module base
    typedef NTSTATUS(NTAPI* pNtQuerySystemInformation)(
        ULONG SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength
    );

    auto NtQuerySystemInformation = (pNtQuerySystemInformation)
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");

    if (!NtQuerySystemInformation) {
        return 0;
    }

    // SystemModuleInformation = 11
    ULONG bufferSize = 0;
    NtQuerySystemInformation(11, NULL, 0, &bufferSize);

    if (bufferSize == 0) {
        return 0;
    }

    std::vector<BYTE> buffer(bufferSize);
    if (NtQuerySystemInformation(11, buffer.data(), bufferSize, &bufferSize) != 0) {
        return 0;
    }

    struct RTL_PROCESS_MODULE {
        PVOID Section;
        PVOID MappedBase;
        PVOID ImageBase;
        ULONG ImageSize;
        ULONG Flags;
        USHORT LoadOrderIndex;
        USHORT InitOrderIndex;
        USHORT LoadCount;
        USHORT OffsetToFileName;
        CHAR FullPathName[256];
    };

    struct RTL_PROCESS_MODULES {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE Modules[1];
    };

    auto modules = (RTL_PROCESS_MODULES*)buffer.data();

    for (ULONG i = 0; i < modules->NumberOfModules; i++) {
        std::string fullPath = modules->Modules[i].FullPathName;
        std::string fileName = fullPath.substr(fullPath.find_last_of("\\") + 1);

        if (_stricmp(fileName.c_str(), moduleName.c_str()) == 0) {
            UINT64 base = (UINT64)modules->Modules[i].ImageBase;
            LOG_DEBUG("Found kernel module: " << moduleName << " @ 0x" << std::hex << base);
            return base;
        }
    }

    LOG_WARNING("Kernel module not found: " << moduleName);
    return 0;
}

UINT64 GetKernelExport(UINT64 moduleBase, const std::string& exportName) {
    // Read kernel module headers
    BYTE headerBuffer[0x1000];
    if (!VulnDriver::ReadKernelMemory(moduleBase, headerBuffer, sizeof(headerBuffer))) {
        return 0;
    }

    auto dosHeader = (PIMAGE_DOS_HEADER)headerBuffer;
    auto ntHeaders = (PIMAGE_NT_HEADERS64)(headerBuffer + dosHeader->e_lfanew);

    auto exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (exportDir->Size == 0) {
        return 0;
    }

    // Read export directory
    std::vector<BYTE> exportData(exportDir->Size);
    if (!VulnDriver::ReadKernelMemory(
        moduleBase + exportDir->VirtualAddress,
        exportData.data(),
        exportDir->Size
    )) {
        return 0;
    }

    auto exportTable = (PIMAGE_EXPORT_DIRECTORY)exportData.data();
    UINT64 baseAddr = moduleBase + exportDir->VirtualAddress;

    // Read name table
    std::vector<DWORD> nameTable(exportTable->NumberOfNames);
    if (!VulnDriver::ReadKernelMemory(
        moduleBase + exportTable->AddressOfNames,
        nameTable.data(),
        nameTable.size() * sizeof(DWORD)
    )) {
        return 0;
    }

    // Read ordinal table
    std::vector<WORD> ordinalTable(exportTable->NumberOfNames);
    if (!VulnDriver::ReadKernelMemory(
        moduleBase + exportTable->AddressOfNameOrdinals,
        ordinalTable.data(),
        ordinalTable.size() * sizeof(WORD)
    )) {
        return 0;
    }

    // Read function table
    std::vector<DWORD> functionTable(exportTable->NumberOfFunctions);
    if (!VulnDriver::ReadKernelMemory(
        moduleBase + exportTable->AddressOfFunctions,
        functionTable.data(),
        functionTable.size() * sizeof(DWORD)
    )) {
        return 0;
    }

    // Search for export
    for (DWORD i = 0; i < exportTable->NumberOfNames; i++) {
        char nameBuffer[256] = {};
        if (!VulnDriver::ReadKernelMemory(
            moduleBase + nameTable[i],
            nameBuffer,
            sizeof(nameBuffer)
        )) {
            continue;
        }

        if (strcmp(nameBuffer, exportName.c_str()) == 0) {
            WORD ordinal = ordinalTable[i];
            UINT64 funcAddress = moduleBase + functionTable[ordinal];
            return funcAddress;
        }
    }

    return 0;
}

} // namespace Utils
