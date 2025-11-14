#include "mapper.h"
#include <shlobj.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace DriverDownloader {

// Default driver URLs (can be customized in drivers_config.txt)
static std::map<std::string, DriverInfo> g_driverList = {
    {"gdrv.sys", {
        "gdrv.sys",
        "https://github.com/hacksysteam/HackSysExtremeVulnerableDriver/raw/master/Driver/Bin/gdrv.sys",
        "Gigabyte driver - works on AMD/Intel (most popular)"
    }},
    {"iqvw64e.sys", {
        "iqvw64e.sys",
        "https://github.com/TheCruZ/kdmapper/raw/master/kdmapper/iqvw64e.sys",
        "Intel driver - works on AMD/Intel"
    }},
    {"RTCore64.sys", {
        "RTCore64.sys",
        "https://github.com/Aki-Hoshikawa/EyeOfRa/raw/master/EyeOfRa/Exploits/RTCore64.sys",
        "MSI Afterburner driver - works on AMD/Intel"
    }}
};

bool LoadConfig() {
    std::ifstream config("drivers_config.txt");
    if (!config.is_open()) {
        LOG_WARNING("drivers_config.txt not found, using default URLs");
        return false;
    }

    LOG_INFO("Loading custom driver URLs from drivers_config.txt...");

    std::string line;
    while (std::getline(config, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Parse: filename|url|description
        size_t pipe1 = line.find('|');
        size_t pipe2 = line.find('|', pipe1 + 1);

        if (pipe1 != std::string::npos && pipe2 != std::string::npos) {
            DriverInfo info;
            info.filename = line.substr(0, pipe1);
            info.url = line.substr(pipe1 + 1, pipe2 - pipe1 - 1);
            info.description = line.substr(pipe2 + 1);

            g_driverList[info.filename] = info;
            LOG_DEBUG("Added driver: " << info.filename);
        }
    }

    config.close();
    return true;
}

std::vector<DriverInfo> GetDriverList() {
    std::vector<DriverInfo> list;
    for (const auto& pair : g_driverList) {
        list.push_back(pair.second);
    }
    return list;
}

bool DownloadFile(const std::string& url, const std::string& outputPath) {
    LOG_INFO("Downloading: " << url);

    // Initialize WinINet
    HINTERNET hInternet = InternetOpenA(
        "DriverMapper/1.0",
        INTERNET_OPEN_TYPE_DIRECT,
        NULL,
        NULL,
        0
    );

    if (!hInternet) {
        LOG_ERROR("Failed to initialize WinINet");
        return false;
    }

    // Open URL
    HINTERNET hUrl = InternetOpenUrlA(
        hInternet,
        url.c_str(),
        NULL,
        0,
        INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD,
        0
    );

    if (!hUrl) {
        LOG_ERROR("Failed to open URL");
        InternetCloseHandle(hInternet);
        return false;
    }

    // Create output file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        LOG_ERROR("Failed to create output file: " << outputPath);
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }

    // Download data
    BYTE buffer[4096];
    DWORD bytesRead = 0;
    DWORD totalBytes = 0;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        outFile.write((char*)buffer, bytesRead);
        totalBytes += bytesRead;
    }

    outFile.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    LOG_SUCCESS("Downloaded " << totalBytes << " bytes to " << outputPath);
    return totalBytes > 0;
}

bool DownloadAllDrivers() {
    // Load custom config if available
    LoadConfig();

    // Create vulnerable_drivers directory
    fs::create_directories("vulnerable_drivers");

    LOG_INFO("========================================================");
    LOG_INFO("  Downloading Vulnerable Drivers");
    LOG_INFO("========================================================");
    std::cout << std::endl;

    bool anySuccess = false;
    int downloaded = 0;
    int skipped = 0;

    for (const auto& pair : g_driverList) {
        const DriverInfo& info = pair.second;
        std::string outputPath = "vulnerable_drivers\\" + info.filename;

        // Check if already exists
        if (fs::exists(outputPath)) {
            LOG_INFO("[SKIP] " << info.filename << " (already exists)");
            skipped++;
            continue;
        }

        LOG_INFO("Downloading: " << info.filename);
        LOG_DEBUG("  " << info.description);

        if (DownloadFile(info.url, outputPath)) {
            LOG_SUCCESS("  ✓ " << info.filename);
            downloaded++;
            anySuccess = true;
        } else {
            LOG_ERROR("  ✗ Failed to download " << info.filename);
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    LOG_INFO("========================================================");
    LOG_INFO("  Download Summary");
    LOG_INFO("========================================================");
    LOG_SUCCESS("Downloaded: " << downloaded << " driver(s)");
    LOG_INFO("Skipped: " << skipped << " (already exist)");
    LOG_INFO("Total available: " << g_driverList.size());
    std::cout << std::endl;

    return anySuccess || (skipped > 0);
}

} // namespace DriverDownloader
