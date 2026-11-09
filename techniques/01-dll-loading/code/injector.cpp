#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2 || argv[1][0] == '-') {
        std::cerr << "Usage: injector.exe <owned-target-pid>" << std::endl;
        return 1;
    }

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(argv[1], &end, 10);
    if (end == argv[1] || *end != 0 || parsed == 0) {
        std::cerr << "Invalid PID" << std::endl;
        return 1;
    }

    const auto pid = static_cast<DWORD>(parsed);
    const std::wstring event = L"Local\\WinInjectionPoc01_" + std::to_wstring(pid);

    HANDLE handle = nullptr;
    for (int attempt = 0; attempt < 30; ++attempt) {
        handle = OpenEventW(EVENT_MODIFY_STATE, FALSE, event.c_str());
        if (handle != nullptr) {
            break;
        }
        Sleep(100);
    }

    if (!handle) {
        std::cerr << "No cooperative lab target with PID " << pid << " was found" << std::endl;
        return 2;
    }

    const BOOL signalled = SetEvent(handle);
    const DWORD error = signalled ? ERROR_SUCCESS : GetLastError();
    CloseHandle(handle);
    if (!signalled) {
        std::cerr << "SetEvent failed=" << error << std::endl;
        return 3;
    }

    std::cout << "poc=01-dll-loading signal_sent=true target_pid=" << pid << std::endl;
    return 0;
}
