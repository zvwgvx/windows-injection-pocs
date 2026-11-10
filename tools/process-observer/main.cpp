#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {
class Handle {
public:
    explicit Handle(HANDLE handle = nullptr) : value(handle) {}
    ~Handle() {
        if (value && value != INVALID_HANDLE_VALUE) {
            CloseHandle(value);
        }
    }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    HANDLE get() const {
        return value;
    }
    explicit operator bool() const {
        return value && value != INVALID_HANDLE_VALUE;
    }

private:
    HANDLE value;
};

struct RegionStats {
    std::uint64_t committed_bytes = 0;
    std::uint64_t private_bytes = 0;
    std::uint64_t image_bytes = 0;
    std::uint64_t mapped_bytes = 0;
    std::uint64_t executable_bytes = 0;
    std::uint64_t writable_bytes = 0;
    std::uint64_t regions = 0;
};

bool parse_number(const char* text, unsigned long& result, unsigned long minimum, unsigned long maximum) {
    if (!text || !*text || *text == '-') {
        return false;
    }
    char* end = nullptr;
    const auto value = std::strtoul(text, &end, 10);
    if (end == text || *end != 0 || value < minimum || value > maximum) {
        return false;
    }
    result = value;
    return true;
}

std::string utf8(const wchar_t* text) {
    if (!text) {
        return {};
    }
    const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(length), '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), length, nullptr, nullptr) == 0) {
        return {};
    }
    result.pop_back();
    return result;
}

bool is_executable(DWORD protection) {
    const DWORD flag = protection & 0xFF;
    return flag == PAGE_EXECUTE || flag == PAGE_EXECUTE_READ ||
           flag == PAGE_EXECUTE_READWRITE || flag == PAGE_EXECUTE_WRITECOPY;
}

bool is_writable(DWORD protection) {
    const DWORD flag = protection & 0xFF;
    return flag == PAGE_READWRITE || flag == PAGE_WRITECOPY ||
           flag == PAGE_EXECUTE_READWRITE || flag == PAGE_EXECUTE_WRITECOPY;
}

void report_process(HANDLE process, DWORD pid) {
    wchar_t path[32768]{};
    DWORD count = static_cast<DWORD>(sizeof(path) / sizeof(path[0]));
    if (QueryFullProcessImageNameW(process, 0, path, &count)) {
        std::cout << "process pid=" << pid << " path=" << std::quoted(utf8(path)) << '\n';
    } else {
        std::cout << "process pid=" << pid << " path_error=" << GetLastError() << '\n';
    }
}

void report_threads(DWORD pid) {
    Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
    if (!snapshot) {
        std::cout << "threads error=" << GetLastError() << '\n';
        return;
    }

    THREADENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    if (!Thread32First(snapshot.get(), &entry)) {
        std::cout << "threads error=" << GetLastError() << '\n';
        return;
    }

    unsigned int count = 0;
    do {
        if (entry.th32OwnerProcessID == pid) {
            ++count;
            std::cout << "thread id=" << entry.th32ThreadID << '\n';
        }
        entry.dwSize = sizeof(entry);
    } while (Thread32Next(snapshot.get(), &entry));

    std::cout << "threads total=" << count << '\n';
}

void report_modules(DWORD pid) {
    Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid));
    if (!snapshot) {
        std::cout << "modules error=" << GetLastError() << '\n';
        return;
    }

    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (!Module32FirstW(snapshot.get(), &entry)) {
        std::cout << "modules error=" << GetLastError() << '\n';
        return;
    }

    unsigned int count = 0;
    do {
        ++count;
        std::cout << "module name=" << std::quoted(utf8(entry.szModule))
                  << " base=" << static_cast<const void*>(entry.modBaseAddr)
                  << " size=" << entry.modBaseSize
                  << " path=" << std::quoted(utf8(entry.szExePath)) << '\n';
        entry.dwSize = sizeof(entry);
    } while (Module32NextW(snapshot.get(), &entry));

    std::cout << "modules total=" << count << '\n';
}

