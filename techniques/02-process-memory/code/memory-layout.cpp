#include <windows.h>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace {
void display(const char* stage, const void* address) {
    MEMORY_BASIC_INFORMATION info{};
    const SIZE_T result = VirtualQuery(address, &info, sizeof(info));
    if (result == 0) {
        std::cerr << "VirtualQuery failed: " << GetLastError() << "\n";
        return;
    }

    std::cout << stage
              << " base=" << info.BaseAddress
              << " size=" << info.RegionSize
              << " state=0x" << std::hex << info.State
              << " protect=0x" << info.Protect
              << " type=0x" << info.Type << std::dec << std::endl;
}
}

int main() {
    SYSTEM_INFO system{};
    GetSystemInfo(&system);
    const SIZE_T bytes = system.dwPageSize;

    std::cout << "experiment=process-memory pid=" << GetCurrentProcessId()
              << " tid=" << GetCurrentThreadId()
              << " page_size=" << bytes << std::endl;

    void* region = VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (region == nullptr) {
        std::cerr << "VirtualAlloc failed: " << GetLastError() << "\n";
        return 1;
    }

    const char marker[] = "WININJ_MEMORY_LAYOUT_MARKER_001";
    std::memcpy(region, marker, sizeof(marker));
    display("allocated", region);
    std::cout << "marker=" << static_cast<const char*>(region) << std::endl;

    DWORD previous = 0;
    if (!VirtualProtect(region, bytes, PAGE_READONLY, &previous)) {
        std::cerr << "VirtualProtect failed: " << GetLastError() << "\n";
        VirtualFree(region, 0, MEM_RELEASE);
        return 2;
    }
    display("readonly", region);

    if (!VirtualProtect(region, bytes, previous, &previous)) {
        std::cerr << "Restore protection failed: " << GetLastError() << "\n";
        VirtualFree(region, 0, MEM_RELEASE);
        return 3;
    }
    display("restored", region);

    if (!VirtualFree(region, 0, MEM_RELEASE)) {
        std::cerr << "VirtualFree failed: " << GetLastError() << "\n";
        return 4;
    }
    std::cout << "released=true" << std::endl;
    return 0;
}
