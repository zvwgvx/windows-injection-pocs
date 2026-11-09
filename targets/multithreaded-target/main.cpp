#include <windows.h>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

std::atomic_bool running{true};

BOOL WINAPI handle_console(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT || type == CTRL_CLOSE_EVENT) {
        running.store(false);
        return TRUE;
    }
    return FALSE;
}

void worker(std::size_t index) {
    constexpr std::size_t region_size = 64 * 1024;
    auto* region = static_cast<unsigned char*>(
        VirtualAlloc(nullptr, region_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)
    );

    std::cout << "Worker " << index << " TID=" << GetCurrentThreadId()
              << " region=" << static_cast<void*>(region) << "\n";

    if (region == nullptr) {
        return;
    }

    unsigned char value = static_cast<unsigned char>(index);
    while (running.load()) {
        region[0] = ++value;
        Sleep(250);
    }

    VirtualFree(region, 0, MEM_RELEASE);
}

int main() {
    SetConsoleCtrlHandler(handle_console, TRUE);
    std::cout << "MultithreadedTarget PID=" << GetCurrentProcessId() << "\n";

    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < 3; ++i) {
        threads.emplace_back(worker, i);
    }

    while (running.load()) {
        Sleep(250);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Target stopped\n";
    return 0;
}
