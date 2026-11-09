#include <windows.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {
std::atomic_bool running{true};
std::mutex output_mutex;

BOOL WINAPI console_handler(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT || event == CTRL_CLOSE_EVENT) {
        running.store(false);
        return TRUE;
    }
    return FALSE;
}

void worker(unsigned int index) {
    constexpr SIZE_T region_size = 4096;
    void* memory = VirtualAlloc(nullptr, region_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!memory) {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cerr << "worker=" << index << " allocation_failed=" << GetLastError() << std::endl;
        return;
    }

    const std::string label = "WINDOWS_INJECTION_POC_WORKER_" + std::to_string(index);
    std::memcpy(memory, label.c_str(), label.size() + 1);

    {
        MEMORY_BASIC_INFORMATION info{};
        VirtualQuery(memory, &info, sizeof(info));
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "worker=" << index
                  << " tid=" << GetCurrentThreadId()
                  << " region=" << memory
                  << " size=" << info.RegionSize
                  << " state=0x" << std::hex << info.State
                  << " protect=0x" << info.Protect << std::dec
                  << " marker=" << label << std::endl;
    }

    auto* counter = static_cast<std::uint32_t*>(memory) + 32;
    while (running.load()) {
        ++(*counter);
        Sleep(100);
    }

    VirtualFree(memory, 0, MEM_RELEASE);
}

bool parse_duration(const char* value, unsigned long long& duration) {
    if (value == nullptr || *value == '-' || *value == 0) {
        return false;
    }
    char* end = nullptr;
    duration = std::strtoull(value, &end, 10);
    return end != value && *end == 0 && duration <= 86400;
}
}

int main(int argc, char** argv) {
    unsigned long long duration_sec = 0;

    if (argc == 3 && std::string(argv[1]) == "--duration-sec") {
        if (!parse_duration(argv[2], duration_sec)) {
            std::cerr << "Invalid duration\n";
            return 1;
        }
    } else if (argc != 1) {
        std::cerr << "Usage: multithreaded-target [--duration-sec 0]\n";
        return 1;
    }

    SetConsoleCtrlHandler(console_handler, TRUE);
    std::cout << "MultithreadedTarget pid=" << GetCurrentProcessId() << std::endl;

    std::vector<std::thread> workers;
    for (unsigned int i = 0; i < 3; ++i) {
        workers.emplace_back(worker, i);
    }

    const ULONGLONG start = GetTickCount64();
    while (running.load()) {
        if (duration_sec != 0 && GetTickCount64() - start >= duration_sec * 1000ULL) {
            break;
        }
        Sleep(100);
    }

    running.store(false);
    for (auto& thread : workers) {
        thread.join();
    }

    std::cout << "MultithreadedTarget stopped" << std::endl;
    return 0;
}
