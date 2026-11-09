#include <windows.h>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>

namespace {
bool parse_pid(const char* value, DWORD& pid) {
    if (value == nullptr || *value == '-' || *value == 0) {
        return false;
    }
    char* end = nullptr;
    const unsigned long number = std::strtoul(value, &end, 10);
    if (end == value || *end != 0 || number == 0) {
        return false;
    }
    pid = static_cast<DWORD>(number);
    return true;
}

const char* kind(DWORD type) {
    switch (type) {
        case MEM_IMAGE: return "IMAGE";
        case MEM_MAPPED: return "MAPPED";
        case MEM_PRIVATE: return "PRIVATE";
        default: return "OTHER";
    }
}

const char* state(DWORD value) {
    switch (value) {
        case MEM_COMMIT: return "COMMIT";
        case MEM_RESERVE: return "RESERVE";
        case MEM_FREE: return "FREE";
        default: return "OTHER";
    }
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: memory-regions <pid>\n";
        return 1;
    }

    DWORD pid = 0;
    if (!parse_pid(argv[1], pid)) {
        std::cerr << "Invalid PID\n";
        return 1;
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process == nullptr) {
        std::cerr << "OpenProcess failed error=" << GetLastError() << "\n";
        return 2;
    }

    SYSTEM_INFO system{};
    GetNativeSystemInfo(&system);
    const std::uintptr_t min_address = reinterpret_cast<std::uintptr_t>(system.lpMinimumApplicationAddress);
    const std::uintptr_t max_address = reinterpret_cast<std::uintptr_t>(system.lpMaximumApplicationAddress);

    std::uint64_t committed = 0;
    std::size_t region_count = 0;

    for (std::uintptr_t address = min_address; address < max_address;) {
        MEMORY_BASIC_INFORMATION info{};
        const SIZE_T result = VirtualQueryEx(process, reinterpret_cast<const void*>(address), &info, sizeof(info));
        if (!result) {
            break;
        }

        if (info.State == MEM_COMMIT) {
            committed += info.RegionSize;
            std::cout << "base=" << info.BaseAddress
                      << " bytes=" << std::dec << info.RegionSize
                      << " state=" << state(info.State)
                      << " type=" << kind(info.Type)
                      << " protect=0x" << std::hex << info.Protect << std::dec << "\n";
            ++region_count;
        }

        const auto base = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
        if (info.RegionSize == 0 || base > std::numeric_limits<std::uintptr_t>::max() - info.RegionSize) {
            break;
        }
        const auto next = base + info.RegionSize;
        if (next <= address) {
            break;
        }
        address = next;
    }

    std::cout << "pid=" << pid
              << " committed_regions=" << region_count
              << " committed_bytes=" << committed << "\n";
    CloseHandle(process);
    return 0;
}