bool report_regions(HANDLE process) {
    SYSTEM_INFO system{};
    GetNativeSystemInfo(&system);
    const auto maximum = reinterpret_cast<std::uintptr_t>(system.lpMaximumApplicationAddress);
    auto address = reinterpret_cast<std::uintptr_t>(system.lpMinimumApplicationAddress);
    RegionStats stats{};
    DWORD last_error = ERROR_SUCCESS;

    while (address < maximum) {
        MEMORY_BASIC_INFORMATION region{};
        if (!VirtualQueryEx(process, reinterpret_cast<const void*>(address), &region, sizeof(region))) {
            last_error = GetLastError();
            break;
        }

        if (region.State == MEM_COMMIT) {
            ++stats.regions;
            stats.committed_bytes += region.RegionSize;
            if (region.Type == MEM_PRIVATE) {
                stats.private_bytes += region.RegionSize;
            }
            if (region.Type == MEM_IMAGE) {
                stats.image_bytes += region.RegionSize;
            }
            if (region.Type == MEM_MAPPED) {
                stats.mapped_bytes += region.RegionSize;
            }
            if (is_executable(region.Protect)) {
                stats.executable_bytes += region.RegionSize;
            }
            if (is_writable(region.Protect)) {
                stats.writable_bytes += region.RegionSize;
            }
        }

        const auto base = reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        if (region.RegionSize == 0 || base > std::numeric_limits<std::uintptr_t>::max() - region.RegionSize) {
            break;
        }
        const auto next = base + region.RegionSize;
        if (next <= address) {
            break;
        }
        address = next;
    }

    std::cout << "memory committed_regions=" << stats.regions
              << " committed_bytes=" << stats.committed_bytes
              << " private_bytes=" << stats.private_bytes
              << " image_bytes=" << stats.image_bytes
              << " mapped_bytes=" << stats.mapped_bytes
              << " executable_bytes=" << stats.executable_bytes
              << " writable_bytes=" << stats.writable_bytes
              << " enumeration_error=" << last_error << '\n';

    return stats.regions != 0;
}

void timestamp(unsigned long sample, DWORD pid) {
    SYSTEMTIME time{};
    GetSystemTime(&time);
    std::cout << "snapshot sample=" << sample
              << " pid=" << pid
              << " utc=" << std::setfill('0')
              << std::setw(4) << time.wYear << '-'
              << std::setw(2) << time.wMonth << '-'
              << std::setw(2) << time.wDay << 'T'
              << std::setw(2) << time.wHour << ':'
              << std::setw(2) << time.wMinute << ':'
              << std::setw(2) << time.wSecond << '.'
              << std::setw(3) << time.wMilliseconds
              << "Z" << std::setfill(' ') << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: process-observer (--self | --pid PID) [--samples N] [--interval-ms N]\n";
        return 1;
    }

    DWORD pid = 0;
    int index = 0;
    if (std::string(argv[1]) == "--self") {
        pid = GetCurrentProcessId();
        index = 2;
    } else if (argc >= 3 && std::string(argv[1]) == "--pid") {
        unsigned long parsed = 0;
        if (!parse_number(argv[2], parsed, 1, std::numeric_limits<DWORD>::max())) {
            std::cerr << "Invalid PID\n";
            return 1;
        }
        pid = static_cast<DWORD>(parsed);
        index = 3;
    } else {
        std::cerr << "Usage: process-observer (--self | --pid PID) [--samples N] [--interval-ms N]\n";
        return 1;
    }

    unsigned long samples = 1;
    unsigned long interval = 1000;
    while (index < argc) {
        if (index + 1 >= argc) {
            return 1;
        }
        unsigned long number = 0;
        const std::string argument = argv[index];
        if (argument == "--samples" && parse_number(argv[index + 1], number, 1, 120)) {
            samples = number;
        } else if (argument == "--interval-ms" && parse_number(argv[index + 1], number, 0, 60000)) {
            interval = number;
        } else {
            std::cerr << "Unknown option or invalid value: " << argument << '\n';
            return 1;
        }
        index += 2;
    }

    Handle process(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
    if (!process) {
        std::cerr << "OpenProcess failed=" << GetLastError() << '\n';
        return 2;
    }

    report_process(process.get(), pid);
    for (unsigned long i = 0; i < samples; ++i) {
        timestamp(i + 1, pid);
        report_threads(pid);
        report_modules(pid);
        if (!report_regions(process.get())) {
            std::cerr << "Memory metadata unavailable\n";
            return 3;
        }
        std::cout << "snapshot_end sample=" << i + 1 << std::endl;
        if (i + 1 < samples) {
            Sleep(static_cast<DWORD>(interval));
        }
    }

    return 0;
}
