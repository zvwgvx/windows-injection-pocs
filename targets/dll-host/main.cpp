#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <string>

using MarkerFunction = const char* (__cdecl*)();
using IncrementFunction = unsigned int (__cdecl*)();

int main(int argc, char** argv) {
    if (argc != 2 && argc != 4) {
        std::cerr << "Usage: dll-host <path-to-marker-dll> [--hold-ms 5000]\n";
        return 1;
    }

    DWORD hold_ms = 0;
    if (argc == 4) {
        if (std::string(argv[2]) != "--hold-ms" || *argv[3] == '-') {
            std::cerr << "Invalid option\n";
            return 1;
        }
        char* end = nullptr;
        const unsigned long value = std::strtoul(argv[3], &end, 10);
        if (end == argv[3] || *end != 0 || value > 300000) {
            std::cerr << "Invalid hold time\n";
            return 1;
        }
        hold_ms = static_cast<DWORD>(value);
    }

    HMODULE module = LoadLibraryA(argv[1]);
    if (module == nullptr) {
        std::cerr << "LoadLibrary failed error=" << GetLastError() << "\n";
        return 2;
    }

    const auto marker = reinterpret_cast<MarkerFunction>(GetProcAddress(module, "LabMarker"));
    const auto increment = reinterpret_cast<IncrementFunction>(GetProcAddress(module, "LabIncrement"));
    if (marker == nullptr || increment == nullptr) {
        std::cerr << "Required exports not found\n";
        FreeLibrary(module);
        return 3;
    }

    std::cout << "DllHost pid=" << GetCurrentProcessId()
              << " module_base=" << static_cast<void*>(module) << std::endl;
    std::cout << "LabMarker=" << marker() << " LabIncrement=" << increment() << std::endl;

    if (hold_ms != 0) {
        std::cout << "Holding module for " << hold_ms << "ms" << std::endl;
        Sleep(hold_ms);
    } else {
        std::cout << "Press Enter to unload" << std::endl;
        std::cin.get();
    }

    FreeLibrary(module);
    std::cout << "Module unloaded" << std::endl;
    return 0;
}
