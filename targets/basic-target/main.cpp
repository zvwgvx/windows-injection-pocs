#include <windows.h>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

namespace {
std::atomic_bool running{true};
char marker[] = "WINDOWS_INJECTION_POC_BASIC_TARGET_V1";

BOOL WINAPI console_handler(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT || event == CTRL_CLOSE_EVENT) {
        running.store(false);
        return TRUE;
    }
    return FALSE;
}

bool parse_unsigned(const char* text, unsigned long long& out) {
    if (text == nullptr || *text == '-' || *text == 0) {
        return false;
    }
    char* end = nullptr;
    const auto value = std::strtoull(text, &end, 10);
    if (end == text || *end != 0) {
        return false;
    }
    out = value;
    return true;
}
}

int main(int argc, char** argv) {
    DWORD interval_ms = 1000;
    unsigned long long duration_sec = 0;

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            std::cerr << "Usage: basic-target [--interval-ms 1000] [--duration-sec 0]\n";
            return 1;
        }
        unsigned long long value = 0;
        if (!parse_unsigned(argv[i + 1], value)) {
            std::cerr << "Invalid argument: " << argv[i + 1] << "\n";
            return 1;
        }
        const std::string flag = argv[i];
        if (flag == "--interval-ms" && value >= 50 && value <= 60000) {
            interval_ms = static_cast<DWORD>(value);
        } else if (flag == "--duration-sec" && value <= 86400) {
            duration_sec = value;
        } else {
            std::cerr << "Invalid option or value: " << flag << "\n";
            return 1;
        }
    }

    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        std::cerr << "Unable to install console handler\n";
        return 2;
    }

    const ULONGLONG start = GetTickCount64();
    std::uint64_t beat = 0;
    std::cout << "BasicTarget pid=" << GetCurrentProcessId()
              << " tid=" << GetCurrentThreadId()
              << " marker_address=" << static_cast<void*>(marker)
              << " marker=" << marker << std::endl;

    while (running.load()) {
        if (duration_sec != 0 && GetTickCount64() - start >= duration_sec * 1000ULL) {
            break;
        }
        std::cout << "heartbeat=" << ++beat << " uptime_ms=" << GetTickCount64() - start << std::endl;
        Sleep(interval_ms);
    }

    std::cout << "BasicTarget stopped beats=" << beat << std::endl;
    return 0;
}
