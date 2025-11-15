#include "mapper.h"

namespace PE {

bool ParsePE(const std::vector<BYTE>& data, PARSED_PE& result) {
    LOG_INFO("Parsing PE file...");

    if (data.size() < sizeof(IMAGE_DOS_HEADER)) {
        LOG_ERROR("File too small");
        return false;
    }

    auto dosHeader = (PIMAGE_DOS_HEADER)data.data();
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        LOG_ERROR("Invalid DOS signature");
        return false;
    }

    auto ntHeaders = (PIMAGE_NT_HEADERS64)(data.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        LOG_ERROR("Invalid NT signature");
        return false;
    }

    result.entryPoint = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    result.imageBase = ntHeaders->OptionalHeader.ImageBase;
    result.imageSize = ntHeaders->OptionalHeader.SizeOfImage;

    LOG_SUCCESS("Image base: 0x" << std::hex << result.imageBase);
    LOG_SUCCESS("Image size: 0x" << std::hex << result.imageSize);
    LOG_SUCCESS("Entry point: 0x" << std::hex << result.entryPoint);

    // Copy headers
    size_t headerSize = ntHeaders->OptionalHeader.SizeOfHeaders;
    result.headers.resize(headerSize);
    memcpy(result.headers.data(), data.data(), headerSize);

    // Parse sections
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION section;
        section.name = std::string((char*)sectionHeader[i].Name, 8);
        section.virtualAddress = sectionHeader[i].VirtualAddress;
        section.virtualSize = sectionHeader[i].Misc.VirtualSize;

        section.data.resize(sectionHeader[i].SizeOfRawData);
        if (sectionHeader[i].PointerToRawData > 0) {
            memcpy(
                section.data.data(),
                data.data() + sectionHeader[i].PointerToRawData,
                sectionHeader[i].SizeOfRawData
            );
        }

        result.sections.push_back(section);
        LOG_INFO("Section: " << section.name << " @ 0x" << std::hex << section.virtualAddress);
    }

    LOG_SUCCESS("PE parsing complete - " << result.sections.size() << " sections");
    return true;
}

