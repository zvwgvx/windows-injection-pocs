#include <windows.h>
#include <atomic>
#include <cstdint>
#include <iostream>

std::atomic_bool running{true};

BOOL WINAPI handle_console(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT || type == CTRL_CLOSE_EVENT) {
        running.store(false);
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(handle_console, TRUE);
    std::cout << "BasicTarget PID=" << GetCurrentProcessId() << "\n";

    std::uint64_t heartbeat = 0;
    while (running.load()) {
        std::cout << "Heartbeat " << ++heartbeat << "\n";
        Sleep(1000);
    }

    std::cout << "Target stopped\n";
    return 0;
}
