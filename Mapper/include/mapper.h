#pragma once

#include <Windows.h>
#include <wininet.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>

#pragma comment(lib, "wininet.lib")

// Console colors
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Logging macros
#define LOG_INFO(msg)    std::cout << COLOR_CYAN    << "[+] " << msg << COLOR_RESET << std::endl
#define LOG_SUCCESS(msg) std::cout << COLOR_GREEN   << "[✓] " << msg << COLOR_RESET << std::endl
#define LOG_ERROR(msg)   std::cout << COLOR_RED     << "[✗] " << msg << COLOR_RESET << std::endl
#define LOG_WARNING(msg) std::cout << COLOR_YELLOW  << "[!] " << msg << COLOR_RESET << std::endl
#define LOG_DEBUG(msg)   std::cout << COLOR_MAGENTA << "[*] " << msg << COLOR_RESET << std::endl

// Driver downloader
namespace DriverDownloader {
    struct DriverInfo {
        std::string filename;
        std::string url;
        std::string description;
    };

    bool DownloadFile(const std::string& url, const std::string& outputPath);
    bool DownloadAllDrivers();
    std::vector<DriverInfo> GetDriverList();
    bool LoadConfig();
}

// Vulnerable driver interface
namespace VulnDriver {
    bool LoadVulnerableDriver();
    bool UnloadVulnerableDriver();
    bool ReadKernelMemory(UINT64 address, PVOID buffer, SIZE_T size);
    bool WriteKernelMemory(UINT64 address, PVOID buffer, SIZE_T size);
    UINT64 AllocateKernelMemory(SIZE_T size);
    bool FreeKernelMemory(UINT64 address);
}

// Manual mapper
namespace Mapper {
    bool MapDriver(const std::vector<BYTE>& driverData);
    bool CallDriverEntry(UINT64 driverBase, UINT64 driverSize);
    bool ClearPiDDBCache();
    bool ClearMmUnloadedDrivers();
}

// PE parser
namespace PE {
    struct IMAGE_SECTION {
        std::string name;
        UINT64 virtualAddress;
        UINT64 virtualSize;
        std::vector<BYTE> data;
    };

    struct PARSED_PE {
        UINT64 entryPoint;
        UINT64 imageBase;
        UINT64 imageSize;
        std::vector<IMAGE_SECTION> sections;
        std::vector<BYTE> headers;
    };

    bool ParsePE(const std::vector<BYTE>& data, PARSED_PE& result);
    bool ProcessRelocations(PARSED_PE& pe, UINT64 newBase);
    bool ResolveImports(PARSED_PE& pe);
}

// Utilities
namespace Utils {
    std::vector<BYTE> ReadFileToMemory(const std::string& path);
    UINT64 GetKernelModuleBase(const std::string& moduleName);
    UINT64 GetKernelExport(UINT64 moduleBase, const std::string& exportName);
}
