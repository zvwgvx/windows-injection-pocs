#include <windows.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
using MarkerFunction = const char* (__cdecl*)();
using IncrementFunction = unsigned int (__cdecl*)();

std::wstring event_name(DWORD pid) {
    return L"Local\\WinInjectionPoc01_" + std::to_wstring(pid);
}

std::wstring payload_path() {
    wchar_t path[32768]{};
    DWORD length = GetModuleFileNameW(nullptr, path, static_cast<DWORD>(std::size(path)));
    if (!length || length == std::size(path)) {
        return {};
    }
    std::wstring full(path, length);
    const auto slash = full.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        return {};
    }
    return full.substr(0, slash + 1) + L"payload.dll";
}

int load_payload() {
    const auto path = payload_path();
    if (path.empty()) {
        std::cerr << "unable_to_locate_payload" << std::endl;
        return 2;
    }

    HMODULE module = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!module) {
        std::cerr << "payload_load_failed=" << GetLastError() << std::endl;
        return 3;
    }

    auto marker = reinterpret_cast<MarkerFunction>(GetProcAddress(module, "LabMarker"));
    auto increment = reinterpret_cast<IncrementFunction>(GetProcAddress(module, "LabIncrement"));
    if (!marker || !increment) {
        std::cerr << "payload_exports_missing" << std::endl;
        FreeLibrary(module);
        return 4;
    }

    std::cout << "payload_module=" << static_cast<void*>(module)
              << " payload_marker=" << marker()
              << " payload_count=" << increment()
              << std::endl;
    FreeLibrary(module);
    std::cout << "payload_unloaded=true" << std::endl;
    return 0;
}
}

int main(int argc, char** argv) {
    DWORD duration_seconds = 30;
    if (argc == 3 && std::string(argv[1]) == "--duration-sec") {
        if (argv[2][0] == '-') {
            return 1;
        }
        char* end = nullptr;
        const auto value = std::strtoul(argv[2], &end, 10);
        if (end == argv[2] || *end != 0 || value == 0 || value > 300) {
            std::cerr << "duration must be between 1 and 300 seconds" << std::endl;
            return 1;
        }
        duration_seconds = static_cast<DWORD>(value);
    } else if (argc != 1) {
        std::cerr << "Usage: target.exe [--duration-sec 30]" << std::endl;
        return 1;
    }

    const DWORD pid = GetCurrentProcessId();
    const auto name = event_name(pid);
    HANDLE trigger = CreateEventW(nullptr, FALSE, FALSE, name.c_str());
    if (!trigger) {
        std::cerr << "CreateEvent failed=" << GetLastError() << std::endl;
        return 2;
    }

    std::cout << "poc=01-dll-loading mode=cooperative-loader pid=" << pid << std::endl;
    std::cout << "event_ready=true" << std::endl;
    const DWORD result = WaitForSingleObject(trigger, duration_seconds * 1000);
    CloseHandle(trigger);

    if (result == WAIT_TIMEOUT) {
        std::cout << "result=timeout" << std::endl;
        return 0;
    }
    if (result != WAIT_OBJECT_0) {
        std::cerr << "WaitForSingleObject failed=" << GetLastError() << std::endl;
        return 3;
    }

    std::cout << "event_received=true" << std::endl;
    return load_payload();
}
