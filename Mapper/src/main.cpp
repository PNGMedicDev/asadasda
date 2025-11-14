#include "mapper.h"
#include <conio.h>
#include <commdlg.h>

std::string OpenFileDialog() {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Driver Files (*.sys)\0*.sys\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Select Driver File to Map";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

void PrintBanner() {
    system("cls");
    std::cout << COLOR_CYAN << R"(
    ╔═══════════════════════════════════════════════════════════════╗
    ║                                                               ║
    ║         ADVANCED MANUAL MAPPER - Anti-Cheat Tester           ║
    ║                                                               ║
    ║   This tool tests if your EAC-level anti-cheat can detect:   ║
    ║     • Vulnerable driver exploitation                         ║
    ║     • Manual driver mapping                                  ║
    ║     • Kernel memory manipulation                             ║
    ║     • PiDDBCache clearing                                    ║
    ║     • Code execution from non-module memory                  ║
    ║                                                               ║
    ║   YOUR ANTI-CHEAT SHOULD BLOCK THIS!                         ║
    ║                                                               ║
    ╚═══════════════════════════════════════════════════════════════╝
)" << COLOR_RESET << std::endl;
}

void PrintUsage() {
    std::cout << std::endl;
    std::cout << COLOR_YELLOW << "Usage:" << COLOR_RESET << std::endl;
    std::cout << "  " << COLOR_GREEN << "Drag and drop" << COLOR_RESET
              << " a .sys driver file onto this executable" << std::endl;
    std::cout << "  OR" << std::endl;
    std::cout << "  " << COLOR_GREEN << "Double-click" << COLOR_RESET
              << " and select file from dialog" << std::endl;
    std::cout << std::endl;
}

void PrintDetectionChecklist() {
    std::cout << std::endl;
    std::cout << COLOR_MAGENTA << "═══════════════════════════════════════════════════════" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAGENTA << "  EAC-LEVEL ANTI-CHEAT DETECTION CHECKLIST" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAGENTA << "═══════════════════════════════════════════════════════" << COLOR_RESET << std::endl;
    std::cout << std::endl;
    std::cout << COLOR_YELLOW << "Your anti-cheat should have detected:" << COLOR_RESET << std::endl;
    std::cout << "  [ ] Vulnerable driver (iqvw64e.sys) loading" << std::endl;
    std::cout << "  [ ] Kernel memory read/write operations" << std::endl;
    std::cout << "  [ ] Non-module memory allocation in kernel" << std::endl;
    std::cout << "  [ ] Manual PE mapping to kernel space" << std::endl;
    std::cout << "  [ ] Unauthorized driver entry point call" << std::endl;
    std::cout << "  [ ] PiDDBCacheTable manipulation" << std::endl;
    std::cout << "  [ ] MmUnloadedDrivers manipulation" << std::endl;
    std::cout << "  [ ] Code execution from unmapped memory" << std::endl;
    std::cout << std::endl;
    std::cout << COLOR_RED << "If ANY of these were NOT detected:" << COLOR_RESET << std::endl;
    std::cout << "  → Your anti-cheat needs improvement!" << std::endl;
    std::cout << "  → Review detection implementation" << std::endl;
    std::cout << "  → Add missing detection vectors" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    PrintBanner();

    std::string driverPath;

    // Check if file was dragged onto the exe
    if (argc >= 2) {
        driverPath = argv[1];
    } else {
        // No file dragged, open file picker dialog
        LOG_INFO("No file specified - opening file picker...");
        std::cout << std::endl;
        driverPath = OpenFileDialog();

        if (driverPath.empty()) {
            LOG_ERROR("No file selected!");
            std::cout << std::endl << "Press any key to exit...";
            _getch();
            return 1;
        }
    }

    LOG_INFO("Driver file: " << driverPath);
    std::cout << std::endl;

    // Check if file exists
    std::ifstream testFile(driverPath);
    if (!testFile.good()) {
        LOG_ERROR("Driver file not found: " << driverPath);
        std::cout << std::endl << "Press any key to exit...";
        _getch();
        return 1;
    }
    testFile.close();

    std::cout << COLOR_RED << "⚠️  WARNING ⚠️" << COLOR_RESET << std::endl;
    std::cout << "This will attempt to load an unsigned driver using manual mapping." << std::endl;
    std::cout << "Your anti-cheat should DETECT and BLOCK this!" << std::endl;
    std::cout << std::endl;
    std::cout << "Continue? (Y/N): ";

    char choice = _getch();
    std::cout << choice << std::endl << std::endl;

    if (choice != 'Y' && choice != 'y') {
        LOG_INFO("Cancelled by user");
        return 0;
    }

    // Load driver file
    LOG_INFO("Reading driver file into memory...");
    auto driverData = Utils::ReadFileToMemory(driverPath);
    if (driverData.empty()) {
        LOG_ERROR("Failed to read driver file");
        std::cout << std::endl << "Press any key to exit...";
        _getch();
        return 1;
    }

    std::cout << std::endl;
    LOG_INFO("═══════════════════════════════════════════════════════");
    LOG_INFO("  PHASE 1: Loading Vulnerable Driver");
    LOG_INFO("═══════════════════════════════════════════════════════");
    std::cout << std::endl;

    // Load vulnerable driver
    if (!VulnDriver::LoadVulnerableDriver()) {
        LOG_ERROR("Failed to load vulnerable driver");
        LOG_INFO("Make sure iqvw64e.sys is in vulnerable_drivers folder");
        std::cout << std::endl << "Press any key to exit...";
        _getch();
        return 1;
    }

    std::cout << std::endl;
    LOG_INFO("═══════════════════════════════════════════════════════");
    LOG_INFO("  PHASE 2: Manual Mapping Driver");
    LOG_INFO("═══════════════════════════════════════════════════════");
    std::cout << std::endl;

    // Map the driver
    bool success = Mapper::MapDriver(driverData);

    std::cout << std::endl;
    LOG_INFO("═══════════════════════════════════════════════════════");
    LOG_INFO("  PHASE 3: Cleanup");
    LOG_INFO("═══════════════════════════════════════════════════════");
    std::cout << std::endl;

    // Cleanup
    VulnDriver::UnloadVulnerableDriver();

    if (success) {
        LOG_SUCCESS("Manual mapping process completed successfully!");
        PrintDetectionChecklist();
    } else {
        LOG_ERROR("Manual mapping failed!");
    }

    std::cout << std::endl << "Press any key to exit...";
    _getch();

    return success ? 0 : 1;
}
