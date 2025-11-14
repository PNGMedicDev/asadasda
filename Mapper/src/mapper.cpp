#include "mapper.h"

namespace Mapper {

static UINT64 g_MappedDriverBase = 0;
static UINT64 g_MappedDriverSize = 0;

bool MapDriver(const std::vector<BYTE>& driverData) {
    LOG_INFO("==============================================");
    LOG_INFO("Starting advanced manual mapping process...");
    LOG_INFO("==============================================");

    // Parse PE
    PE::PARSED_PE pe;
    if (!PE::ParsePE(driverData, pe)) {
        return false;
    }

    // Allocate kernel memory
    LOG_INFO("Allocating kernel memory for driver...");
    UINT64 driverBase = VulnDriver::AllocateKernelMemory(pe.imageSize);
    if (!driverBase) {
        LOG_ERROR("Failed to allocate kernel memory");
        return false;
    }

    LOG_SUCCESS("Driver allocated at kernel address: 0x" << std::hex << driverBase);
    LOG_WARNING("Your anti-cheat should detect non-module memory allocation!");

    // Process relocations
    if (!PE::ProcessRelocations(pe, driverBase)) {
        LOG_ERROR("Failed to process relocations");
        VulnDriver::FreeKernelMemory(driverBase);
        return false;
    }

    // Resolve imports
    if (!PE::ResolveImports(pe)) {
        LOG_ERROR("Failed to resolve imports");
        VulnDriver::FreeKernelMemory(driverBase);
        return false;
    }

    // Map headers
    LOG_INFO("Mapping PE headers to kernel...");
    if (!VulnDriver::WriteKernelMemory(driverBase, pe.headers.data(), pe.headers.size())) {
        LOG_ERROR("Failed to write headers");
        VulnDriver::FreeKernelMemory(driverBase);
        return false;
    }

    // Map sections
    LOG_INFO("Mapping sections to kernel...");
    for (auto& section : pe.sections) {
        UINT64 sectionAddr = driverBase + section.virtualAddress;
        LOG_INFO("Mapping section: " << section.name << " -> 0x" << std::hex << sectionAddr);

        if (!VulnDriver::WriteKernelMemory(sectionAddr, section.data.data(), section.data.size())) {
            LOG_ERROR("Failed to write section: " << section.name);
            VulnDriver::FreeKernelMemory(driverBase);
            return false;
        }
    }

    g_MappedDriverBase = driverBase;
    g_MappedDriverSize = pe.imageSize;

    LOG_SUCCESS("==============================================");
    LOG_SUCCESS("Driver successfully mapped to kernel memory!");
    LOG_SUCCESS("Base: 0x" << std::hex << driverBase);
    LOG_SUCCESS("Size: 0x" << std::hex << pe.imageSize);
    LOG_SUCCESS("Entry: 0x" << std::hex << (driverBase + pe.entryPoint));
    LOG_SUCCESS("==============================================");

    // Call entry point
    if (!CallDriverEntry(driverBase, pe.imageSize)) {
        LOG_ERROR("Failed to call driver entry point");
        return false;
    }

    // Apply stealth techniques
    LOG_INFO("Applying stealth techniques...");
    ClearPiDDBCache();
    ClearMmUnloadedDrivers();

    LOG_SUCCESS("==============================================");
    LOG_SUCCESS("DRIVER SUCCESSFULLY LOADED VIA MANUAL MAPPING!");
    LOG_SUCCESS("==============================================");
    LOG_WARNING("");
    LOG_WARNING("If your EAC-level anti-cheat didn't detect this:");
    LOG_WARNING("  1. Add vulnerable driver blacklist");
    LOG_WARNING("  2. Implement memory integrity scanning");
    LOG_WARNING("  3. Detect code in non-module memory");
    LOG_WARNING("  4. Monitor for PiDDBCache manipulation");
    LOG_WARNING("  5. Validate system callback origins");
    LOG_WARNING("");

    return true;
}

bool CallDriverEntry(UINT64 driverBase, UINT64 driverSize) {
    LOG_INFO("Calling driver entry point...");
    LOG_WARNING("Your anti-cheat should detect unauthorized DriverEntry call!");

    // In a real mapper, this would use a kernel callback or shellcode
    // For testing purposes, we log what would happen

    LOG_DEBUG("Would call: DriverEntry(DriverObject, RegistryPath)");
    LOG_DEBUG("From address: 0x" << std::hex << driverBase);

    // Simulate successful entry
    LOG_SUCCESS("Driver entry point executed (simulated)");
    return true;
}

bool ClearPiDDBCache() {
    LOG_INFO("Clearing PiDDBCacheTable to hide driver traces...");
    LOG_WARNING("Your anti-cheat should detect PiDDBCache manipulation!");

    // This would normally involve finding and clearing the PiDDBCacheTable
    // For testing, we just log the technique

    LOG_DEBUG("Technique: Locate PiDDBCacheTable in ntoskrnl.exe");
    LOG_DEBUG("Technique: Remove driver entries from cache");
    LOG_DEBUG("Technique: Clear driver load timestamps");

    LOG_SUCCESS("PiDDBCache cleared (simulated)");
    return true;
}

bool ClearMmUnloadedDrivers() {
    LOG_INFO("Clearing MmUnloadedDrivers list...");
    LOG_WARNING("Your anti-cheat should detect MmUnloadedDrivers manipulation!");

    LOG_DEBUG("Technique: Locate MmUnloadedDrivers array");
    LOG_DEBUG("Technique: Overwrite driver entries");
    LOG_DEBUG("Technique: Adjust array index");

    LOG_SUCCESS("MmUnloadedDrivers cleared (simulated)");
    return true;
}

} // namespace Mapper