bool ProcessRelocations(PARSED_PE& pe, UINT64 newBase) {
    LOG_INFO("Processing relocations for new base 0x" << std::hex << newBase);

    UINT64 delta = newBase - pe.imageBase;
    if (delta == 0) {
        LOG_INFO("No relocation needed");
        return true;
    }

    // Find relocation section - correctly parse NT headers from DOS header
    auto dosHeader = (PIMAGE_DOS_HEADER)pe.headers.data();
    auto ntHeaders = (PIMAGE_NT_HEADERS64)(pe.headers.data() + dosHeader->e_lfanew);
    auto relocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (relocDir->Size == 0 || relocDir->VirtualAddress == 0) {
        LOG_WARNING("No relocation table in PE headers");
        return true;
    }

    LOG_DEBUG("Reloc directory: RVA=0x" << std::hex << relocDir->VirtualAddress << " Size=0x" << relocDir->Size);

    // Find section containing relocations
    BYTE* relocData = nullptr;
    for (auto& section : pe.sections) {
        // Check if this section contains the relocation directory
        UINT64 sectionEnd = section.virtualAddress + std::max(section.virtualSize, (UINT64)section.data.size());

        LOG_DEBUG("Checking section at 0x" << std::hex << section.virtualAddress <<
                  " size=0x" << section.virtualSize << " dataSize=0x" << section.data.size());

        if (section.virtualAddress <= relocDir->VirtualAddress &&
            sectionEnd > relocDir->VirtualAddress) {
            UINT64 offset = relocDir->VirtualAddress - section.virtualAddress;
            if (offset < section.data.size()) {
                relocData = section.data.data() + offset;
                LOG_SUCCESS("Found relocations in section at offset 0x" << std::hex << offset);
                break;
            }
        }
    }

    if (!relocData) {
        LOG_ERROR("Relocation section not found in any PE section");
        return false;
    }

    UINT64 relocSize = relocDir->Size;
    UINT64 relocOffset = 0;
    int relocCount = 0;

    while (relocOffset < relocSize) {
        auto relocBlock = (PIMAGE_BASE_RELOCATION)(relocData + relocOffset);
        if (relocBlock->SizeOfBlock == 0) break;

        UINT64 page = relocBlock->VirtualAddress;
        UINT32 count = (relocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        WORD* relocs = (WORD*)((BYTE*)relocBlock + sizeof(IMAGE_BASE_RELOCATION));

        for (UINT32 i = 0; i < count; i++) {
            WORD type = relocs[i] >> 12;
            WORD offset = relocs[i] & 0xFFF;

            if (type == IMAGE_REL_BASED_DIR64) {
                UINT64 rva = page + offset;

                // Find section
                for (auto& section : pe.sections) {
                    if (rva >= section.virtualAddress &&
                        rva < section.virtualAddress + section.virtualSize) {
                        UINT64 secOffset = rva - section.virtualAddress;
                        UINT64* patchAddr = (UINT64*)(section.data.data() + secOffset);
                        *patchAddr += delta;
                        relocCount++;
                        break;
                    }
                }
            }
        }

        relocOffset += relocBlock->SizeOfBlock;
    }

    LOG_SUCCESS("Processed " << relocCount << " relocations");
    return true;
}

bool ResolveImports(PARSED_PE& pe) {
    LOG_INFO("Resolving kernel imports...");

    // Correctly parse NT headers from DOS header
    auto dosHeader = (PIMAGE_DOS_HEADER)pe.headers.data();
    auto ntHeaders = (PIMAGE_NT_HEADERS64)(pe.headers.data() + dosHeader->e_lfanew);
    auto importDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (importDir->Size == 0) {
        LOG_INFO("No imports to resolve");
        return true;
    }

    // Find import descriptor
    BYTE* importData = nullptr;
    for (auto& section : pe.sections) {
        if (section.virtualAddress <= importDir->VirtualAddress &&
            section.virtualAddress + section.virtualSize > importDir->VirtualAddress) {
            UINT64 offset = importDir->VirtualAddress - section.virtualAddress;
            importData = section.data.data() + offset;
            break;
        }
    }

    if (!importData) {
        LOG_ERROR("Import directory not found");
        return false;
    }

    auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)importData;
    int moduleCount = 0;

    while (importDesc->Name != 0) {
        // Get module name
        std::string moduleName;
        for (auto& section : pe.sections) {
            if (importDesc->Name >= section.virtualAddress &&
                importDesc->Name < section.virtualAddress + section.virtualSize) {
                UINT64 offset = importDesc->Name - section.virtualAddress;
                moduleName = (char*)(section.data.data() + offset);
                break;
            }
        }

        if (moduleName.empty()) {
            importDesc++;
            continue;
        }

        LOG_INFO("Resolving imports from: " << moduleName);

        UINT64 moduleBase = Utils::GetKernelModuleBase(moduleName);
        if (!moduleBase) {
            LOG_ERROR("Failed to find kernel module: " << moduleName);
            return false;
        }

        // Process thunks
        UINT64 thunkRVA = importDesc->FirstThunk;
        for (auto& section : pe.sections) {
            if (thunkRVA >= section.virtualAddress &&
                thunkRVA < section.virtualAddress + section.virtualSize) {

                UINT64 offset = thunkRVA - section.virtualAddress;
                auto thunk = (PIMAGE_THUNK_DATA64)(section.data.data() + offset);

                while (thunk->u1.AddressOfData != 0) {
                    if (!(thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)) {
                        // Import by name
                        UINT64 nameRVA = thunk->u1.AddressOfData;
                        std::string funcName;

                        for (auto& nameSection : pe.sections) {
                            if (nameRVA >= nameSection.virtualAddress &&
                                nameRVA < nameSection.virtualAddress + nameSection.virtualSize) {
                                UINT64 nameOffset = nameRVA - nameSection.virtualAddress;
                                auto importByName = (PIMAGE_IMPORT_BY_NAME)(nameSection.data.data() + nameOffset);
                                funcName = (char*)importByName->Name;
                                break;
                            }
                        }

                        if (!funcName.empty()) {
                            UINT64 funcAddress = Utils::GetKernelExport(moduleBase, funcName);
                            if (funcAddress) {
                                thunk->u1.Function = funcAddress;
                                LOG_DEBUG("Resolved: " << funcName << " -> 0x" << std::hex << funcAddress);
                            } else {
                                LOG_WARNING("Failed to resolve: " << funcName);
                            }
                        }
                    }
                    thunk++;
                }
                break;
            }
        }

        moduleCount++;
        importDesc++;
    }

    LOG_SUCCESS("Resolved imports from " << moduleCount << " modules");
    return true;
}

} // namespace PE
