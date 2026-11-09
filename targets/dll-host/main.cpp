#include <windows.h>
#include <iostream>

using MarkerFunction = const char* (__cdecl*)();

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: dll-host <path-to-marker-dll>\n";
        return 1;
    }

    HMODULE module = LoadLibraryA(argv[1]);
    if (!module) {
        std::cerr << "LoadLibrary failed: " << GetLastError() << "\n";
        return 2;
    }

    const auto marker = reinterpret_cast<MarkerFunction>(GetProcAddress(module, "LabMarker"));
    if (!marker) {
        std::cerr << "LabMarker export not found\n";
        FreeLibrary(module);
        return 3;
    }

    std::cout << "DLL loaded inside PID=" << GetCurrentProcessId() << "\n";
    std::cout << "Marker: " << marker() << "\n";
    std::cout << "Press Enter to unload the DLL\n";
    std::cin.get();

    FreeLibrary(module);
    return 0;
}
